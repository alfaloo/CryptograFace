#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_set>

#include "headers/facialrecognision.h"

namespace fs = std::__fs::filesystem;
std::string directoryPath = fs::current_path();

std::unordered_set<std::string> currentUsers;

int main() {

    if (!faceCascade.load(directoryPath + "/data/haarcascades/haarcascade_frontalface_alt.xml")) {
        std::cout << "[Error] Could not load face cascade.\n";
        return -1;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath + "/facesets")) {
        if (fs::is_directory(entry)) {
            currentUsers.insert(entry.path().filename().string());
        }
    }

    std::string userName;
    std::cout << "Please enter your name: ";
    std::getline(std::cin, userName);

    if (currentUsers.count(userName) == 0) {
        if (!generateFaceset(userName, 5, 10)) {
            std::cout << "[Error] Could not generate facial data.\n";
            return -1;
        }
    }

    if (!trainFaceset()) {
        std::cout << "[Error] Could not train facial data.\n";
        return -1;
    }

    recogniseFaces();

    return 0;
}