rem ---- set the following user's variable ---
rem QT_DEPLOY_LIB=C:\Projects\CppProjects\riscv_vhdl\debugger\libgui\libqtwin64
rem QT_QPA_PLATFORM_PLUGIN_PATH=C:\Projects\CppProjects\riscv_vhdl\debugger\libgui\libqtwin64\platforms
rem QT_PATH=C:\Projects\Qt\Qt5.6.1\5.6\msvc2013_64


set GUI_PLUGIN_SRC=..\..\src\gui_plugin\

%QT_PATH%\bin\rcc.exe -binary %GUI_PLUGIN_SRC%\resources\gui.qrc -o %GUI_PLUGIN_SRC%\resources\gui.rcc
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\DbgMainWindow.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_DbgMainWindow.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\MdiAreaWidget.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_MdiAreaWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\UnclosableQMdiSubWindow.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_UnclosableQMdiSubWindow.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\ControlWidget\ConsoleWidget.h -o %GUI_PLUGIN_SRC%\ControlWidget\moc_ConsoleWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\UartWidget.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_UartWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\GpioWidget.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_GpioWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\RegWidget.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_RegWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\RegsViewWidget.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_RegsViewWidget.h
