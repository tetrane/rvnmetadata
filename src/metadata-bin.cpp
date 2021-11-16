#include "metadata-bin.h"

#include <rvnbinresource/metadata.h>

namespace reven {
namespace metadata {

namespace {

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

}

Metadata from_raw_metadata(const reven::binresource::Metadata& md) {
	return Metadata(
		static_cast<ResourceType>(md.type()),
		Version::from_string(md.format_version()),
		md.tool_name(), Version::from_string(md.tool_version()), md.tool_info(),
		std::chrono::system_clock::time_point{std::chrono::seconds(md.generation_date())}
	);
}

reven::binresource::Metadata to_bin_raw_metadata(const Metadata& md) {
	if (not md.custom_metadata().empty()) {
		throw WriteMetadataError("Binary resource does not support custom metadata.");
	}
	return BinaryMetadataWriter::write(md);
}

}} // namespace reven::metadata
