
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <metadata.h>

using RequiredMetadata = std::vector<std::pair<std::string, std::string>>;
using CustomMetadata = std::vector<std::pair<std::string, std::string>>;

std::string from_time_point_to_string(const std::chrono::system_clock::time_point& tp)
{
	auto time = std::chrono::system_clock::to_time_t(tp);
	char buffer[24];
	strftime(buffer, sizeof(buffer), "%Y-%m-%dT%TZ", gmtime(&time));
	return buffer;
}

std::pair<RequiredMetadata, CustomMetadata> get_metadata(const boost::program_options::variables_map& vars,
                                                         const reven::metadata::Metadata& md) {
	RequiredMetadata required_metadata {};
	CustomMetadata custom_metadata {};
	if (vars.count("format-version")) {
		required_metadata.emplace_back("format-version", md.format_version().to_string());
	}
	if (vars.count("type")) {
		required_metadata.emplace_back("type", reven::metadata::to_string(md.type()).to_string());
	}
	if (vars.count("generation-date")) {
		required_metadata.emplace_back("generation-date", from_time_point_to_string(md.generation_date()));
	}
	if (vars.count("tool-name")) {
		required_metadata.emplace_back("tool-name", md.tool_name().to_string());
	}
	if (vars.count("tool-version")) {
		required_metadata.emplace_back("tool-version", md.tool_version().to_string());
	}
	if (vars.count("tool-info")) {
		required_metadata.emplace_back("tool-info", md.tool_info().to_string());
	}
	if (vars.count("custom")) {
		for (const auto& custom : md.custom_metadata()) {
			custom_metadata.emplace_back(custom.first, custom.second);
		}
	}
	if (required_metadata.empty() and custom_metadata.empty()) {
		required_metadata.emplace_back("format-version", md.format_version().to_string());
		required_metadata.emplace_back("type", reven::metadata::to_string(md.type()).to_string());
		required_metadata.emplace_back("generation-date", from_time_point_to_string(md.generation_date()));
		required_metadata.emplace_back("tool-name", md.tool_name().to_string());
		required_metadata.emplace_back("tool-version", md.tool_version().to_string());
		required_metadata.emplace_back("tool-info", md.tool_info().to_string());
		for (const auto& custom : md.custom_metadata()) {
			custom_metadata.emplace_back(custom.first, custom.second);
		}
	}
	return std::make_pair(required_metadata, custom_metadata);
}

void print_metadata_text(const std::pair<RequiredMetadata, CustomMetadata>& metadata)
{
	for (const auto& m : metadata.first) {
		std::cout << m.first << ": " << m.second << std::endl;
	}

	if (not metadata.second.empty()) {
		std::cout << "custom:" << std::endl;
		for (const auto& m : metadata.second) {
			std::cout << "\t" << m.first << ": " << m.second << std::endl;
		}
	}
}

void print_metadata_json(const std::pair<RequiredMetadata, CustomMetadata>& metadata)
{
	boost::property_tree::ptree root;

	for (const auto& m : metadata.first) {
		root.put(m.first, m.second);
	}

	for (const auto& m : metadata.second) {
		root.put(std::string("custom.") + m.first, m.second);
	}

	boost::property_tree::write_json(std::cout, root);
}

int main(int argc, char* argv[])
{
	try {
		std::string file;
		std::string output_format;

		namespace po = boost::program_options;
		po::options_description desc("Options description");
		desc.add_options()
			("help,h",
			 "Produce help message.")
			("file",
			 po::value<std::string>(&file),
			 "The file to read from")
			("output,o",
			 po::value<std::string>(&output_format)->default_value("text"),
			 "Format of the output. Must be \"text\" or \"json\"")
			("format-version",
			 "The format version of the file")
			("type,t",
			 "The type of the file")
			("generation-date,d",
			 "The generation date of the file. Must be in the standard ISO-8601 format (%Y-%m-%dT%TZ)")
			("tool-name",
			 "Name of the tool used for the generation")
			("tool-version",
			 "Version of the tool used for the generation")
			("tool-info",
			 "Info about the tool used for the generation")
			("custom",
			 "Custom data associated to the resource");;

		po::positional_options_description positional_options;
		positional_options.add("file", -1);

		po::variables_map vars;
		try {
			po::store(po::command_line_parser(argc, argv).options(desc).positional(positional_options).run(), vars);
			if (vars.count("help")) {
				std::cout << "Usage: ./metadata_reader [FILE] [OPTION]..." << std::endl;
				std::cout << desc << std::endl;
				return EXIT_SUCCESS;
			}
			po::notify(vars);
		} catch (const boost::program_options::error& error) {
			std::cerr << "Error: " << error.what() << std::endl;
			return EXIT_FAILURE;
		}

		if (!vars.count("file")) {
			std::cerr << "Error: missing the file to read from" << std::endl;
			std::cerr << "Usage: ./metadata_reader [FILE] [OPTION]..." << std::endl;
			return EXIT_FAILURE;
		}
		if (output_format != "text" && output_format != "json") {
			std::cerr << "Error: cannot format the output in " << output_format << std::endl;
			std::cerr << "Choose \"text\" or \"json\" as output" << std::endl;
			return EXIT_FAILURE;
		}

		const auto md = reven::metadata::Metadata::from_resource(file.c_str());
		const auto metadata = get_metadata(vars, md);
		if (output_format == "text") {
			print_metadata_text(metadata);
		} else {
			print_metadata_json(metadata);
		}

	} catch (const std::runtime_error& error) {
		std::cerr << "Error: " << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
