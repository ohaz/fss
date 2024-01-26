#include "hash.h"
#include "sha.h"
#include "hex.h"
#include "filters.h"
#include "base64.h"
#include <sstream>
#include <cstddef>

std::string hashString(std::string input) {
    std::string digest;
    std::string output;
    
    using namespace CryptoPP;
    SHA256 hash;
    HexEncoder encoder(new StringSink(output));
    StringSource(input, true, new HashFilter(hash, new StringSink(digest)));

    StringSource(digest, true, new Redirector(encoder));
    
    return output;
}

std::string hashFile(boost::filesystem::path path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return hashString(buffer.str());
}