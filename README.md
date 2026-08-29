# UNVM - Universal Node Version Manager

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/Scriptor25/unvm/cmake.yaml?style=flat-square)
![GitHub last commit](https://img.shields.io/github/last-commit/Scriptor25/unvm?style=flat-square)
![GitHub License](https://img.shields.io/github/license/Scriptor25/unvm?style=flat-square)
![GitHub Release Date](https://img.shields.io/github/release-date-pre/Scriptor25/unvm?style=flat-square&label=pre-release%20date)
![GitHub Release Date](https://img.shields.io/github/release-date/Scriptor25/unvm?style=flat-square)

## About

UNVM is a user-mode Node.js version manager. It does **not require administrative/root permissions**, which is useful
for:

- Work devices with restricted access
- Non-rooted mobile environments (e.g., Android with Termux)

## From 0 to running

1. You can either choose to use one of the prebuild binaries, or build your own.
2. Once you have the main binary, `unvm` (or `unvm.exe` on Windows), put it somewhere and add the directory to your
   `PATH` variable (or environment on Windows).
3. Then, if not already installed with the binary, create three symlinks (or hardlinks) to `unvm`:
    - `node`
    - `npm`
    - `npx`
4. Now, if all of these are also on your `PATH`, you should be able to just execute `node`, `npm` and `npx` as normal,
   with the core difference that they are only shims, which point to `unvm`. Then, a lot of dark magic happens, like
   finding the version for the current directory or project, before running the actual executables.

## Build

The project uses CMake for multi-platform builds. Tested configurations:

- **Windows x64**:
    - `Visual Studio 17 2022` | `MSVC`
    - `Ninja` | `Clang`
- **Linux x64**:
    - `Ninja` | `GCC`
    - `Ninja` | `Clang`
- **Darwin ARM64**:
    - `Ninja` | `GCC`
    - `Ninja` | `Clang`

### Dependencies

Make sure the following libraries are installed:

- `OpenSSL`
- `LibArchive`
- `ZLib`
- `LibLZMA`

### Build using CMake

```shell
git clone --depth 1 --single-branch --recurse-submodules --shallow-submodules https://github.com/Scriptor25/unvm.git
cd unvm
cmake -S . -B build -G Ninja
cmake --build build
```

### Install using CMake

```shell
cd unvm
sudo cmake --install build
```

## Usage

Run without arguments to see available commands:

```shell
unvm
```

Or append `?`, `-?`, `-h` or `--help` to print out the same manual.

### Version Names

- `latest` - latest version
- `lts` - latest long-term-support version
- `[v]<major>[.<minor>[.<patch>]]` - specific version
- LTS by name, e.g., `Krypton` (case-insensitive)

### Commands

| Command                                                                          | Description                                                                                                                                                                                              |
|----------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `install`,              `i` `<version>`                                          | Install the specified Node.js version.                                                                                                                                                                   |
| `remove`,               `r` `<version>`                                          | Remove the specified Node.js version.                                                                                                                                                                    |
| `use`,                  `u` `<version>\|none` `[-l\|--local]`                    | Set the active Node.js version, or `none` to deactivate. Use `-l` or `--local` to only use for the current directory tree.                                                                               |
| `list`,                 `l` `[-a\|--available]` `[-f\|--flat]` `[-d\|--details]` | List installed versions. Use `-a` or `--available` to list version available online. Use `-f` or `--flat` to print as a flat list. Use `-d` or `--details` to print more details and subversions.        |
| `complete`,             `c` `--` ...                                             | Print a list of available auto-complete options to standard out.                                                                                                                                         |
| `execute`, `exec`, `e`, `x` `[<version>]` `[-y\|--yes]` `--` ...                 | Execute the given command within the context of the specified Node.js version, or the detected Node.js version if omitted. Use `-y` or `--yes` to skip confirmation on auto-installing missing versions. |
| `track`                     `<tag>`                                              | Track a version tag to get latest versions of that tag when updating. `tag` can either be `latest` to always track the latest version, or some long-term-support version name (case-insensitive).        |
| `untrack`                   `<tag>`                                              | Untrack a previously tracked version tag.                                                                                                                                                                |
| `tags`                      `[-a\|--available]` `[-f\|--flat]`                   | List all tracked tags. Use `-a` or `--available` to list available tags. Use `-f` or `--flat` to print as a flat list.                                                                                   |
| `update`                    `[<tag>]`                                            | Update one or all tracked versions. If `tag` is specified, only update this version, else update all.                                                                                                    |

### Active Version

UNVM determines the active version for the current context using following steps:

1. if the current directory contains a file named `.unvm`, read it and use the exact version specified.
2. if the current directory contains a file named `package.json`, read it, parse the `semver` version specification from
   `engines.node`, and use the latest matching version.
3. if the current directory has a parent directory, move up one level and continue with step `1`
4. otherwise we have reached the file system root, so the global default version is used.

> The `.unvm` file will only be created if you call `unvm use <version> --local` to manually use a specific version for
> a directory tree.

## Files

UNVM generates a configuration file to track installed and active versions:

| Platform     | Path                                                                                               |
|--------------|----------------------------------------------------------------------------------------------------|
| Windows      | `%APPDATA%\unvm\config.json`                                                                       |
| Linux / Unix | `$XDG_CONFIG_HOME/unvm/config.json`, `$HOME/.config/unvm/config.json`, or `$PWD/.unvm/config.json` |

In the same directory, a local copy of the file at https://nodejs.org/dist/index.json is stored to avoid having to
stream it every time a version check happens. Also, the data directory contains a directory with the files for each
installed version.

This directory is also the home of all your locally installed versions, lock files etc.

## How does UNVM work

The core mechanic used by UNVM are shims. It installs with symlinks or hardlinks for `node`, `npm` and `npx`, pointing
to the `unvm` executable. Then, when they are executed, UNVM determines the active version for the current context and
executes the real executable for that version. Also, the version will be installed automatically if it is not yet
installed.

## License

UNVM is released under the **MIT License**. See the installed [`LICENSE`](./LICENSE.txt) file for the full license text.

## Third-Party Software

This project includes third-party software. See the installed [`THIRD_PARTY_NOTICES`](./THIRD_PARTY_NOTICES.txt) file
for full details and attributions.

Included libraries:

- **OpenSSL**: Apache License 2.0
- **LibArchive**: New BSD License
- **ZLib**: zlib License

These notices are included in the installation package to comply with each project's license requirements.
