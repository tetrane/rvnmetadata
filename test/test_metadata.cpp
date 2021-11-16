#define BOOST_TEST_MODULE RVN_METADATA_METADATA
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <rvnsqlite/resource_database.h>
#include <rvnbinresource/metadata.h>
#include <rvnjsonresource/metadata.h>

#include "test_helpers.h"

BOOST_AUTO_TEST_CASE(sqlite_raw_metadata)
{
	Metadata md(
		ResourceType::TraceBin,
		Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
		"TestSqliteMetadataWriter", Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}), "Test v1"
	);

	auto sqliteMetadata = to_sqlite_raw_metadata(md);
	auto md2 = reven::metadata::from_raw_metadata(sqliteMetadata);

	BOOST_CHECK(md.type() == md2.type());
	BOOST_CHECK(check_version_strict_equality(md.format_version(), md2.format_version()));
	BOOST_CHECK(md.tool_name() == md2.tool_name());
	BOOST_CHECK(check_version_strict_equality(md.tool_version(), md2.tool_version()));
	BOOST_CHECK(md.tool_info() == md2.tool_info());
	BOOST_CHECK(md.generation_date() == md2.generation_date());
}

BOOST_AUTO_TEST_CASE(binary_raw_metadata)
{
	Metadata md(
		ResourceType::TraceBin,
		Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
		"TestBinaryMetadataWriter", Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}), "Test v1"
	);

	auto binaryMetadata = to_bin_raw_metadata(md);
	auto md2 = reven::metadata::from_raw_metadata(binaryMetadata);

	BOOST_CHECK(md.type() == md2.type());
	BOOST_CHECK(check_version_strict_equality(md.format_version(), md2.format_version()));
	BOOST_CHECK(md.tool_name() == md2.tool_name());
	BOOST_CHECK(check_version_strict_equality(md.tool_version(), md2.tool_version()));
	BOOST_CHECK(md.tool_info() == md2.tool_info());
	BOOST_CHECK(md.generation_date() == md2.generation_date());
}

BOOST_AUTO_TEST_CASE(json_raw_metadata)
{
	Metadata md(
		ResourceType::KernelDescription,
		Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
		"TestJsonMetadataWriter", Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}), "Test v1"
	);

	auto jmd = to_json_raw_metadata(md);
	auto md2 = reven::metadata::from_raw_metadata(jmd);

	BOOST_CHECK(md.type() == md2.type());
	BOOST_CHECK(check_version_strict_equality(md.format_version(), md2.format_version()));
	BOOST_CHECK(md.tool_name() == md2.tool_name());
	BOOST_CHECK(check_version_strict_equality(md.tool_version(), md2.tool_version()));
	BOOST_CHECK(md.tool_info() == md2.tool_info());
	BOOST_CHECK(md.generation_date() == md2.generation_date());
}

BOOST_AUTO_TEST_CASE(resource_unknown_type)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/foo.png"),
	                  reven::metadata::UnknownResourceError);
}

BOOST_AUTO_TEST_CASE(resource_sqlite_not_versioned)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/sqlite/without_metadata.sqlite"),
	                  reven::metadata::ReadMetadataError);
}

BOOST_AUTO_TEST_CASE(resource_binary_not_versioned)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/binary/without_metadata.bin"),
	                  reven::metadata::ReadMetadataError);
}

BOOST_AUTO_TEST_CASE(resource_josn_not_versioned)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/json/without_metadata.json"),
	                  reven::metadata::ReadMetadataError);
}

BOOST_AUTO_TEST_CASE(resource_sqlite_wrong_type)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/sqlite/wrong_type.sqlite"),
	                  reven::metadata::UnknownMetadataTypeError);
}

BOOST_AUTO_TEST_CASE(resource_binary_wrong_type)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/binary/wrong_type.bin"),
	                  reven::metadata::UnknownMetadataTypeError);
}

BOOST_AUTO_TEST_CASE(resource_json_wrong_type)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/json/wrong_type.json"),
	                  reven::metadata::UnknownMetadataTypeError);
}

