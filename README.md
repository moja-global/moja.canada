# moja.canada

## Building GCBM

1)
Clone the 'develop' branch of https://github.com/moja-global/FLINT

2)
Follow the instructions on https://github.com/moja-global/FLINT/tree/develop to build
vcpkg dependencies and the FLINT framework Visual Studio 2019 solution, with the
following changes because GCBM requires some optional FLINT modules:

-DENABLE_MOJA.MODULES.ZIPPER=ON
-DENABLE_MOJA.MODULES.GDAL=ON 

Resulting in a cmake command like:
cmake -G "Visual Studio 16 2019" -DCMAKE_INSTALL_PREFIX=C:/Development/Software/moja -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_TESTS=OFF -DENABLE_MOJA.MODULES.ZIPPER=ON -DENABLE_MOJA.MODULES.GDAL=ON -DCMAKE_TOOLCHAIN_FILE=c:\Development\moja-global\vcpkg\scripts\buildsystems\vcpkg.cmake ..

3)
Open the FLINT solution in VS2019 and build in debug mode, then right-click the INSTALL
project under CMakePredefinedTargets and build that. Repeat for release mode.

4)
Clone the 'develop' branch of https://github.com/SLEEK-TOOLS/moja.canada

5)
In the cloned moja.canada directory:

cd Source
mkdir build
cd build

cmake -G "Visual Studio 16 2019" -DCMAKE_INSTALL_PREFIX=C:/Development/Software/gcbm -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=c:\Development\moja-global\vcpkg\scripts\buildsystems\vcpkg.cmake ..
