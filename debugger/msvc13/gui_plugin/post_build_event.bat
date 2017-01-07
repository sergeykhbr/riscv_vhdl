rem cd %OutDir%
rem pushd ..
rem set COPY_TO=%cd%
rem popd

rem set COPY_TO=$(SolutionDir)\..\win64build\$(Configuration)

cd %1
pushd ..
set COPY_TO=%cd%
popd

set GUI_PLUGIN_SRC=..\..\..\src\gui_plugin

rem  /F /R /Y /I
rem /S /Y /D
xcopy %QT_PATH%\plugins\platforms\qwindows.dll %COPY_TO%\platforms\ /F /R /Y /D
xcopy %QT_PATH%\lib\Qt5Core.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\Qt5Core.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\lib\Qt5Gui.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\Qt5Gui.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\lib\Qt5Widgets.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\Qt5Widgets.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\lib\libEGL.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\libEGL.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\icudt*.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\icuin*.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH%\bin\icuuc*.dll %COPY_TO%\ /F /R /Y /D
xcopy %GUI_PLUGIN_SRC%\resources\gui.rcc %COPY_TO%\resources\ /F /R /Y /D

@echo off
@echo appdbg64g.exe -c ../../targets/functional_sim_gui.json > %COPY_TO%\_run_functional_sim.bat
@echo appdbg64g.exe -c ../../targets/sysc_river_gui.json > %COPY_TO%\_run_systemc_sim.bat
