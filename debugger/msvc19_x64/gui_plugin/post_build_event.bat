set GUI_PLUGIN_SRC=..\..\src\gui_plugin
set COPY_TO=%1..

xcopy %QT_PATH64%\plugins\platforms\qwindows.dll %COPY_TO%\platforms\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt5Core.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt5Core.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt5Gui.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt5Gui.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt5Widgets.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt5Widgets.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt5Network.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt5Network.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\libEGL.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\libEGL.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\icudt*.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\icuin*.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\icuuc*.dll %COPY_TO%\ /F /R /Y /D
xcopy %GUI_PLUGIN_SRC%\resources\gui.rcc %COPY_TO%\resources\ /F /R /Y /D

rem xcopy %windir%\SysWOW64\msvcr120.dll %COPY_TO%\ /F /R /Y /D
rem xcopy %windir%\SysWOW64\msvcp120.dll %COPY_TO%\ /F /R /Y /D

@echo off
@echo appdbg64g.exe -c ../../targets/functional_sim_gui.json > %COPY_TO%\_run_functional_sim.bat
@echo appdbg64g.exe -c ../../targets/sysc_river_gui.json > %COPY_TO%\_run_systemc_sim.bat
@echo appdbg64g.exe -c ../../targets/fpga_gui.json > %COPY_TO%\_run_fpga_gui.bat
@echo appdbg64g.exe -c ../../targets/stm32l4xx_gui.json > %COPY_TO%\_run_stm32l4xx_sim.bat
@echo appdbg64g.exe -c ../../targets/dpi_gui.json > %COPY_TO%\_run_dpi.bat
