#pragma once

#include "metadata-common.h"

namespace reven {

namespace jsonresource {
class Metadata;
}

namespace metadata {
	/// \brief from_raw_metadata Construct a metadata from a raw metadata extracted from a json file
	/// \param md The raw json metadata
	/// \throws MetadataError if the version or the resource type are ill-formed
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	Metadata from_raw_metadata(const reven::jsonresource::Metadata& md);

	/// \brief to_json_raw_metadata Construct a raw json metadata from the information stored in this metadata
	reven::jsonresource::Metadata to_json_raw_metadata(const Metadata& md);
}} // namespace reven::metadata
