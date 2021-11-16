#pragma once

#include "metadata-common.h"

namespace reven {

namespace binresource {
	class Metadata;
}

namespace metadata {
	/// \brief from_raw_metadata Construct a metadata from a raw metadata extracted from a binary file
	/// \param md The raw binary metadata
	/// \throws MetadataError if the version or the resource type are ill-formed
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	Metadata from_raw_metadata(const reven::binresource::Metadata& md);

	/// \brief to_bin_raw_metadata Construct a raw binary metadata from the information stored in this metadata
	reven::binresource::Metadata to_bin_raw_metadata(const Metadata& md);
}} // namespace reven::metadata
