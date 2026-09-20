# Gothic Multiplayer Launcher (GML)
___

# Getting started

## Windows

- [Visual Studio (>= 2022 17.6)](https://visualstudio.microsoft.com/vs/) with C++ Toolchain and the English language pack
- [vcpkg](https://github.com/microsoft/vcpkg?tab=readme-ov-file#get-started) *(optional)*

Qt6 can be built with vcpkg (takes some time) or [installed](https://www.qt.io/product/qt6) as a pre-built library.

### Build

```bash
cmake -B build --preset windows-x86-release
# Or if you have Qt installed as a pre-built library, pass -DCMAKE_PREFIX_PATH=C:/Qt/<version>/<compiler>/lib/cmake/Qt6
cmake --build build
cmake --install build --prefix dist
```

After build go to the dist folder and add `gmp/gmp.dll` into it.

## Debian/Ubuntu

```bash
sudo apt install cmake build-essential qt6-base-dev libssl-dev
```

### Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix dist
```

Go to the dist folder and add `gmp/gmp.dll` into it.

### Run
```bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install wine wine32:i386 libqt6widgets6
./gml
```
Adjust `gmpinjector.sh` if necessary.

To get windows-like path to your Gothic installation with winepath. In GML, set it under Settings => Options.
```bash
winepath -w "/path/to/gothic"
```
___
# Preview
![](docs/mainwindow.jpg)
___
# License
[MIT](LICENSE)
