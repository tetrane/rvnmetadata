#include "metadata-json.h"

#include <rvnjsonresource/metadata.h>

namespace reven {
namespace metadata {

namespace {

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

} // anonymous namespace

Metadata from_raw_metadata(const reven::jsonresource::Metadata& md) {
	return Metadata(
		static_cast<ResourceType>(md.type()),
		Version::from_string(md.format_version()),
		md.tool_name(), Version::from_string(md.tool_version()), md.tool_info(),
		static_cast<CustomMetadata>(md.custom_metadata()),
		std::chrono::system_clock::time_point{std::chrono::seconds(md.generation_date())}
	);
}

reven::jsonresource::Metadata to_json_raw_metadata(const Metadata& md) {
	return JsonMetadataWriter::write(md);
}

}} // namespace reven::metadata
