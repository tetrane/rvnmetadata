#pragma once

#include "metadata-common.h"

namespace reven {

namespace sqlite {
	class Metadata;
}

namespace metadata {
	/// \brief from_raw_metadata Construct a metadata from a raw metadata extracted from a sqlite database
	/// \param md The raw sqlite metadata
	/// \throws MetadataError if the version or the resource type are ill-formed
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	Metadata from_raw_metadata(const reven::sqlite::Metadata& md);

	/// \brief to_sqlite_raw_metadata Construct a raw sqlite metadata from the information stored in this metadata
	reven::sqlite::Metadata to_sqlite_raw_metadata(const Metadata& md);
}} // namespace reven::metadata
