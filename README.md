# ptr-tidy

## Building on Linux

#### First install clang, llvm and other dependancies. This will depend on your distribution.

##### Arch

```sh
pacman -S clang llvm gtest boost
```

##### Ubuntu

```sh
apt-get install clang libclang-dev libboost-dev libgtest-dev
```

#### Then to build ptr-tidy

```sh
git clone https://github.com/a-usov/ptr-tidy.git
cmake -B ptr-tidy/build -S ptr-tidy
cmake --build ptr-tidy/build
```
