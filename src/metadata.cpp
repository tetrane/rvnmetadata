#include "metadata.h"

#include <algorithm>
#include <sstream>
#include <regex>

extern "C" {
#include <magic.h>
}

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <rvnsqlite/resource_database.h>
#include <rvnbinresource/metadata.h>
#include <rvnbinresource/writer.h>
#include <rvnbinresource/reader.h>
#include <rvnjsonresource/metadata.h>
#include <rvnjsonresource/writer.h>
#include <rvnjsonresource/reader.h>

namespace reven {
namespace metadata {

namespace {

class SqliteMetadataWriter : reven::sqlite::MetadataWriter {
public:
	static reven::sqlite::Metadata write(const Metadata& md) {
		return reven::sqlite::MetadataWriter::write(
			static_cast<std::uint32_t>(md.type()), md.format_version().to_string(),
			md.tool_name().to_string(), md.tool_version().to_string(), md.tool_info().to_string(),
			static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(md.generation_date().time_since_epoch()).count())
		);
	}
};

class BinaryMetadataWriter : reven::binresource::MetadataWriter {
public:
	static reven::binresource::Metadata write(const Metadata& md) {
		return reven::binresource::MetadataWriter::write(
			static_cast<std::uint32_t>(md.type()), md.format_version().to_string(),
			md.tool_name().to_string(), md.tool_version().to_string(), md.tool_info().to_string(),
			static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(md.generation_date().time_since_epoch()).count())
		);
	}
};

class JsonMetadataWriter : reven::jsonresource::MetadataWriter {
public:
	static reven::jsonresource::Metadata write(const Metadata& md) {
		return reven::jsonresource::MetadataWriter::write(
			static_cast<std::uint32_t>(md.type()), md.format_version().to_string(),
			md.tool_name().to_string(), md.tool_version().to_string(), md.tool_info().to_string(),
			static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(md.generation_date().time_since_epoch()).count()),
			static_cast<reven::jsonresource::CustomMetadata>(md.custom_metadata())
		);
	}
};

bool is_number(const std::string& s) {
	return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

}

std::vector<Version::Identifier> Version::Identifier::from_string(const std::string& str) {
	if (str.empty())
		return {};

	std::vector<std::string> tokens;
	boost::split(tokens, str, boost::is_any_of("."));

	std::vector<Version::Identifier> identifiers;
	std::transform(
		tokens.begin(), tokens.end(), std::back_inserter(identifiers),
		[](const std::string& s) {
			if (is_number(s)) {
				if (s[0] == '0')
					throw MetadataError("Numeric identifier can't start with '0'");

				return Identifier(std::stoull(s));
			}

			return Identifier(s);
		}
	);

	return identifiers;
}

std::string Version::Identifier::to_string(const std::vector<Version::Identifier>& identifiers) {
	std::string output;

	for (unsigned i = 0; i < identifiers.size(); ++i) {
		output += identifiers[i].to_string();

		if (i < identifiers.size() - 1)
			output += ".";
	}

	return output;
}

Version Version::from_string(const std::string& str) {
	// regex to match semver 2.0.0
	//  (1) major version (0 or unlimited number)
	//  (2) minor version (0 or unlimited number)
	//  (3) patch version (0 or unlimited number)
	//  (4) optional pre-release following a dash consisting of
	//      identifiers (alphanumeric letters and hyphens) separated by dots
	//  (5) optional build following a plus consisting of
	//      identifiers (alphanumeric letters and hyphens) separated by dots
	std::regex regex("^"
	                 "(0|[1-9][0-9]*)" // (1)
	                 "\\.(0|[1-9][0-9]*)" // (2)
	                 "\\.(0|[1-9][0-9]*)" // (3)
	                 "(?:\\-([0-9a-zA-Z-]+[\\.0-9a-zA-Z-]*))?" // (4)
	                 "(?:\\+([0-9a-zA-Z-]+[\\.0-9a-zA-Z-]*))?" // (5)
	                 "$" , std::regex_constants::ECMAScript
	);

	std::smatch result;
	if (!std::regex_search(str, result, regex)) {
		throw MetadataError("The string version isn't correct");
	}

	return Version(
		std::stoull(result[1]), std::stoull(result[2]), std::stoull(result[3]),
		Version::Identifier::from_string(result[4]),
		Version::Identifier::from_string(result[5])
	);
}

std::string Version::to_string() const {
	std::stringstream ss;

	ss << version_numbers_[0] << "." << version_numbers_[1] << "." << version_numbers_[2];

	if (!prerelease_.empty()) {
		ss << "-" << Version::Identifier::to_string(prerelease_);
	}

	if (!build_.empty()) {
		ss << "+" << Version::Identifier::to_string(build_);
	}

	return ss.str();
}

