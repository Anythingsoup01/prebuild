# Prebuild
## My own custom build generator, inspired by premake5

### To compile
Clone / Download the repository<br/>
Compile with your favorite tool<br/>
#### LINUX
```
git clone https://github.com/Anythingsoup01/prebuild /desired/path/to/repo
cd /desired/path/to/repo
mkdir build; cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build -j N
```
### To use
> Read documentation closely<br/>
#### LINUX
```
cp /path/to/binary /path/to/project/prebuild
cd /path/to/project
./prebuild cmake path/to/root
OR
./prebuild cmake
```

### Supported systems
cmake<br/>

### Syntax
Locate the docs folder for examples<br/>
