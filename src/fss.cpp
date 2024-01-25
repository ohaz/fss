#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;

namespace fss {
    void init(const po::variables_map& vm) {
        boost::filesystem::path full_path(boost::filesystem::current_path());
        boost::filesystem::path base_path = full_path;
        if (!boost::filesystem::exists(full_path.append(".fss"))) {
            std::cout << "Initialising in " << base_path << std::endl;
        }
    }
}


int main(int argc, char* argv[]) {
    po::variables_map vm;
    po::options_description desc{"Options"};
    try {
        desc.add_options()
            ("help,h", "Help")
            ("command", po::value<std::string>(), "Command");
        po::positional_options_description pos_desc;
        pos_desc.add("command", 1);
        po::command_line_parser parser{argc, argv};
        parser.options(desc).positional(pos_desc).allow_unregistered();
        po::parsed_options parsed_options = parser.run();
        po::store(parsed_options, vm);
        po::notify(vm);
    } catch (const po::error &ex) {
        std::cerr << ex.what() << std::endl;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
    }
    else if (vm.count("command")) {
        if (vm["command"].as<std::string>() == "init"){
            fss::init(&vm);
        } else {
            std::cerr << "Unknown command" << std::endl;
        }        
    } else {
        std::cerr << "You have to input a command." << std::endl
        << desc << std::endl;
        for (auto entry: vm) {
            std::cerr << entry.first << ":" << std::endl;
        }
    }
    return 0;
}