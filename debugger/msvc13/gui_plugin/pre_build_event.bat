rem ---- set the following user's variable ---
rem QT_DEPLOY_LIB=C:\Projects\CppProjects\riscv_vhdl\debugger\libgui\libqtwin64
rem QT_QPA_PLATFORM_PLUGIN_PATH=C:\Projects\CppProjects\riscv_vhdl\debugger\libgui\libqtwin64\platforms
rem QT_PATH=C:\Projects\Qt\Qt5.6.1\5.6\msvc2013_64


set GUI_PLUGIN_SRC=..\..\src\gui_plugin\

rem %QT_PATH%\rcc.exe -binary ..\resources\gui.qrc -o ..\resources\gui.rcc
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\DbgMainWindow.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_DbgMainWindow.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\MdiAreaWidget.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_MdiAreaWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\SerialWidget\RawOutputWidget.h -o %GUI_PLUGIN_SRC%\SerialWidget\moc_RawOutputWidget.h
