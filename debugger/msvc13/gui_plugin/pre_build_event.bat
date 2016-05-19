set GUI_PATH=..\..\src\gui_plugin\
set QT_PATH=E:\Projects\Qt\Qt5.4.1\5.4\msvc2013_64_opengl\bin
rem set QT_PATH=C:\Projects\Qt\Qt5.4.1\5.4\msvc2013_64\bin

rem %QT_PATH%\rcc.exe -binary ..\resources\gui.qrc -o ..\resources\gui.rcc
%QT_PATH%\moc.exe -i %GUI_PATH%\MainWindow\DbgMainWindow.h -o %GUI_PATH%\MainWindow\moc_DbgMainWindow.h
