# Gothic Multiplayer Launcher (GML)

## Getting started

Clone with submodules, like `git clone --recurse-submodules https://github.com/Reveares/GMP-Launcher`

### Windows

Install Visual Studio 2022 with C++ Toolchain.
Of course, Qt6 can also be installed as a pre-build library instead of building it with vcpkg.

#### Build

```bash
cmake -S . -B build -A Win32 "-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --target gml --config Release
```
Go to the build folder and add `gmp/gmp.dll` into it.

### Ubuntu

#### Build
```bash
apt install git cmake build-essential qt6-base-dev libssl-dev
cmake -S . -B build
cmake --build build --target gml --config Release
```
Go to the build folder and add `gmp/gmp.dll` into it. Also copy or cross compile `gmpinjector.exe`.

#### Run
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
