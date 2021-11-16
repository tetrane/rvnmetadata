#include "metadata-common.h"

#include <algorithm>
#include <sstream>
#include <regex>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace reven {
namespace metadata {

namespace {

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
		case ResourceType::OssiRanges:
			return "ossi_ranges";
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
	} else if (resource_name == "ossi_ranges") {
		return ResourceType::OssiRanges;
	}
	throw UnknownResourceError(("Resource type named " + resource_name.to_string() + " is not known").c_str());
}

}} // namespace reven::metadata
