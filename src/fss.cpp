#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "hash.h"

namespace po = boost::program_options;

namespace fss {

    boost::filesystem::path workingDirectory() {
        return boost::filesystem::current_path();
    }

    void createFolder(boost::filesystem::path path) {
        if (!boost::filesystem::exists(path)) {
            boost::filesystem::create_directories(path);
        }
    }

    void createFolder(boost::filesystem::path path, std::string folderName) {
        path /= folderName;
        createFolder(path);
    }

    void writeHEAD(const boost::filesystem::path basePath, std::string type, std::string ref) {
        std::ofstream HEADfile;
        boost::filesystem::path tempPath = basePath;
        HEADfile.open(tempPath /= "HEAD", std::ofstream::out | std::ofstream::trunc);
        HEADfile << type << ": " << ref;
        HEADfile.close();
    }

    std::string createHash(boost::filesystem::path file) {
        std::string hash = hashFile(file);
        return hash;
    }

    std::vector<boost::filesystem::path> listFiles(boost::filesystem::path path) {
        boost::filesystem::directory_iterator end_itr;
        std::vector<boost::filesystem::path> paths = {};

        for (boost::filesystem::directory_iterator itr(path); itr != end_itr; ++itr) {
            if (boost::filesystem::is_regular_file(itr->path())) {
                paths.push_back(itr->path());
            }
        }
        return paths;
    }

    boost::filesystem::path objectsPath() {
        boost::filesystem::path full_path(boost::filesystem::current_path());
        full_path /= ".fss";
        full_path /= "objects";
        return full_path;
    }

    std::vector<boost::filesystem::path> listObjects() {
        return listFiles(objectsPath());
    }

    void addFile(boost::filesystem::path file) {
        std::string hash = createHash(file);
        bool found = false;
        for (auto path: listObjects()) {
            if (path.filename() == hash) {
                found = true;
            }
        }
        if (!found) {
            boost::filesystem::copy_file(file, objectsPath() / hash);
        }
        
    }

    void addFile(std::string file) {
        addFile(workingDirectory() / file);
    }

    void createBranch(const boost::filesystem::path basePath, std::string name) {
        boost::filesystem::path branches = basePath;
        branches /= "refs";
        branches /= "heads";
        createFolder(branches);
        std::string hash = "00000INITHASH";
        std::ofstream HEADfile;
        HEADfile.open(branches /= name, std::ofstream::out | std::ofstream::trunc);
        HEADfile << hash;
        HEADfile.close();
    }

    void init(const po::variables_map& vm) {
        boost::filesystem::path full_path = workingDirectory();
        boost::filesystem::path base_path = full_path;
        if (boost::filesystem::exists(full_path.append(".fss"))) {
            std::cerr << "Initialisation failed, folder already initialized" << std::endl;
            return;
        }
        std::cout << "Initialising in " << base_path << std::endl;
        createFolder(full_path);
        createBranch(full_path, "main");
        writeHEAD(full_path, "ref", "refs/heads/main");
        boost::filesystem::path objects = full_path;
        createFolder(objects.append("objects"));
        boost::filesystem::path trees = full_path;
        createFolder(trees.append("trees"));
    }

}


int main(int argc, char* argv[]) {
    po::variables_map vm;
    po::options_description desc{"Options"};
    try {
        desc.add_options()
            ("help,h", "Help")
            ("command", po::value<std::string>(), "Command")
            ("file", po::value<std::string>(), "file");
        po::positional_options_description pos_desc;
        pos_desc.add("command", 1);
        pos_desc.add("file", 1);
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
        std::string command = vm["command"].as<std::string>();
        if (command == "init"){
            fss::init(&vm);
        } else if (command == "add") {
            fss::addFile(vm["file"].as<std::string>());
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