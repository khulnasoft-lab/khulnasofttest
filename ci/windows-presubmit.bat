SETLOCAL ENABLEDELAYEDEXPANSION

SET BAZEL_EXE=%KOKORO_GFILE_DIR%\bazel-5.1.1-windows-x86_64.exe

SET PATH=C:\Python34;%PATH%
SET BAZEL_PYTHON=C:\python34\python.exe
SET BAZEL_SH=C:\tools\msys64\usr\bin\bash.exe
SET CMAKE_BIN="cmake.exe"
SET CTEST_BIN="ctest.exe"
SET CTEST_OUTPUT_ON_FAILURE=1
SET CMAKE_BUILD_PARALLEL_LEVEL=16
SET CTEST_PARALLEL_LEVEL=16

IF EXIST git\khulnasofttest (
  CD git\khulnasofttest
) ELSE IF EXIST github\khulnasofttest (
  CD github\khulnasofttest
)

IF %errorlevel% neq 0 EXIT /B 1

:: ----------------------------------------------------------------------------
:: CMake
MKDIR cmake_msvc2022
CD cmake_msvc2022

%CMAKE_BIN% .. ^
  -G "Visual Studio 17 2022" ^
  -DPYTHON_EXECUTABLE:FILEPATH=c:\python37\python.exe ^
  -DPYTHON_INCLUDE_DIR:PATH=c:\python37\include ^
  -DPYTHON_LIBRARY:FILEPATH=c:\python37\lib\site-packages\pip ^
  -Dktest_build_samples=ON ^
  -Dktest_build_tests=ON ^
  -Dkmock_build_tests=ON
IF %errorlevel% neq 0 EXIT /B 1

%CMAKE_BIN% --build . --target ALL_BUILD --config Debug -- -maxcpucount
IF %errorlevel% neq 0 EXIT /B 1

%CTEST_BIN% -C Debug --timeout 600
IF %errorlevel% neq 0 EXIT /B 1

CD ..
RMDIR /S /Q cmake_msvc2022

:: ----------------------------------------------------------------------------
:: Bazel

SET BAZEL_VS=C:\Program Files\Microsoft Visual Studio\2022\Community
%BAZEL_EXE% test ... ^
  --compilation_mode=dbg ^
  --copt=/std:c++14 ^
  --copt=/WX ^
  --features=external_include_paths ^
  --keep_going ^
  --test_output=errors ^
  --test_tag_filters=-no_test_msvc2017
IF %errorlevel% neq 0 EXIT /B 1
