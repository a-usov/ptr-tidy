# ptr-tidy

## Building on Linux

#### First install Clang, LLVM and other dependencies. This will depend on your distribution.

##### Arch

```sh
# pacman -S clang llvm gtest gmock cmake make
```

##### Ubuntu/Debian

```sh
# apt-get install clang libclang-dev libgtest-dev cmake make
```

#### Then to build ptr-tidy

```sh
git clone https://github.com/a-usov/ptr-tidy.git
cmake -B ptr-tidy/build -S ptr-tidy
cmake --build ptr-tidy/build
```
