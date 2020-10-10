# ptr-tidy

## Building 

First build and install llvm

```
mkdir llvm
git clone https://github.com/llvm/llvm-project.git
cmake -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=llvm -G Ninja -B llvm-project/build -S llvm-project
cmake --build llvm-project/build --target install 
```

Then to build ptr-tidy

````
git clone https://github.com/a-usov/ptr-tidy.git
cmake -DLLVM_ROOT=llvm -B ptr-tidy/build -S ptr-tidy
cmake --build ptr-tidy/build
```
