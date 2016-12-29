rem ---- set the following user's variable ---
rem QT_DEPLOY_LIB=C:\Projects\CppProjects\riscv_vhdl\debugger\libgui\libqtwin64
rem QT_QPA_PLATFORM_PLUGIN_PATH=C:\Projects\CppProjects\riscv_vhdl\debugger\libgui\libqtwin64\platforms
rem QT_PATH=C:\Projects\Qt\Qt5.6.1\5.6\msvc2013_64


set GUI_PLUGIN_SRC=..\..\src\gui_plugin\

%QT_PATH%\bin\rcc.exe -binary %GUI_PLUGIN_SRC%\resources\gui.qrc -o %GUI_PLUGIN_SRC%\resources\gui.rcc
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\DbgMainWindow.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_DbgMainWindow.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\MdiAreaWidget.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_MdiAreaWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\UnclosableQMdiSubWindow.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_UnclosableQMdiSubWindow.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\MainWindow\UnclosableWidget.h -o %GUI_PLUGIN_SRC%\MainWindow\moc_UnclosableWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\ControlWidget\ConsoleWidget.h -o %GUI_PLUGIN_SRC%\ControlWidget\moc_ConsoleWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\ControlWidget\PnpWidget.h -o %GUI_PLUGIN_SRC%\ControlWidget\moc_PnpWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\UartEditor.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_UartEditor.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\UartWidget.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_UartWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\LedArea.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_LedArea.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\DipArea.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_DipArea.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\PeriphWidgets\GpioWidget.h -o %GUI_PLUGIN_SRC%\PeriphWidgets\moc_GpioWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\RegWidget.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_RegWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\RegsViewWidget.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_RegsViewWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\AsmArea.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_AsmArea.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\AsmControl.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_AsmControl.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\AsmViewWidget.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_AsmViewWidget.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\MemArea.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_MemArea.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\MemControl.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_MemControl.h
%QT_PATH%\bin\moc.exe -i %GUI_PLUGIN_SRC%\CpuWidgets\MemViewWidget.h -o %GUI_PLUGIN_SRC%\CpuWidgets\moc_MemViewWidget.h
