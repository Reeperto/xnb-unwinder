#include "xnb.h"

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    std::string file_path(argv[1]);
    Xnb file1(file_path);

    return 0;
}
