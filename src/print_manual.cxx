#include <unvm/unvm.hxx>

#include <version.h>

#include <iostream>

void unvm::PrintManual()
{
    std::cerr
            << PROJECT_NAME << " - " << PROJECT_TITLE << "\n"
            << "\n"
            << "  Version:    " << PROJECT_VERSION << "\n"
            << "  Build date: " << PROJECT_BUILD_DATE << "\n"
            << "\n"
            << "Usage:\n"
            << "  unvm [<option|flag>...] [--] [<option>...]\n"
            << "\n"
            << "Options:\n"
            << "  i, install, r, remove, u, use, l, list, c, complete\n"
            << "\n"
            << "Flags:\n"
            << "  ?, -?, -h, --help, -l, --local, -a, --available, -f, --flat\n"
            << "\n"
            << "Definitions:\n"
            << "  <version> := latest | lts | [v]<uint>[.<uint>[.<uint>]] | <lts-name>\n"
            << "\n"
            << "Commands:\n"
            << "  install,   i <version>                                    Install the specified Node.js version.\n"
            << "  remove,    r <version>                                    Remove the specified Node.js version.\n"
            << "  use,       u <version>|none [-l|--local]                  Set the active Node.js version, or 'none' to deactivate. Use `-l` or `--local` to only use for the current directory tree.\n"
            <<
            "  list,      l [-a|--available] [-f|--flat] [-d|--details]  List installed versions. Use `-a` or `--available` to list version available online. Use `-f` or `--flat` to print as a flat list. Use `-d` or `--details` to print more details and subversions.\n"
            << "  complete,  c -- ...                                       Print a list of available auto-complete options to standard out.\n"
            << "\n"
            << "Examples:\n"
            << "  unvm ?\n"
            << "  unvm install lts\n"
            << "  unvm install iron\n"
            << "  unvm use 20.3.1\n"
            << "  unvm use krypton -l\n"
            << "  unvm list --available\n"
            << std::endl;
}
