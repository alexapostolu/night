#include <stdio.h>

#include "lib/error.h"
#include "./file_extraction.h"

int main(int argc, char* argv[])
{
    try {
        if (argc > 2)
            throw "invalid command line arguments";

        ExtractFile(argc == 1 ? "source.night" : argv[1]);
    }
    catch (const char* e) {
        fprintf(stderr, "[error] - %s\n", e);
    }
    catch (const Error& e) {
        fprintf(stderr, "%s\n", e.what().cstr());
    }
}