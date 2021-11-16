#include "metadata-sql.h"

#include <rvnsqlite/resource_database.h>

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
} // anonymous namespace

Metadata from_raw_metadata(const reven::sqlite::Metadata& md) {
	return Metadata(
		static_cast<ResourceType>(md.type()),
		Version::from_string(md.format_version()),
		md.tool_name(), Version::from_string(md.tool_version()), md.tool_info(),
		std::chrono::system_clock::time_point{std::chrono::seconds(md.generation_date())}
	);
}

reven::sqlite::Metadata to_sqlite_raw_metadata(const Metadata& md) {
	if (not md.custom_metadata().empty()) {
		throw WriteMetadataError("SQLITE resource does not support custom metadata.");
	}
	return SqliteMetadataWriter::write(md);
}

}} // namespace reven::metadata
