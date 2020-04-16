
#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>
#include <string>

#include <metadata.h>

std::chrono::system_clock::time_point from_string_to_time_point(const std::string& gen_date)
{
	struct tm tm = {};
	const char* result = strptime(gen_date.c_str(), "%Y-%m-%dT%TZ", &tm);
	if (result == nullptr || *result != 0) {
		throw std::runtime_error("Wrong date format");
	}
	return std::chrono::system_clock::from_time_t(timegm(&tm));
}


reven::metadata::Metadata build_metadata(const boost::program_options::variables_map& vars,
                                         const reven::metadata::Metadata& old_metadata)
{
	reven::metadata::Version format_version = old_metadata.format_version();
	reven::metadata::ResourceType type = old_metadata.type();
	std::chrono::system_clock::time_point gen_date = old_metadata.generation_date();
	std::string tool_name = old_metadata.tool_name().to_string();
	reven::metadata::Version tool_version = old_metadata.tool_version();
	std::string tool_info = old_metadata.tool_info().to_string();
	reven::metadata::CustomMetadata custom_metadata = old_metadata.custom_metadata();

	if (vars.count("format-version")) {
		format_version = reven::metadata::Version::from_string(vars["format-version"].as<std::string>());
	}
	if (vars.count("type")) {
		type = reven::metadata::to_resource_type(vars["type"].as<std::string>());
	}
	if (vars.count("generation-date")) {
		gen_date = from_string_to_time_point(vars["generation-date"].as<std::string>());
	}
	if (vars.count("tool-name")) {
		tool_name = vars["tool-name"].as<std::string>();
	}
	if (vars.count("tool-version")) {
		tool_version = reven::metadata::Version::from_string(vars["tool-version"].as<std::string>());
	}
	if (vars.count("tool-info")) {
		tool_info = vars["tool-info"].as<std::string>();
	}
	if (vars.count("custom")) {
		for (const auto& custom : vars["custom"].as<std::vector<std::string>>()) {
			const auto split = custom.find(":");
			if (split == std::string::npos) {
				throw reven::metadata::WriteMetadataError("Wrong `custom` option format.");
			}
			const auto key = custom.substr(0, split);
			const auto value = custom.substr(split + 1);
			custom_metadata[key] = value;
		}
	}
	return reven::metadata::Metadata(type, format_version, tool_name, tool_version,
	                                 tool_info, custom_metadata, gen_date);
}

int main(int argc, char* argv[])
{
	try {
		std::string file;

		namespace po = boost::program_options;
		po::options_description desc("Options description");
		desc.add_options()
			("help",
			 "Produce help message.")
			("file",
			 po::value<std::string>(&file),
			 "The file to read from")
			("format-version,v",
			 po::value<std::string>(),
			 "The format version of the file")
			("type,t",
			 po::value<std::string>(),
			 "The type of the file")
			("generation-date,d",
			 po::value<std::string>(),
			 "The generation date of the file")
			("tool-name",
			 po::value<std::string>(),
			 "Name of the tool used for the generation")
			("tool-version",
			 po::value<std::string>(),
			 "Version of the tool used for the generation")
			("tool-info",
			 po::value<std::string>(),
			 "Info about the tool used for the generation")
			("custom",
			 po::value<std::vector<std::string>>(),
			 "Add custom metadata to the ressource. \n"
			 "Multiple custom metadata can be added. \n"
			 "format: \"key:value\" \n"
			 "example: --custom \"name:toto\" \n");

		po::positional_options_description positional_options;
		positional_options.add("file", -1);

		po::variables_map vars;
		try {
			po::store(po::command_line_parser(argc, argv).options(desc).positional(positional_options).run(), vars);
			if (vars.count("help")) {
				std::cout << "Usage: ./metadata_writer [FILE] [OPTION]..." << std::endl;
				std::cout << desc << std::endl;
				return EXIT_SUCCESS;
			}
			po::notify(vars);
		} catch (const boost::program_options::error& error) {
			std::cerr << "Error: " << error.what() << std::endl;
			return EXIT_FAILURE;
		}

		if (!vars.count("file")) {
			std::cerr << "Error: missing the file to write in" << std::endl;
			std::cerr << "Usage: ./metadata_writer [FILE] [OPTION]..." << std::endl;
			return EXIT_FAILURE;
		}

		auto md = reven::metadata::Metadata::from_resource(file.c_str());
		md.set_metadata(file.c_str(), build_metadata(vars, md));

	} catch (const std::runtime_error& error) {
		std::cerr << "Error: " << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