BOOST_AUTO_TEST_CASE(resource_sqlite_outdated)
{
	auto md = reven::metadata::from_resource(TEST_DATA "/sqlite/outdated.sqlite");

	BOOST_CHECK(md.type() == ResourceType::MemHist);

	BOOST_CHECK(check_version_strict_equality(
		md.format_version(),
		Version(1, 42, 12, {{"test"}}, {{"dummy"}})
	));

	BOOST_CHECK(md.tool_name() == "TestMetaDataWriter");
	BOOST_CHECK(check_version_strict_equality(
		md.tool_version(),
		Version(1, 0, 0, {{"prerelease"}}, {{}})
	));
	BOOST_CHECK(md.tool_info() == "Written for Quentin to test his type detection");
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(42424242)});
}

BOOST_AUTO_TEST_CASE(resource_binary_outdated)
{
	auto md = reven::metadata::from_resource(TEST_DATA "/binary/outdated.bin");

	BOOST_CHECK(md.type() == ResourceType::TraceBin);

	BOOST_CHECK(check_version_strict_equality(
		md.format_version(),
		Version(1, 0, 0, {{"dummy"}})
	));

	BOOST_CHECK(md.tool_name() == "TestMetaDataWriter");
	BOOST_CHECK(check_version_strict_equality(
		md.tool_version(),
		Version(1, 0, 0, {{"prerelease"}}, {{}})
	));
	BOOST_CHECK(md.tool_info() == "Tests version 1.0.0");
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(42424242)});
}

BOOST_AUTO_TEST_CASE(resource_json_incompatible)
{
	BOOST_CHECK_THROW(reven::metadata::from_resource(TEST_DATA "/json/incompatible.json"),
	                  reven::metadata::ReadMetadataError);
}

BOOST_AUTO_TEST_CASE(resource_sqlite_good)
{
	auto md = reven::metadata::from_resource(TEST_DATA "/sqlite/good.sqlite");

	BOOST_CHECK(md.type() == ResourceType::MemHist);

	BOOST_CHECK(check_version_strict_equality(
		md.format_version(),
		Version(1, 42, 12, {{"test"}}, {{"dummy"}})
	));

	BOOST_CHECK(md.tool_name() == "TestMetaDataWriter");
	BOOST_CHECK(check_version_strict_equality(
		md.tool_version(),
		Version(2, 42, 12, {{"test"}}, {{"dummy"}})
	));
	BOOST_CHECK(md.tool_info() == "Written by Quentin to test his type detection");
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(42424242)});
}

BOOST_AUTO_TEST_CASE(resource_binary_good)
{
	auto md = reven::metadata::from_resource(TEST_DATA "/binary/good.bin");

	BOOST_CHECK(md.type() == ResourceType::TraceBin);

	BOOST_CHECK(check_version_strict_equality(
		md.format_version(),
		Version(1, 0, 0, {{"dummy"}})
	));

	BOOST_CHECK(md.tool_name() == "TestMetaDataWriter");
	BOOST_CHECK(check_version_strict_equality(
		md.tool_version(),
		Version(1, 0, 0)
	));
	BOOST_CHECK(md.tool_info() == "Tests version 1.0.0");
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(42424242)});
}

BOOST_AUTO_TEST_CASE(resource_json_good)
{
	auto md = reven::metadata::from_resource(TEST_DATA "/json/good.json");

	BOOST_CHECK(md.type() == ResourceType::KernelDescription);

	BOOST_CHECK(check_version_strict_equality(
		md.format_version(),
		Version(1, 0, 0)
	));

	BOOST_CHECK(md.tool_name() == "TestMetaDataWriter");
	BOOST_CHECK(check_version_strict_equality(
		md.tool_version(),
		Version(1, 0, 0)
	));
	BOOST_CHECK(md.tool_info() == "Tests version 1.0.0");
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(42424242)});
}

constexpr const char* metadata_setter = "metadata_setter";
constexpr const char* metadata_setter_info = "metadata_setter info";

