#if defined(SYSTEM_LINUX) || defined(SYSTEM_DARWIN) // TODO: Please refactor i have no idea of C++ best practices (its for macos support)

#include <iostream>
#include <util.hxx>

int AppendUserPath(std::filesystem::path directory)
{
    directory = std::filesystem::absolute(directory / "bin");

    std::cerr << "please add the following line to your shell configuration:" << std::endl;
    std::cout << "export PATH=\"$PATH:" << directory.string() << "\"" << std::endl;
    return 0;
}

#endif
