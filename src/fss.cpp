#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include "hash.h"

namespace po = boost::program_options;

namespace fss {

    boost::filesystem::path workingDirectory() {
        return boost::filesystem::current_path();
    }

    boost::filesystem::path headPath() {
        boost::filesystem::path full_path(boost::filesystem::current_path());
        full_path /= ".fss";
        full_path /= "HEAD";
        return full_path;
    }

    boost::filesystem::path treesPath() {
        boost::filesystem::path full_path(boost::filesystem::current_path());
        full_path /= ".fss";
        full_path /= "trees";
        return full_path;
    }

    boost::filesystem::path objectsPath() {
        boost::filesystem::path full_path(boost::filesystem::current_path());
        full_path /= ".fss";
        full_path /= "objects";
        return full_path;
    }

    boost::filesystem::path currentBranchFile() {
        std::ifstream inputStream(headPath());
        std::string data;
        inputStream >> data;
        inputStream.close();
        boost::filesystem::path headFile(boost::filesystem::current_path());
        headFile /= ".fss";
        headFile /= data;
        return headFile;
    }

    std::string currentParent() {
        auto headFile = currentBranchFile();
        std::ifstream inputStream(headFile);
        std::string hash;
        inputStream >> hash;
        inputStream.close();
        return hash;
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
        HEADfile.open(headPath(), std::ofstream::out | std::ofstream::trunc);
        HEADfile << type << ref;
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

    std::vector<boost::filesystem::path> listObjects() {
        return listFiles(objectsPath());
    }

    boost::filesystem::path stagingPath() {
        boost::filesystem::path full_path(boost::filesystem::current_path());
        full_path /= ".fss";
        full_path /= "staging";
        return full_path;
    }

    std::vector<boost::filesystem::path> listStaging() {
        return listFiles(stagingPath());
    }

    boost::filesystem::path commitFile(boost::filesystem::path path) {
        boost::filesystem::path commitFilePath = path / ".__fss__commit.json";
        return commitFilePath;
    }

    boost::filesystem::path createCommitFile(boost::filesystem::path path) {
        auto j2 = R"(
            {
                "files": [],
                "meta": {
                    "message": "",
                    "parent": ""
                }
            }
            )"_json;
        std::string jsontext = j2.dump();

        std::ofstream commitFileStream;

        boost::filesystem::path commitFilePath = commitFile(path);
        
        if (!boost::filesystem::exists(commitFilePath)) {
            commitFileStream.open(commitFilePath, std::ofstream::out | std::ofstream::trunc);
            commitFileStream << jsontext << std::endl;
            commitFileStream.close();
        }
        return commitFilePath;
    }

    void addFile(boost::filesystem::path file) {
        std::string hash = createHash(file);
        bool found = false;
        for (auto path: listStaging()) {
            if (path.filename() == hash) {
                found = true;
            }
        }
        if (!found) {
            boost::filesystem::copy_file(file, stagingPath() / hash);
        }
        auto commitFilePath = createCommitFile(stagingPath());

        std::ifstream inputStream(commitFilePath);
        nlohmann::json json;
        inputStream >> json;
        inputStream.close();
        auto newElement = nlohmann::json::object();
        newElement["type"] = "file";
        newElement["name"] = file.filename().string();
        newElement["hash"] = hash;
        json["files"].push_back(newElement);

        std::ofstream outputStream(commitFilePath, std::ofstream::out | std::ofstream::trunc);
        outputStream << std::setw(4) << json << std::endl;
    }

    void addFile(std::string file) {
        addFile(workingDirectory() / file);
    }

    void updateBranch(std::string commitHash) {
        auto branchFile = currentBranchFile();
        std::ofstream HEADfile;
        HEADfile.open(branchFile, std::ofstream::out | std::ofstream::trunc);
        HEADfile << commitHash;
        HEADfile.close();
    }

    void commit(std::string message) {
        auto commitFilePath = createCommitFile(stagingPath());
        std::ifstream inputStream(commitFilePath);
        nlohmann::json json;
        inputStream >> json;
        inputStream.close();
        json["meta"]["message"] = message;
        json["meta"]["parent"] = currentParent();

        std::ofstream outputStream(commitFilePath, std::ofstream::out | std::ofstream::trunc);
        outputStream << std::setw(4) << json << std::endl;
        outputStream.close();

        std::string commitHash = createHash(commitFilePath);
        boost::filesystem::copy_file(commitFilePath, treesPath() / commitHash);
        boost::filesystem::remove(commitFilePath);
        for (auto fileName: listFiles(stagingPath())) {
            boost::filesystem::copy_file(fileName, objectsPath() / fileName.filename().string());
            boost::filesystem::remove(fileName);
        }
        updateBranch(commitHash);
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
        writeHEAD(full_path, "", "refs/heads/main");
        boost::filesystem::path objects = full_path;
        createFolder(objects.append("objects"));
        boost::filesystem::path trees = full_path;
        createFolder(trees.append("trees"));
        boost::filesystem::path staging = full_path;
        createFolder(staging.append("staging"));
    }
}


int main(int argc, char* argv[]) {
    po::variables_map vm;
    po::options_description desc{"Options"};
    try {
        desc.add_options()
            ("help,h", "Help")
            ("command", po::value<std::string>(), "Command")
            ("params", po::value<std::string>(), "Parameters");
        po::positional_options_description pos_desc;
        pos_desc.add("command", 1);
        pos_desc.add("params", -1);
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
            fss::addFile(vm["params"].as<std::string>());
        } else if (command == "commit") {
            fss::commit(vm["params"].as<std::string>());
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