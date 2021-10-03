mkdir $1/../qtlib
mkdir $1/../qtlib/platforms
cp $(QT_PATH64)/plugins/platforms/libqlinuxfb.so $1/../qtlib/platforms/
cp $(QT_PATH64)/plugins/platforms/libqminimal.so $1/../qtlib/platforms/
cp $(QT_PATH64)/lib/libicudata.* $1/../qtlib
cp $(QT_PATH64)/lib/libicui18n.* $1/../qtlib
cp $(QT_PATH64)/lib/libicuuc.* $1/../qtlib
cp $(QT_PATH64)/lib/libQt5Core.* $1/../qtlib
cp $(QT_PATH64)/lib/libQt5Gui.* $1/../qtlib
cp $(QT_PATH64)/lib/libQt5Widgets.* $1/../qtlib
cp $(QT_PATH64)/lib/libQt5Network.* $1/../qtlib
