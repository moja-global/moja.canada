@echo off

REM Build moja.modules.gcbm dependencies from vcpkg manifest using Visual Studio 2022.
REM Requires CMake >= 3.26.3.
set FLINT_VCPKG_DIR="../../../FLINT/vcpkg/vcpkg"

pushd ..\source

if not exist build md build
pushd build

cmake -S .. ^
    -G "Visual Studio 17 2022" ^
    -DPQXX_INCLUDE_DIR=%FLINT_VCPKG_DIR%/packages/libpqxx_x64-windows/include ^
    -DPQXX_LIBRARY=%FLINT_VCPKG_DIR%/packages/libpqxx_x64-windows/bin/pqxx.dll ^
    -DCMAKE_INSTALL_PREFIX=bin ^
    -DVCPKG_TARGET_TRIPLET=x64-windows ^
    -DENABLE_TESTS=OFF ^
    -DCMAKE_TOOLCHAIN_FILE=%FLINT_VCPKG_DIR%/scripts/buildsystems/vcpkg.cmake

popd
popd
