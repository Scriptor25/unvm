#ifdef SYSTEM_DARWIN

#include <util.hxx>

#include <iostream>

int AppendUserPath(std::filesystem::path directory)
{
    directory = std::filesystem::absolute(directory / "bin");

    std::cerr << "please add the following line to your shell configuration:" << std::endl;
    std::cout << "export PATH=\"$PATH:" << directory.string() << "\"" << std::endl;
    return 0;
}

#endif
