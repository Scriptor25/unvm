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
            << "  i, install, t, track, n, untrack, r, remove, u, use, l, list, c, complete, x, e, exec, execute\n"
            << "\n"
            << "Global Flags:\n"
            << "  ?, -?, -h, --help  Print this manual.\n"
            << "\n"
            << "Definitions:\n"
            << "  <version> := latest | lts | [v]<uint>[.<uint>[.<uint>]] | <lts-name> | <version range>\n"
            << "\n"
            << "Commands:\n"
            << "  install,          i <version> [-t|--track <name>]                Install the specified Node.js version. Use `-t` or `--track` to track this version, using the specified name.\n"
            << "  track,            t <name> <version>                             Track the specified version with the specified name. If the latest version matching the specified version is not yet installed, it is getting installed."
            << "  untrack,          n <name> [-p|--prune]                          Remove the version track with the specified name. This does not remove the tracked version itself, only if you either use `-p` or `--prune`."
            << "  update,           p [<name>]                                     Update a tracked version to the latest matching Node.js version. If no name is specified, update all tracked versions.\n"
            << "  remove,           r <version>                                    Remove the specified Node.js version.\n"
            << "  use,              u <version>|none [-l|--local]                  Set the active Node.js version, or 'none' to deactivate. Use `-l` or `--local` to only use for the current directory tree.\n"
            << "  list,             l [-a|--available] [-f|--flat] [-d|--details]  List installed versions. Use `-a` or `--available` to list version available online. Use `-f` or `--flat` to print as a flat list. Use `-d` or `--details` to print more details and subversions.\n"
            << "  complete,         c -- ...                                       Print a list of available auto-complete options to standard out.\n"
            << "  execute, exec, e, x [<version>] [-y|--yes] -- ...                Execute the given command within the context of the specified Node.js version, or the detected Node.js version if omitted. Use `-y` or `--yes` to skip confirmation on auto-installing missing versions.\n"
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
