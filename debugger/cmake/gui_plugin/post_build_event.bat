set COPY_TO=%1

xcopy %QT_PATH64%\plugins\platforms\qwindows.dll %COPY_TO%\platforms\ /F /R /Y /D
xcopy %QT_PATH64%\plugins\platforms\qwindowsd.dll %COPY_TO%\platforms\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Cored.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Cored.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Guid.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Guid.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Widgetsd.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Widgetsd.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Networkd.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Networkd.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Core.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Core.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Gui.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Gui.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Widgets.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Widgets.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\Qt6Network.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\Qt6Network.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\lib\libEGL.lib %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\libEGL.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\icudt*.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\icuin*.dll %COPY_TO%\ /F /R /Y /D
xcopy %QT_PATH64%\bin\icuuc*.dll %COPY_TO%\ /F /R /Y /D

@echo off
@echo appdbg64g.exe -c ../../targets/functional_sim_gui.json > %COPY_TO%\_run_functional_sim.bat
@echo appdbg64g.exe -c ../../targets/sysc_river_gui.json > %COPY_TO%\_run_systemc_sim.bat
@echo appdbg64g.exe -c ../../targets/fpga_gui.json > %COPY_TO%\_run_fpga_gui.bat
@echo appdbg64g.exe -c ../../targets/stm32l4xx_gui.json > %COPY_TO%\_run_stm32l4xx.bat
@echo appdbg64g.exe -c ../../targets/dpi_gui.json > %COPY_TO%\_run_dpi.bat
