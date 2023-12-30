# Gothic Multiplayer Launcher

## Getting started

Clone with submodules, like `git clone --recurse-submodules {URL}`

#### Windows

Install Visual Studio 2022 with C++ Toolchain.

Build Launcher
```bash
cmake -S . -B build -A Win32 "-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --target gml --config Release
```
Go to the build folder and add `gmp/gmp.dll` into it.

#### Ubuntu

Build Launcher
```bash
apt install git cmake gcc g++ qt6-base-dev libssl-dev
cmake -S . -B
cmake --build build --target gml --config Release
```
Go to the build folder and add `gmp/gmp.dll` into it. Also copy or cross compile `gmpinjector.exe`.

Run
```bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install wine wine32:i386 libqt6widgets6
chmod +x gmpinjector.sh
./gml
```
Adjust `gmpinjector.sh` if necessary.

To get windows-like path to your Gothic installation with winepath. In GML, set it under Settings => Options.
```bash
winepath -w "/path/to/gothic"
```

## Preview
![](docs/mainwindow.jpg)

## License
[MIT](LICENSE)
