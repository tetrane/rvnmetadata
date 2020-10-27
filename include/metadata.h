#pragma once

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <experimental/string_view>

namespace reven {

namespace sqlite {
	class Metadata;
}

namespace binresource {
class Metadata;
}

namespace jsonresource {
class Metadata;
}

namespace metadata {

///
/// Root metadata exception. Catch this exception to catch all exceptions related to metadata.
///
class MetadataError : public std::runtime_error {
public:
	MetadataError(const char* msg) : std::runtime_error(msg) {}
};

///
/// Exception that occurs when the metadata type is unknown.
///
class UnknownMetadataTypeError : public MetadataError {
public:
	UnknownMetadataTypeError(const char* msg) : MetadataError(msg) {}
};

///
/// Exception that occurs when it is not possible to read the metadata from a resource.
///
class ReadMetadataError : public MetadataError {
public:
	ReadMetadataError(const char* msg) : MetadataError(msg) {}
};

///
/// Exception that occurs when it is not possible to write the metadata to a resource.
///
class WriteMetadataError : public MetadataError {
public:
	WriteMetadataError(const char* msg) : MetadataError(msg) {}
};

///
/// Exception that occurs when the reader doesn't recognize a resource.
///
class UnknownResourceError : public ReadMetadataError {
public:
	UnknownResourceError(const char* msg) : ReadMetadataError(msg) {}
};

///
/// Enum representing the different types of resources used in Reven
/// Important notes:
///  * The `_MinValue` **has to** be the same value than the first element of the enum
///  * The `_MaxValue` **has to be** the same value than the lats element of the enum
///  * Each element **has to** to be sorted in ascending order
///  * Each element value **has to** be consecutive
///
enum class ResourceType : std::uint32_t {
	_MinValue = 0x00000001,
	TraceBin = 0x00000001,
	TraceCache = 0x00000002,
	MemHist = 0x00000003,
	Strings = 0x00000004,
	StackEvents = 0x00000005,
	BinaryRanges = 0x00000006, // Deprecated
	PCRanges = 0x00000007,
	KernelDescription = 0x00000008,
	Block = 0x00000009,
	OssiRanges = 0x0000000a,
	_MaxValue = 0x0000000a,
};

/// \brief the method translate a ResourceType into a string name
/// \param type the ResourceType to translate
/// \return the std::string_view representation of the ResourceType
/// \throw UnknownResourceError if the ResourceType is not known
/// \note This method needs to be updated when the ResourceType enum class is updated
std::experimental::string_view to_string(const ResourceType type);

/// \brief the method translate a string (name of a resource) into a ResourceType
/// \param type the std::string to translate
/// \return the ResourceType corresponding to the string `resource_name`
/// \throw UnknownResourceError if the string is not a known type name
/// \note This method needs to be updated when the ResourceType enum class is updated
ResourceType to_resource_type(std::experimental::string_view resource_name);

///
/// Version class that contains the semantic versioning 2.0 of a resource
/// See the semantic versioning 2.0 specifications for more information
/// With only operators == and <, the identifiers can be put in a vector and sort in the lexicography order
///
class Version {
public:
	///
	/// Subclass representing an identifier in the Version, either a prerelease one or a build one
	/// Could be either a alphanumeric identifier or a numeric one
	///
	class Identifier {
	public:
		///
		/// \brief from_string Take string containing identifiers separated by dots and split it in a vector
		/// \param str The string of identifiers separated by dots
		/// \throws std::out_of_range if a numerical identifier doesn't fit in a std::uint64_t
		static std::vector<Identifier> from_string(const std::string& str);

		///
		/// \brief to_string Take a vector of identifiers and create a string by joining them with dots
		/// \param identifiers The vector of identifiers
		static std::string to_string(const std::vector<Identifier>& identifiers);

	public:
		///
		/// Enum representing the type of the Identifier
		///
		enum class Type {
			Number,
			String,
		};

	public:
		///
		/// \brief Identifier Construct a numeric identifier
		/// \param number The value of the identifier
		Identifier(std::uint64_t number)
		 : type_(Type::Number), value_{number, ""} {}

		///
		/// \brief Identifier Construct an alphanumeric identifier
		/// \param str The value of the identifier
		Identifier(std::string str)
		 : type_(Type::String), value_{0, std::move(str)} {}

		bool operator==(const Identifier& id) const {
			return id.type_ == type_ && id.value_.number == value_.number && id.value_.str == value_.str ;
		}

		bool operator!=(const Identifier& id) const {
			return !(*this == id);
		}

		bool operator<(const Identifier& id) const {
			if (type_ == Type::String && id.type_ == Type::Number)
				return false;
			else if (type_ == Type::Number && id.type_ == Type::String)
				return true;

			if (type_ == Type::Number)
				return value_.number < id.value_.number;
			else
				return value_.str < id.value_.str;
		}

		///
		/// \brief to_string Stringify the identifier
		std::string to_string() const {
			if (type_ == Version::Identifier::Type::Number) {
				return std::to_string(value_.number);
			}

			return value_.str;
		}

