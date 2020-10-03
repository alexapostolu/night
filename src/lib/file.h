#pragma once

#include <sstream>

std::ifstream Read(const char* fileName) {
    std::ifstream codeFile(fileName);
    std::stringstream msg;
    msg << "file " << fileName << " could not be opened";
    if (!codeFile.is_open())
        throw msg;
}