namespace {


bool is_valid_key(const std::string& s)
{
	return std::find_if(s.begin(), s.end(), [](char c) { return c == '.'; }) == s.end();
}

bool is_printable(const std::string& s)
{
	return std::find_if(s.begin(), s.end(), [](char c) { return !std::isprint(c); }) == s.end();
}

void check_custom_metadata(const CustomMetadata& custom_metadata)
{
	for (const auto& custom : custom_metadata) {
		if (not is_printable(custom.first)) {
			throw MetadataError((std::string("Custom metadata key \"") + custom.first +
			                    "\" is not a printable string.").c_str());
		}
		if (not is_valid_key(custom.first)) {
			throw MetadataError((std::string("Custom metadata key \"") + custom.first +
			                    "\" must not contain a \'.\' char.").c_str());
		}
		if (not is_printable(custom.second)) {
			throw MetadataError((std::string("Custom metadata value \"") + custom.second +
			                    "\" is not a printable string.").c_str());
		}
	}
}

}

Metadata::Metadata(ResourceType type, Version format_version,
                   std::string tool_name, Version tool_version, std::string tool_info,
                   const CustomMetadata& custom_metadata,
                   std::chrono::system_clock::time_point generation_date)
	: type_{type}
	, format_version_{std::move(format_version)}
	, tool_name_{std::move(tool_name)}
	, tool_version_{std::move(tool_version)}
	, tool_info_{std::move(tool_info)}
	, generation_date_{std::chrono::time_point_cast<std::chrono::seconds>(generation_date)}
	, custom_metadata_{custom_metadata}
{
	if (type_ < ResourceType::_MinValue || type_ > ResourceType::_MaxValue) {
		throw UnknownMetadataTypeError("Unknown resource type");
	}

	check_custom_metadata(custom_metadata);
}

Metadata Metadata::from_resource(const char* filename) {
	auto format_type = Metadata::get_resource_format_type(filename);

	switch (format_type) {
		case Metadata::FormatType::Sqlite:
			try {
				auto rdb = reven::sqlite::ResourceDatabase::open(filename);
				return Metadata::from_raw_metadata(rdb.metadata());
			} catch(const reven::sqlite::MetadataError& e) {
				throw ReadMetadataError(e.what());
			} catch(const reven::sqlite::DatabaseError& e) {
				throw ReadMetadataError(e.what());
			}
		case Metadata::FormatType::Binary:
			try {
				const auto bin_reader = reven::binresource::Reader::open(filename);
				return Metadata::from_raw_metadata(bin_reader.metadata());
			} catch (const reven::binresource::ReaderError& e) {
				throw ReadMetadataError(e.what());
			}
			break;
		case Metadata::FormatType::Json:
			try {
				const auto json_reader = reven::jsonresource::Reader::open(filename);
				return Metadata::from_raw_metadata(json_reader.metadata());
			} catch (const reven::jsonresource::MetadataError& e) {
				throw ReadMetadataError(e.what());
			} catch (const reven::jsonresource::ReaderError& e) {
				throw ReadMetadataError(e.what());
			}
			break;
	};

	throw std::logic_error("Unreachable code");
}

Metadata Metadata::from_raw_metadata(const reven::sqlite::Metadata& md) {
	return Metadata(
		static_cast<ResourceType>(md.type()),
		Version::from_string(md.format_version()),
		md.tool_name(), Version::from_string(md.tool_version()), md.tool_info(),
		std::chrono::system_clock::time_point{std::chrono::seconds(md.generation_date())}
	);
}

Metadata Metadata::from_raw_metadata(const reven::binresource::Metadata& md) {
	return Metadata(
		static_cast<ResourceType>(md.type()),
		Version::from_string(md.format_version()),
		md.tool_name(), Version::from_string(md.tool_version()), md.tool_info(),
		std::chrono::system_clock::time_point{std::chrono::seconds(md.generation_date())}
	);
}

Metadata Metadata::from_raw_metadata(const reven::jsonresource::Metadata& md) {
	return Metadata(
		static_cast<ResourceType>(md.type()),
		Version::from_string(md.format_version()),
		md.tool_name(), Version::from_string(md.tool_version()), md.tool_info(),
		static_cast<CustomMetadata>(md.custom_metadata()),
		std::chrono::system_clock::time_point{std::chrono::seconds(md.generation_date())}
	);
}