		///
		/// \brief type get the major of this version
		Type type() const { return type_; }

		///
		/// \brief number get the numeric value of the identifier if it's a numeric one
		///   it's a undefined behaviour if the Identifier isn't a numeric one
		std::uint64_t number() const { assert(type_ == Type::Number); return value_.number; }

		///
		/// \brief str get the numeric value of the identifier if it's an alphanumeric one
		///   it's a undefined behaviour if the Identifier isn't an alphanumeric one
		std::experimental::string_view str() const { assert(type_ == Type::String); return value_.str; }

	private:
		// TODO: Replace it by a std::variant in C++17
		Type type_;
		struct {
			std::uint64_t number;
			std::string str;
		} value_;
	};

	///
	/// A structure representing the comparison between two versions
	/// With the compatibility and more details about the timeline of the versions
	///
	struct Comparison {
		/// If the two versions are compatible or not
		bool is_compatible() const {
			return detail != PastIncompatible && detail != FutureIncompatible;
		}

		/// More detail about the timeline of the versions
		/// The greater the difference with Current, the greater the difference between the two versions
		enum : std::uint8_t {
			PastIncompatible,
			PastInFunctionalities,
			PastInFixes,
			Current,
			FutureInFixes,
			FutureInFunctionalities,
			FutureIncompatible,
		} detail;
	};

public:
	///
	/// \brief from_string Take string containing a semantic version 2.0.0 and create a Version instance
	/// \param str The string containing the version
	/// \throws MetadataError if the version is ill-formed
	/// \throws std::out_of_range if a numerical identifier doesn't fit in a std::uint64_t
	static Version from_string(const std::string& str);

public:
	///
	/// \brief Version Construct a semantic version
	/// \param major The major of this version
	/// \param minor The minor of this version
	/// \param patch The patch of this version
	/// \param prerelease A vector of identifier representing the prerelease identifiers
	/// \param build A vector of identifier representing the build identifiers
	Version(std::uint64_t major, std::uint64_t minor = 0, std::uint64_t patch = 0,
		std::vector<Identifier> prerelease = {}, std::vector<Identifier> build = {})
	 : version_numbers_{{major, minor, patch}},
	   prerelease_{std::move(prerelease)}, build_{std::move(build)} {}

	bool operator==(const Version& v) const {
		return v.version_numbers_ == version_numbers_ && v.prerelease_ == prerelease_;
	}

	bool operator!=(const Version& v) const {
		return !(*this == v);
	}

	bool operator<(const Version& v) const {
		if (version_numbers_ < v.version_numbers_)
			return true;
		else if (version_numbers_ > v.version_numbers_)
			return false;

		if (prerelease_.size() == 0 && v.prerelease_.size() > 0)
			return false;
		else if (prerelease_.size() > 0 && v.prerelease_.size() == 0)
			return true;

		return prerelease_ < v.prerelease_;
	}

	bool operator>(const Version& v) const {
		return !(*this == v || *this < v);
	}

	bool operator<=(const Version& v) const {
		return *this < v || *this == v;
	}

	bool operator>=(const Version& v) const {
		return !(*this < v);
	}

	///
	/// \brief compare Compare the version with another one to check the compatibility
	/// \param v The other version to compare to
	/// \note Do not compare the prerelease part of the version
	Comparison compare(const Version& v) const {
		if (version_numbers_ == v.version_numbers_) {
			return {Comparison::Current};
		}

		if (major() != v.major()) {
			if (major() < v.major()) {
				return {Comparison::PastIncompatible};
			} else {
				return {Comparison::FutureIncompatible};
			}
		}

		if (minor() != v.minor()) {
			if (minor() < v.minor()) {
				return {Comparison::PastInFunctionalities};
			} else {
				return {Comparison::FutureInFunctionalities};
			}
		}

		if (patch() < v.patch()) {
			return {Comparison::PastInFixes};
		} else {
			return {Comparison::FutureInFixes};
		}
	}

	///
	/// \brief to_string Stringify the version
	std::string to_string() const;

	///
	/// \brief major get the major of this version
	std::uint64_t major() const { return version_numbers_[0]; }

	///
	/// \brief minor get the minor of this version
	std::uint64_t minor() const { return version_numbers_[1]; }

	///
	/// \brief patch get the patch of this version
	std::uint64_t patch() const { return version_numbers_[2]; }

	///
	/// \brief prerelease get the prerelease identifiers of this version
	const std::vector<Identifier>& prerelease() const { return prerelease_; }

	///
	/// \brief build get the build identifiers of this version
	const std::vector<Identifier>& build() const { return build_; }

private:
	std::array<std::uint64_t, 3> version_numbers_;
	std::vector<Identifier> prerelease_;
	std::vector<Identifier> build_;
};

using CustomMetadata = std::unordered_map<std::string /* key */, std::string /* value */>;

class Metadata {
public:
	///
	/// \brief from_resource Construct a metadata from a resource file pointed by the filename
	/// \param filename The filename of the resource to open
	/// \throws UnknownResourceError if we can't determine how to open this resource
	/// \throws ReadMetadataError if there is an error when or after opening the resource
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	static Metadata from_resource(const char* filename);

