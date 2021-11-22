#include "mach.hpp"

#include <iostream>

int
main(int argc, const char * argv[])
{
    if (argc < 2) {
        return 1;
    }
    
    std::ifstream in;
    in.open(argv[1], std::ios::binary);
    if (!in) {
        std::cerr << "Unable to open file.\n";
        return 1;
    }

    Mach::Mach mach;
    if (mach.parse(in)) {
        mach.print(std::cout);
    } else {
        std::cerr << "Unable to parse macho file.\n";
        return 1;
    }
    
    in.close();
    
    return 0;
}
