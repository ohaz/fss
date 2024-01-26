#pragma once
#include <iostream>
#include <boost/filesystem.hpp>

std::string hashString(std::string input);
std::string hashFile(boost::filesystem::path path);