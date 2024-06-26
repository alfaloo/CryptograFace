//
// Created by Zhiyang Lu on 26/05/2024.
//

#include "cryptlib.h"
#include "rijndael.h"
#include "files.h"
#include "osrng.h"
#include "hex.h"
#include "eax.h"

#include <iostream>
#include <fstream>
#include <string>

bool encrypt(std::string username, std::string filename, std::string plain, std::string type);

std::pair<std::string, std::string> decrypt(std::string username, std::string filename);
