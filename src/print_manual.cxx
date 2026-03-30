#include <unvm/unvm.hxx>

#include <iostream>

#include <version.h>

void unvm::PrintManual()
{
    std::cerr
            << PROJECT_NAME << " - " << PROJECT_TITLE << "\n"
            << "\n"
            << "  Version:    " << PROJECT_VERSION << "\n"
            << "  Build date: " << PROJECT_BUILD_DATE << "\n"
            << "\n"
            << "Usage:\n"
            << "  <version> := latest | lts | v<uint>[.<uint>[.<uint>]] | <lts-name>\n"
            << "\n"
            << "Commands:\n"
            << "  install,   i <version>        Install the specified Node.js version\n"
            << "  remove,    r <version>        Remove the specified Node.js version\n"
            << "  use,       u <version>|none   Set active Node.js version, or 'none' to deactivate\n"
            << "  list,      l [available|a]    List installed or available versions\n"
            << "             ls                 Alias for `list`\n"
            << "             la                 Alias for `list available`\n"
            << "  workspace, w                  Read package.json in current directory and automatically install and use a suitable Node.js version.\n"
            << "\n"
            << "Examples:\n"
            << "  unvm install lts\n"
            << "  unvm install iron\n"
            << "  unvm use v20.3.1\n"
            << "  unvm list available\n"
            << std::endl;
}
