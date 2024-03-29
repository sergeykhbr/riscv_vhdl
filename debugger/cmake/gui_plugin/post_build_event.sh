#!/bin/sh

echo "QT_PATH=" $QT_PATH

mkdir -pv $1/../qtlib
mkdir -pv $1/../qtlib/platforms
cp $QT_PATH/plugins/platforms/libqlinuxfb.so $1/../qtlib/platforms/
cp $QT_PATH/plugins/platforms/libqminimal.so $1/../qtlib/platforms/
cp $QT_PATH/plugins/platforms/libqxcb.so $1/../qtlib/platforms/
cp $QT_PATH/lib/libicudata.* $1/../qtlib
cp $QT_PATH/lib/libicui18n.* $1/../qtlib
cp $QT_PATH/lib/libicuuc.* $1/../qtlib
cp $QT_PATH/lib/libQt6Core.* $1/../qtlib
cp $QT_PATH/lib/libQt6Gui.* $1/../qtlib
cp $QT_PATH/lib/libQt6Widgets.* $1/../qtlib
cp $QT_PATH/lib/libQt6Network.* $1/../qtlib
cp $QT_PATH/lib/libQt6DBus.* $1/../qtlib
cp $QT_PATH/lib/libQt6XcbQpa.* $1/../qtlib
cp $QT_PATH/lib/libQt6OpenGL.* $1/../qtlib
