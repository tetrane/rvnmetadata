#include <boost/filesystem.hpp>
#include <string>

#include <metadata.h>

using reven::metadata::ResourceType;
using reven::metadata::Metadata;
using reven::metadata::Version;

inline bool check_version_strict_equality(const Version& a, const Version& b) {
	return a.major() == b.major()
	       && a.minor() == b.minor()
	       && a.patch() == b.patch()
	       && a.prerelease() == b.prerelease()
	       && a.build() == b.build();
}

inline bool check_version_strict_equality(const std::string& str_a, const Version& b) {
	auto a = Version::from_string(str_a);
	return check_version_strict_equality(a, b);
}

struct transient_directory {
	//! Path of created directory.
	boost::filesystem::path path;

	//! Create a uniquely named temporary directory in base_dir.
	//! A suffix is generated and appended to the given prefix to ensure the directory name is unique.
	//! Throw if directory cannot be created.
	transient_directory(const boost::filesystem::path& base_dir = boost::filesystem::temp_directory_path(),
	                    std::string prefix = {}) {
		boost::filesystem::path tmp_path = boost::filesystem::unique_path(prefix + "%%%%-%%%%-%%%%-%%%%");
		tmp_path = base_dir / tmp_path;

		if (!boost::filesystem::create_directories(tmp_path)) {
			throw std::runtime_error(("Can't create the directory " + tmp_path.native()).c_str());
		}

		this->path = tmp_path;
	}

	//! Delete created directory.
	~transient_directory() {
		boost::filesystem::remove_all(this->path);
	}
};
