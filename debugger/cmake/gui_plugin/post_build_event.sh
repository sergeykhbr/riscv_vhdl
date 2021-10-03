mkdir $(ELF_DIR)/qtlib
mkdir $(ELF_DIR)/qtlib/platforms
cp $(QT_PATH)/plugins/platforms/libqlinuxfb.so $(ELF_DIR)/qtlib/platforms/
cp $(QT_PATH)/plugins/platforms/libqminimal.so $(ELF_DIR)/qtlib/platforms/
cp $(QT_LIB_PATH)/libicudata.* $(ELF_DIR)/qtlib
cp $(QT_LIB_PATH)/libicui18n.* $(ELF_DIR)/qtlib
cp $(QT_LIB_PATH)/libicuuc.* $(ELF_DIR)/qtlib
cp $(QT_LIB_PATH)/libQt5Core.* $(ELF_DIR)/qtlib
cp $(QT_LIB_PATH)/libQt5Gui.* $(ELF_DIR)/qtlib
cp $(QT_LIB_PATH)/libQt5Widgets.* $(ELF_DIR)/qtlib
cp $(QT_LIB_PATH)/libQt5Network.* $(ELF_DIR)/qtlib
