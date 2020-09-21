#include <stdio.h>

#include "lib/error.h"
#include "file_extraction.h"

int main(int argc, char* argv[])
{   
    try {
        if (argc == 1)
            ExtractFile("source.night");
        else if (argc == 2)
            ExtractFile(argv[1]);
        else
            throw "invalid command line arguments";
    }
    catch (const char* e) {
        printf("[error] - %s\n", e);
    }
    catch (const Error& e) {
        printf("%s\n", e.what().cstr());
    }
}