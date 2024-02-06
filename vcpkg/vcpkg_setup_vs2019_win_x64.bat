@echo off

REM Build moja.modules.gcbm dependencies from vcpkg manifest using Visual Studio 2022.
set FLINT_VCPKG_DIR="%~dp0..\..\FLINT\vcpkg"

if not exist ..\source\build md ..\source\build
pushd ..\source\build

cmake -S .. ^
    -G "Visual Studio 16 2019" ^
    -DVCPKG_TARGET_TRIPLET=moja-x64-windows ^
    -DVCPKG_INSTALL_OPTIONS="--x-abi-tools-use-exact-versions" ^
    -DCMAKE_TOOLCHAIN_FILE=%FLINT_VCPKG_DIR%/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DCMAKE_INSTALL_PREFIX=bin ^
    -DPQXX_INCLUDE_DIR=%FLINT_VCPKG_DIR%/vcpkg/packages/libpqxx_x64-windows/include ^
    -DPQXX_LIBRARY=%FLINT_VCPKG_DIR%/vcpkg/packages/libpqxx_x64-windows/bin/pqxx.dll ^
    -DENABLE_TESTS=OFF

popd
