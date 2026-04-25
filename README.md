# UNVM - Universal Node Version Manager

![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/Scriptor25/unvm/cmake.yaml?style=flat-square)
![GitHub last commit](https://img.shields.io/github/last-commit/Scriptor25/unvm?style=flat-square)
![GitHub License](https://img.shields.io/github/license/Scriptor25/unvm?style=flat-square)
![GitHub Release Date](https://img.shields.io/github/release-date-pre/Scriptor25/unvm?style=flat-square&label=pre-release%20date)
![GitHub Release Date](https://img.shields.io/github/release-date/Scriptor25/unvm?style=flat-square)

## About

UNVM is a user-mode Node.js version manager. It does **not require administrative/root permissions**, which is useful for:

- Work devices with restricted access
- Non-rooted mobile environments (e.g., Android with Termux)

## Build

The project uses CMake for cross-platform builds. Supported configurations:

- **Windows x64**: Visual Studio 17 2022 / MSVC or Ninja / Clang
- **Linux x64**: Ninja / GCC or Clang
- **Darwin ARM64**: Ninja / GCC or Clang

### Dependencies

Make sure the following libraries are installed:

- `OpenSSL`
- `LibArchive`
- `ZLib`

### Example: Fedora Linux 43

```shell
git clone --depth 1 --single-branch --recurse-submodules --shallow-submodules https://github.com/Scriptor25/unvm.git
cd unvm
cmake -S . -B build -G Ninja
cmake --build build --parallel
```

## Usage

Run without arguments to see available commands:

```shell
unvm
```

### Version Names

- `latest` — latest version
- `lts` — latest long-term-support version
- `<major>[.<minor>[.<patch>]]` — specific version
- LTS by name, e.g., `Krypton` (case-insensitive)

### Commands

| Command                         | Description                                                                                                   |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| `list [available]`              | List installed or available versions. `*` marks the active version in the current context.                    |
| `install <version>`             | Install the specified Node.js version.                                                                        |
| `remove <version>`              | Remove the specified Node.js version.                                                                         |
| `use <version> \| none [local]` | Set active Node.js version, or `none` to deactivate. Use `local` to only apply to the current directory tree. |

### Active Version

UNVM determines the active version for the current context using following steps:

1.  if the current directory contains a file named `.unvm`, read it and use the exact version specified.
2.  if the current directory contains a file named `package.json`, read it, parse the semver version specification from `engines.node`, and use the latest matching version.
3.  if the current directory has a parent directory, move up one level and continue with step `1`
4.  otherwise we have reached the file system root, so the global default version is used.

> The `.unvm` file will only be created if you call `unvm use ... local` to manually use a specific version for a directory tree.

## Files

UNVM generates a configuration file to track installed and active versions:

| Platform     | Path                                                                                               |
| ------------ | -------------------------------------------------------------------------------------------------- |
| Windows      | `%APPDATA%\unvm\config.json`                                                                       |
| Linux / Unix | `$XDG_CONFIG_HOME/unvm/config.json`, `$HOME/.config/unvm/config.json`, or `$PWD/.unvm/config.json` |

In the same directory, a local copy of the file at https://nodejs.org/dist/index.json is stored to avoid having to stream it every time a version check happens. Also, the data directory contains a directory with the files for each installed version.

## How does UNVM work

The core mechanic used by UNVM are shims. It installs with symlinks or hardlinks for `node`, `npm` and `npx`, pointing to the `unvm` executable. Then, when they are executed, UNVM determines the active version for the current context and executes the real executable for that version. Also, the version will be installed automatically if it is not yet installed.

## License

UNVM is released under the **MIT License**.
See the installed [`LICENSE`](./LICENSE.txt) file for the full license text.

## Third-Party Software

This project includes third-party software.
See the installed [`THIRD_PARTY_NOTICES`](./THIRD_PARTY_NOTICES.txt) file for full details and attributions.

Included libraries:

- **OpenSSL** — Apache License 2.0
- **LibArchive** — BSD 2-Clause License
- **ZLib** — zlib License

These notices are included in the install package to comply with each project's license requirements.