BOOST_AUTO_TEST_CASE(set_metadata_resource_unknown_type)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "foo.png";
	boost::filesystem::copy_file(TEST_DATA "/foo.png", tmp_file);

	BOOST_CHECK_THROW(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		), reven::metadata::UnknownResourceError
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_sqlite_not_versioned)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "without_metadata.sqlite";
	boost::filesystem::copy_file(TEST_DATA "/sqlite/without_metadata.sqlite", tmp_file);

	auto check_error_message = [&](const reven::metadata::WriteMetadataError& e) {
		return e.what() == std::string("Missing metadata. Is this a resource database?");
	};
	BOOST_CHECK_EXCEPTION(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		),
		reven::metadata::WriteMetadataError,
		check_error_message
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_binary_not_versioned)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "without_metadata.bin";
	boost::filesystem::copy_file(TEST_DATA "/binary/without_metadata.bin", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	auto check_error_message = [&](const reven::metadata::WriteMetadataError& e) {
		return e.what() == std::string("Wrong magic");
	};
	BOOST_CHECK_EXCEPTION(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		),
		reven::metadata::WriteMetadataError,
		check_error_message
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_json_not_versioned)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "without_metadata.json";
	boost::filesystem::copy_file(TEST_DATA "/json/without_metadata.json", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	auto check_error_message = [&](const reven::metadata::WriteMetadataError& e) {
		return e.what() == std::string("Missing \"metadata\" field");
	};
	BOOST_CHECK_EXCEPTION(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::KernelDescription,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		),
		reven::metadata::WriteMetadataError,
		check_error_message
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_sqlite_outdated)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "outdated.sqlite";
	boost::filesystem::copy_file(TEST_DATA "/sqlite/outdated.sqlite", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	BOOST_CHECK_THROW(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		), reven::metadata::WriteMetadataError
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_binary_outdated)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "outdated.bin";
	boost::filesystem::copy_file(TEST_DATA "/binary/outdated.bin", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	BOOST_CHECK_THROW(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		), reven::metadata::WriteMetadataError
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_json_incompatible)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "incompatible.json";
	boost::filesystem::copy_file(TEST_DATA "/json/incompatible.json", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	BOOST_CHECK_THROW(
		set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::KernelDescription,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
		), reven::metadata::WriteMetadataError
	);
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_sqlite_wrong_type)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "wrong_type.sqlite";
	boost::filesystem::copy_file(TEST_DATA "/sqlite/wrong_type.sqlite", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	set_metadata(
		tmp_file.c_str(),
		Metadata(
			ResourceType::Strings,
			Version(42, 42, 42),
			metadata_setter, Version(42, 42, 42), metadata_setter_info,
			std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
		)
	);

	auto md = reven::metadata::from_resource(tmp_file.c_str());

	BOOST_CHECK(md.type() == ResourceType::Strings);
	BOOST_CHECK(md.format_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_name() == metadata_setter);
	BOOST_CHECK(md.tool_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_info() == metadata_setter_info);
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(242424)});
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_binary_wrong_type)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "wrong_type.bin";
	boost::filesystem::copy_file(TEST_DATA "/binary/wrong_type.bin", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
	);

	auto md = reven::metadata::from_resource(tmp_file.c_str());

	BOOST_CHECK(md.type() == ResourceType::Strings);
	BOOST_CHECK(md.format_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_name() == metadata_setter);
	BOOST_CHECK(md.tool_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_info() == metadata_setter_info);
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(242424)});
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_json_wrong_type)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "wrong_type.json";
	boost::filesystem::copy_file(TEST_DATA "/json/wrong_type.json", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	set_metadata(
		tmp_file.c_str(),
		Metadata(
			ResourceType::KernelDescription,
			Version(42, 42, 42),
			metadata_setter, Version(42, 42, 42), metadata_setter_info,
			std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
		)
	);

	auto md = reven::metadata::from_resource(tmp_file.c_str());

	BOOST_CHECK(md.type() == ResourceType::KernelDescription);
	BOOST_CHECK(md.format_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_name() == metadata_setter);
	BOOST_CHECK(md.tool_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_info() == metadata_setter_info);
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(242424)});
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_sqlite_good)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "good.sqlite";
	boost::filesystem::copy_file(TEST_DATA "/sqlite/good.sqlite", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	set_metadata(
		tmp_file.c_str(),
		Metadata(
			ResourceType::Strings,
			Version(42, 42, 42),
			metadata_setter, Version(42, 42, 42), metadata_setter_info,
			std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
		)
	);

	auto md = reven::metadata::from_resource(tmp_file.c_str());

	BOOST_CHECK(md.type() == ResourceType::Strings);
	BOOST_CHECK(md.format_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_name() == metadata_setter);
	BOOST_CHECK(md.tool_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_info() == metadata_setter_info);
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(242424)});
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_binary_good)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "good.bin";
	boost::filesystem::copy_file(TEST_DATA "/binary/good.bin", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	set_metadata(
			tmp_file.c_str(),
			Metadata(
				ResourceType::Strings,
				Version(42, 42, 42),
				metadata_setter, Version(42, 42, 42), metadata_setter_info,
				std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
			)
	);

	auto md = reven::metadata::from_resource(tmp_file.c_str());

	BOOST_CHECK(md.type() == ResourceType::Strings);
	BOOST_CHECK(md.format_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_name() == metadata_setter);
	BOOST_CHECK(md.tool_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_info() == metadata_setter_info);
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(242424)});
}

