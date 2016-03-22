# ZXing C++ Port

[ZXing](https://github.com/zxing/zxing) is/was a Java library.

At some point a complete C++ port/rewrite was created and maintained in the official [ZXing](https://github.com/zxing/zxing) repo. However, at the time of writing the C++ port is no longer maintained and has been removed from the official ZXing repo.

This project was forked from the [last ZXing commit](https://github.com/zxing/zxing/commit/00f6340) to contain the C++ project, with the following exceptions

 * scons (Python) build system has been deleted.
 * Deleted black box tests, because they refer to a large test data in ZXing repo.
 * Added appropriate copyright/licensing details (based on those in the ZXing repo).
 * Updated README.md

Removal of build systems was done to minimise maintenance burden.

If tests and XCode projects (other than those produced automatically be CMake) are desired, then another repo should be created and this repo referenced as a submodule. 

# Building using CMake

CMake is a tool, that generates native makefiles and workspaces. It integrates well with a number of IDEs including Qt Creator and Visual Studio.

Usage with CLion or Qt Creator:

  1. Simply open `CMakeLists.txt` as a new project
  2. Additional command line arguments can be specified (see below)

Usage with Makefiles, Visual Studio, etc. (see `cmake --help` for a complete list of generators):

  1. `cd` to `cpp/build`
  3. Unix: run `cmake -G "Unix Makefiles" ..`
  3. Windows: run `cmake -G "Visual Studio 10" ..`
  
You can switch between build modes by specifying:

  - `-DCMAKE_BUILD_TYPE=Debug` or
  - `-DCMAKE_BUILD_TYPE=Release`

# Development tips

To profile the code (very useful to optimize the code):

  1. Install Valgrind
  2. Run `valgrind --tool=callgrind build/zxing - path/to/test/data/*.jpg > report.html`
  3. Analyze output using KCachegrind

To run the black box tests and check for changes:

  1. Build `zxing-img`, e.g., scons zxing
  2. Run the tests: `bash blackboxtest.sh 2>&1 | tee bb.results`
  3. Diff them with the known results: `diff bb.results blackboxtest.results`
