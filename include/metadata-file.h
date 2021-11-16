#pragma once

#include "metadata-common.h"

namespace reven {
namespace metadata {
	/// \brief from_resource Construct a metadata from a resource file pointed by the filename
	/// \param filename The filename of the resource to open
	/// \throws UnknownResourceError if we can't determine how to open this resource
	/// \throws ReadMetadataError if there is an error when or after opening the resource
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	Metadata from_resource(const char* filename);

    /// \brief set_metadata Set the metadata of a resource pointed by the filename
	/// \note The resource must already have metadata
	/// \param filename The filename of the resource to write to
	/// \param md The metadata to write in the resource
	/// \throws UnknownResourceError if we can't determine how to open this resource
	/// \throws WriteMetadataError if there is an error when or after opening the resource, e.g if the resource
	///   doesn't have metadata
	void set_metadata(const char* filename, const Metadata& md);
}} // namespace reven::metadata