void Metadata::set_metadata(const char* filename, const Metadata& md) {
	auto format_type = Metadata::get_resource_format_type(filename);

	switch (format_type) {
		case Metadata::FormatType::Sqlite:
			try {
				auto rdb = reven::sqlite::ResourceDatabase::open(filename, false);
				rdb.set_metadata(md.to_sqlite_raw_metadata());
				return;
			} catch(const reven::sqlite::MetadataError& e) {
				throw WriteMetadataError(e.what());
			} catch(const reven::sqlite::DatabaseError& e) {
				throw WriteMetadataError(e.what());
			}
		case Metadata::FormatType::Binary:
			try {
				auto bin_writer = reven::binresource::Writer::open(filename);
				bin_writer.set_metadata(md.to_bin_raw_metadata());
				return;
			} catch (const reven::binresource::WriterError& e) {
				throw WriteMetadataError(e.what());
			}
			break;
		case Metadata::FormatType::Json:
			try {
				auto json_writer = reven::jsonresource::Writer::open(filename);
				json_writer.set_metadata(md.to_json_raw_metadata());
				return;
			} catch (const reven::jsonresource::MetadataError& e) {
				throw WriteMetadataError(e.what());
			} catch (const reven::jsonresource::WriterError& e) {
				throw WriteMetadataError(e.what());
			}
			break;
	};

	throw std::logic_error("Unreachable code");
}

reven::sqlite::Metadata Metadata::to_sqlite_raw_metadata() const {
	if (not custom_metadata().empty()) {
		throw WriteMetadataError("SQLITE resource does not support custom metadata.");
	}
	return SqliteMetadataWriter::write(*this);
}

reven::binresource::Metadata Metadata::to_bin_raw_metadata() const {
	if (not custom_metadata().empty()) {
		throw WriteMetadataError("Binary resource does not support custom metadata.");
	}
	return BinaryMetadataWriter::write(*this);
}

reven::jsonresource::Metadata Metadata::to_json_raw_metadata() const {
	return JsonMetadataWriter::write(*this);
}

Metadata::FormatType Metadata::get_resource_format_type(const char* filename) {
	std::string magic_full = [filename]() {
		magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE | MAGIC_SYMLINK);

		if (magic_cookie == nullptr) {
			throw ReadMetadataError("Unable to initialize the magic library");
		}

		if (magic_load(magic_cookie, nullptr) != 0) {
			magic_close(magic_cookie);
			throw ReadMetadataError(("Cannot load magic database: " + std::string(magic_error(magic_cookie))).c_str());
		}

		std::string magic_full = magic_file(magic_cookie, filename);
		magic_close(magic_cookie);
		return magic_full;
	}();

	if (magic_full == "application/x-sqlite3") {
		return Metadata::FormatType::Sqlite;
	} else if (magic_full == "application/octet-stream") {
		return Metadata::FormatType::Binary;
	} else if (magic_full == "text/plain") {
		boost::filesystem::path file(filename);
		if (file.extension() == ".json") {
			return Metadata::FormatType::Json;
		}
	} else if (magic_full == "application/json") {
		return Metadata::FormatType::Json;
	}

	throw UnknownResourceError((std::string("Don't know how to read this resource")
	                            + " \"" + magic_full + "\".").c_str());
}

std::experimental::string_view to_string(const ResourceType type)
{
	switch (type) {
		case ResourceType::TraceBin:
			return "trace_bin";
		case ResourceType::TraceCache:
			return "trace_cache";
		case ResourceType::MemHist:
			return "memory_history";
		case ResourceType::Strings:
			return "strings";
		case ResourceType::StackEvents:
			return "stack_events";
		case ResourceType::BinaryRanges:
			return "binary_ranges";
		case ResourceType::PCRanges:
			return "pc_ranges";
		case ResourceType::KernelDescription:
			return "kernel_description";
		case ResourceType::Block:
			return "block";
	}
	throw UnknownResourceError(
			("Resource type with value " + std::to_string(static_cast<std::uint32_t>(type)) + " is not known").c_str());
}

ResourceType to_resource_type(std::experimental::string_view resource_name)
{
	if (resource_name == "trace_bin") {
		return ResourceType::TraceBin;
	} else if (resource_name == "trace_cache") {
		return ResourceType::TraceCache;
	} else if (resource_name == "memory_history") {
		return ResourceType::MemHist;
	} else if (resource_name == "strings") {
		return ResourceType::Strings;
	} else if (resource_name == "stack_events") {
		return ResourceType::StackEvents;
	} else if (resource_name == "binary_ranges") {
		return ResourceType::BinaryRanges;
	} else if (resource_name == "pc_ranges") {
		return ResourceType::PCRanges;
	} else if (resource_name == "kernel_description") {
		return ResourceType::KernelDescription;
	} else if (resource_name == "block") {
		return ResourceType::Block;
	}
	throw UnknownResourceError(("Resource type named " + resource_name.to_string() + " is not known").c_str());
}

}} // namespace reven::metadata