	///
	/// \brief from_raw_metadata Construct a metadata from a raw metadata extracted from a sqlite database
	/// \param md The raw sqlite metadata
	/// \throws MetadataError if the version or the resource type are ill-formed
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	static Metadata from_raw_metadata(const reven::sqlite::Metadata& md);

	///
	/// \brief from_raw_metadata Construct a metadata from a raw metadata extracted from a binary file
	/// \param md The raw binary metadata
	/// \throws MetadataError if the version or the resource type are ill-formed
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	static Metadata from_raw_metadata(const reven::binresource::Metadata& md);

	///
	/// \brief from_raw_metadata Construct a metadata from a raw metadata extracted from a json file
	/// \param md The raw json metadata
	/// \throws MetadataError if the version or the resource type are ill-formed
	/// \throws std::out_of_range if a numerical identifier in the version doesn't fit in a std::uint64_t
	static Metadata from_raw_metadata(const reven::jsonresource::Metadata& md);

	///
	/// \brief set_metadata Set the metadata of a resource pointed by the filename
	/// \note The resource must already have metadata
	/// \param filename The filename of the resource to write to
	/// \param md The metadata to write in the resource
	/// \throws UnknownResourceError if we can't determine how to open this resource
	/// \throws WriteMetadataError if there is an error when or after opening the resource, e.g if the resource
	///   doesn't have metadata
	static void set_metadata(const char* filename, const Metadata& md);

public:
	///
	/// \brief Metadata Construct a metadata instance
	/// \param type The type of the resource
	/// \param format_version The version of the resource's format
	/// \param tool_name The name of the tool used to generate this resource
	/// \param tool_version The version of the tool used to generate this resource
	/// \param tool_info Other information about the tool used to generate this resource
	/// \param generation_date A point in time representing the date of the generation
	/// \throws MetadataError if the resource type is unknown
	Metadata(ResourceType type, Version format_version,
	         std::string tool_name, Version tool_version, std::string tool_info,
	         std::chrono::system_clock::time_point generation_date = std::chrono::system_clock::now())
		: Metadata(type, std::move(format_version), std::move(tool_name), std::move(tool_version),
		           std::move(tool_info), {}, std::chrono::time_point_cast<std::chrono::seconds>(generation_date))
	{
	}
	///
	/// \brief Metadata Construct a metadata instance
	/// \param type The type of the resource
	/// \param format_version The version of the resource's format
	/// \param tool_name The name of the tool used to generate this resource
	/// \param tool_version The version of the tool used to generate this resource
	/// \param tool_info Other information about the tool used to generate this resource
	/// \param custom_metadata Custom information associated to this resource
	/// \param generation_date A point in time representing the date of the generation
	/// \throws MetadataError if the resource type is unknown or custom metadata not printable
	Metadata(ResourceType type, Version format_version,
	         std::string tool_name, Version tool_version, std::string tool_info, const CustomMetadata& custom_metadata,
	         std::chrono::system_clock::time_point generation_date = std::chrono::system_clock::now());

	///
	/// \brief to_sqlite_raw_metadata Construct a raw sqlite metadata from the information stored in this metadata
	reven::sqlite::Metadata to_sqlite_raw_metadata() const;

	///
	/// \brief to_bin_raw_metadata Construct a raw binary metadata from the information stored in this metadata
	reven::binresource::Metadata to_bin_raw_metadata() const;

	///
	/// \brief to_json_raw_metadata Construct a raw json metadata from the information stored in this metadata
	reven::jsonresource::Metadata to_json_raw_metadata() const;

	///
	/// \brief type get the resource type of this metadata
	ResourceType type() const { return type_; }

	///
	/// \brief version get the format version of this metadata
	const Version& format_version() const { return format_version_; }

	///
	/// \brief tool_name get the tool name of this metadata
	std::experimental::string_view tool_name() const { return tool_name_; }

	///
	/// \brief version get the tool version of this metadata
	const Version& tool_version() const { return tool_version_; }

	///
	/// \brief tool_info get the tool info of this metadata
	std::experimental::string_view tool_info() const { return tool_info_; }

	///
	/// \brief generation_date get the generation date of this metadata
	std::chrono::system_clock::time_point generation_date() const { return generation_date_; }

	const CustomMetadata& custom_metadata() const { return custom_metadata_; }

private:
	enum class FormatType : std::uint8_t {
		Sqlite,
		Binary,
		Json,
	};

private:
	static FormatType get_resource_format_type(const char* filename);

private:
	ResourceType type_;
	Version format_version_;

	std::string tool_name_;
	Version tool_version_;
	std::string tool_info_;

	std::chrono::system_clock::time_point generation_date_;

	CustomMetadata custom_metadata_;
};

}} // namespace reven::metadata
