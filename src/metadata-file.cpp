#include "metadata-file.h"

extern "C" {
#include <magic.h>
}

#include <boost/filesystem.hpp>

#include <rvnsqlite/resource_database.h>
#include <rvnjsonresource/writer.h>
#include <rvnjsonresource/reader.h>
#include <rvnbinresource/writer.h>
#include <rvnbinresource/reader.h>

#include "metadata-bin.h"
#include "metadata-json.h"
#include "metadata-sql.h"

namespace reven {
namespace metadata {

namespace {

enum class FormatType : std::uint8_t {
	Sqlite,
	Binary,
	Json,
};

FormatType get_resource_format_type(const char* filename) {
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
		return FormatType::Sqlite;
	} else if (magic_full == "application/octet-stream") {
		return FormatType::Binary;
	} else if (magic_full == "text/plain") {
		boost::filesystem::path file(filename);
		if (file.extension() == ".json") {
			return FormatType::Json;
		}
	} else if (magic_full == "application/json") {
		return FormatType::Json;
	}

	throw UnknownResourceError((std::string("Don't know how to read this resource")
	                            + " \"" + magic_full + "\".").c_str());
}


} // anonymous namespace

Metadata from_resource(const char* filename) {
	auto format_type = get_resource_format_type(filename);

	switch (format_type) {
		case FormatType::Sqlite:
			try {
				auto rdb = reven::sqlite::ResourceDatabase::open(filename);
				return from_raw_metadata(rdb.metadata());
			} catch(const reven::sqlite::MetadataError& e) {
				throw ReadMetadataError(e.what());
			} catch(const reven::sqlite::DatabaseError& e) {
				throw ReadMetadataError(e.what());
			}
		case FormatType::Binary:
			try {
				const auto bin_reader = reven::binresource::Reader::open(filename);
				return from_raw_metadata(bin_reader.metadata());
			} catch (const reven::binresource::ReaderError& e) {
				throw ReadMetadataError(e.what());
			}
			break;
		case FormatType::Json:
			try {
				const auto json_reader = reven::jsonresource::Reader::open(filename);
				return from_raw_metadata(json_reader.metadata());
			} catch (const reven::jsonresource::MetadataError& e) {
				throw ReadMetadataError(e.what());
			} catch (const reven::jsonresource::ReaderError& e) {
				throw ReadMetadataError(e.what());
			}
			break;
	};

	throw std::logic_error("Unreachable code");
}

void set_metadata(const char* filename, const Metadata& md) {
	auto format_type = get_resource_format_type(filename);

	switch (format_type) {
		case FormatType::Sqlite:
			try {
				auto rdb = reven::sqlite::ResourceDatabase::open(filename, false);
				rdb.set_metadata(to_sqlite_raw_metadata(md));
				return;
			} catch(const reven::sqlite::MetadataError& e) {
				throw WriteMetadataError(e.what());
			} catch(const reven::sqlite::DatabaseError& e) {
				throw WriteMetadataError(e.what());
			}
		case FormatType::Binary:
			try {
				auto bin_writer = reven::binresource::Writer::open(filename);
				bin_writer.set_metadata(to_bin_raw_metadata(md));
				return;
			} catch (const reven::binresource::WriterError& e) {
				throw WriteMetadataError(e.what());
			}
			break;
		case FormatType::Json:
			try {
				auto json_writer = reven::jsonresource::Writer::open(filename);
				json_writer.set_metadata(to_json_raw_metadata(md));
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


}} // namespace reven::metadata
