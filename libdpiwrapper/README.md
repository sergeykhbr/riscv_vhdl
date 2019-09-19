1. Build libdpiwrapper library using standalone tools (dll/so)
     Use msvc2019 Release x64 target by default on windows
     Use makefile on linux
2. Edit file "questasim64_10.4c/modelsim.ini" variable:
     DpiCppPath = $MODEL_TECH/../gcc-4.5.0-mingw64/bin/gcc
3. Run simulation with linked library (Questa command)
     vsim -novopt -c -sv_lib ../../../libdpiwrapper/win64build/Release/libdpiwrapper system_top
4. Increase simulation length (default 20 us)
5. Start Simulation
6. See 'dpilib.log' in the project directory for the additional debug information
     rtl/prj/questadpi1/dpilib.log (default on windows)