BOOST_AUTO_TEST_CASE(set_metadata_resource_json_good)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "good.json";
	boost::filesystem::copy_file(TEST_DATA "/json/good.json", tmp_file);
	boost::filesystem::permissions(tmp_file, boost::filesystem::perms::owner_read |
	                                         boost::filesystem::perms::owner_write);

	set_metadata(
		tmp_file.c_str(),
		Metadata(
			ResourceType::Strings,
			Version(42, 42, 42),
			metadata_setter, Version(42, 42, 42), metadata_setter_info,
			std::chrono::system_clock::time_point{std::chrono::seconds(242424)}
		)
	);

	auto md = reven::metadata::from_resource(tmp_file.c_str());

	BOOST_CHECK(md.type() == ResourceType::Strings);
	BOOST_CHECK(md.format_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_name() == metadata_setter);
	BOOST_CHECK(md.tool_version() == Version(42, 42, 42));
	BOOST_CHECK(md.tool_info() == metadata_setter_info);
	BOOST_CHECK(md.generation_date() == std::chrono::system_clock::time_point{std::chrono::seconds(242424)});
}

BOOST_AUTO_TEST_CASE(correspondence_resource_type_and_string)
{
	for (std::uint32_t type = static_cast<std::uint32_t>(ResourceType::_MinValue);
	     type <= static_cast<std::uint32_t>(ResourceType::_MaxValue);
	     ++type) {
		BOOST_CHECK(reven::metadata::to_resource_type(
				reven::metadata::to_string(
						static_cast<ResourceType>(type)).to_string()) == static_cast<ResourceType>(type));
	}
}

BOOST_AUTO_TEST_CASE(known_resource_type_to_string)
{
	std::vector<std::string> string_type {
		"trace_bin", "trace_cache", "memory_history", "strings", "stack_events",
		"binary_ranges", "pc_ranges", "kernel_description", "block", "ossi_ranges"
	};

	BOOST_CHECK_EQUAL(string_type.size(), static_cast<std::uint32_t>(ResourceType::_MaxValue));
	for (std::uint32_t type = static_cast<std::uint32_t>(ResourceType::_MinValue);
	     type <= static_cast<std::uint32_t>(ResourceType::_MaxValue);
	     ++type) {
		// type - 1 to get the right string from the vector
		// because resources in ResourceType start at 1 but the vector start at 0
		BOOST_CHECK(reven::metadata::to_string(static_cast<ResourceType>(type)) == string_type.at(type - 1));
	}
}

