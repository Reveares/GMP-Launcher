# Gothic Multiplayer Launcher

## Getting started

Clone with submodules, like `git clone --recurse-submodules {URL}`

#### Windows

Install Visual Studio 2022 with C++ Toolchain.
```shell
cmake -S . -B build -A Win32 "-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```
Create a folder named `gmp` and add `gmp.dll` into it.

## Preview
![](docs/mainwindow.jpg)

## License
[MIT](LICENSE)
