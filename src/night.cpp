#include <stdio.h>

#include "lib/error.h"
#include "file_extraction.h"

int main(int argc, char* argv[])
{
    try {
        if (argc == 1)
            ExtractFile("C:\\Users\\apost\\source\\repos\\Night-Dev\\Night-Lang\\Night\\source.night");
        else if (argc == 2)
            ExtractFile(argv[1]);
        else
            throw "invalid command line arguments";
    }
    catch (const char* e) {
        fprintf(stderr, "[error] - %s\n", e);
    }
    catch (const Error& e) {
        fprintf(stderr, "%s\n", e.what().cstr());
    }
}