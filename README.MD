# Pre-build

```
pip3 install conan
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
conan profile update settings.cppstd=17 default
```

# Build
```
mkdir build && cd build
conan install ../recipies/connector --build=missing
conan create ../recipies/connector local@stable

conan install .. --build=missing
cmake .. 
cmake --build . --config Release
```
