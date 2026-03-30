#if defined(SYSTEM_LINUX) || defined(SYSTEM_ANDROID) || defined(SYSTEM_DARWIN)

#include <unvm/util.hxx>

#include <iostream>

int unvm::AppendUserPath(std::filesystem::path directory)
{
    directory = std::filesystem::absolute(directory / "bin");

    std::cerr << "please add the following line to your shell configuration:" << std::endl;
    std::cout << "export PATH=\"$PATH:" << directory.string() << "\"" << std::endl;
    return 0;
}

#endif