BOOST_AUTO_TEST_CASE(unknown_resource_type_to_string)
{
	auto unknown_resource = static_cast<ResourceType>(static_cast<std::uint32_t>(ResourceType::_MaxValue) + 10);
	BOOST_CHECK_THROW(reven::metadata::to_string(unknown_resource), reven::metadata::UnknownResourceError);
}

BOOST_AUTO_TEST_CASE(known_resource_name_to_type)
{
	std::vector<std::string> string_type {
		"trace_bin", "trace_cache", "memory_history", "strings", "stack_events",
		"binary_ranges", "pc_ranges", "kernel_description", "block", "ossi_ranges"
	};

	BOOST_CHECK_EQUAL(string_type.size(), static_cast<std::uint32_t>(ResourceType::_MaxValue));
	for (std::uint32_t type = 0; type < string_type.size(); ++type) {
		// `type + 1` to find ResourceType
		// because the vector (then the loop) starts at 0 but resource starts at 1
		BOOST_CHECK(reven::metadata::to_resource_type(string_type.at(type)) == static_cast<ResourceType>(type + 1));
	}
}

BOOST_AUTO_TEST_CASE(unknown_resource_name_to_type)
{
	std::string unknown_string_type("toto");
	BOOST_CHECK_THROW(reven::metadata::to_resource_type(unknown_string_type), reven::metadata::UnknownResourceError);
}

BOOST_AUTO_TEST_CASE(custom_metadata)
{
	std::string valid_custom_key = "key";
	std::string valid_custom_value = "value";
	std::string invalid_custom_key = "invalid.key";
	std::string unprintable_custom_key = "ùkey";
	std::string unprintable_custom_value = "ùvalue";

	std::string error_message;
	auto check_write_metadata_error_message = [&error_message](const reven::metadata::WriteMetadataError& e) {
		return e.what() == error_message;
	};
	auto check_metadata_error_message = [&error_message](const reven::metadata::MetadataError& e) {
		return e.what() == error_message;
	};

	/* Test unsupported custom metadata feature */

	Metadata md(
		ResourceType::KernelDescription,
		Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
		"dummy",
		Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
		"Test v1",
		{{ valid_custom_key, valid_custom_value }}
	);

	error_message = "Binary resource does not support custom metadata.";
	BOOST_CHECK_EXCEPTION(to_bin_raw_metadata(md),
	                      reven::metadata::WriteMetadataError,
	                      check_write_metadata_error_message);

	error_message = "SQLITE resource does not support custom metadata.";
	BOOST_CHECK_EXCEPTION(to_sqlite_raw_metadata(md),
	                      reven::metadata::WriteMetadataError,
	                      check_write_metadata_error_message);

	BOOST_CHECK_NO_THROW(to_json_raw_metadata(md));


	/* Test custom metadata format errors */

	error_message = std::string("Custom metadata key \"") + unprintable_custom_key + "\" is not a printable string.";
	BOOST_CHECK_EXCEPTION(
		Metadata md(
			ResourceType::KernelDescription,
			Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
			"dummy",
			Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
			"Test v1",
			{{ unprintable_custom_key, valid_custom_value }}
		),
		reven::metadata::MetadataError,
		check_metadata_error_message
	);

	error_message = std::string("Custom metadata key \"") + invalid_custom_key + "\" must not contain a \'.\' char.";
	BOOST_CHECK_EXCEPTION(
		Metadata md(
			ResourceType::KernelDescription,
			Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
			"dummy",
			Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
			"Test v1",
			{{ invalid_custom_key, valid_custom_value }}
		),
		reven::metadata::MetadataError,
		check_metadata_error_message
	);

	error_message = std::string("Custom metadata value \"") + unprintable_custom_value
	                + "\" is not a printable string.";
	BOOST_CHECK_EXCEPTION(
		Metadata md(
			ResourceType::KernelDescription,
			Version(1, 2, 3, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
			"dummy",
			Version(3, 2, 1, {{"foo"}, {"bar"}}, {{"bar"}, {42}}),
			"Test v1",
			{{ valid_custom_key, unprintable_custom_value }}
		),
		reven::metadata::MetadataError,
		check_metadata_error_message
	);
}
