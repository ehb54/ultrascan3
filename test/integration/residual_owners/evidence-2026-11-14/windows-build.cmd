@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" arm64_x64
if errorlevel 1 exit /b 1
set VCPKG_ROOT=C:\src\vcpkg
set PATH=C:\Users\me3\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;%PATH%
cmake -S C:/src/us3-residual-t10/source -B C:/src/us3-residual-t10/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DUS3_PROFILE=APP -DUS3_BUILD_SOMO=OFF -DUSE_QT6=ON -DCMAKE_TOOLCHAIN_FILE=C:/src/us3-residual-t10/source/admin/cmake/toolchain.cmake -DVCPKG_INSTALLED_DIR=C:/src/us3-calc/build/windows-release-qt6/vcpkg_installed -DVCPKG_MANIFEST_INSTALL=OFF -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_HOST_TRIPLET=x64-windows > C:\src\us3-residual-t10\configure.log 2>&1
if errorlevel 1 exit /b 2
cmake --build C:/src/us3-residual-t10/build --target us_fematch us_autoflow_analysis us_query_rmsd us_reporter_gmp us_com_project us_comproject_academic us_protocoldev -j 3 > C:\src\us3-residual-t10\build.log 2>&1
exit /b %errorlevel%
