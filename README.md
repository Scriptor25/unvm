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
git clone --depth 1 --single-branch https://github.com/Scriptor25/unvm.git
cd unvm
cmake -S . -B build -G Ninja
cmake --build build --parallel
```

## Usage

Run without arguments to see available commands:

```shell
unvm
```

### Version names

* `latest` — latest version
* `lts` — latest long-term-support version
* `v<X>[.<Y>[.<Z>]]` — specific version
* LTS by name, e.g., `Krypton` (case-insensitive)

### Commands

| Command                 | Description                                                 |
| ----------------------- | ----------------------------------------------------------- |
| `list [available]`      | List installed packages or all available. `*` marks active. |
| `install <version>`     | Install Node.js version.                                    |
| `remove <version>`      | Remove Node.js version.                                     |
| `use <version> \| none` | Set active Node.js version, or `none` to deactivate.        |

## Configuration

By default, UNVM generates a configuration file to track installed versions and active instance:

| Platform     | Path                                                                                               |
| ------------ | -------------------------------------------------------------------------------------------------- |
| Windows      | `%APPDATA%\unvm\config.json`                                                                       |
| Linux / Unix | `$XDG_CONFIG_HOME/unvm/config.json`, `$HOME/.config/unvm/config.json`, or `$CWD/.unvm/config.json` |

This file controls:

* Installation paths for Node.js versions
* Active instance linking
* Current installed versions and active version

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
