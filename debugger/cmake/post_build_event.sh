echo "#!/bin/bash" > $1/_run_func_river_x1_gui.sh
echo "export LD_LIBRARY_PATH=$1:$1/qtlib" >> $1/_run_func_river_x1_gui.sh
echo "./riscvdebugger -c $2/../targets/func_river_x1_gui.json" >> $1/_run_func_river_x1_gui.sh

echo "#!/bin/bash" > $1/_run_sysc_river_x1_gui.sh
echo "export LD_LIBRARY_PATH=$1:$1/qtlib" >> $1/_run_sysc_river_x1_gui.sh
echo "./riscvdebugger -c $2/../targets/sysc_river_x1_gui.json" >> $1/_run_sysc_river_x1_gui.sh

echo "#!/bin/bash" > $1/_run_gdb.sh
echo "export LD_LIBRARY_PATH=$1:$1/qtlib" >> $1/_run_gdb.sh
echo "export QT_DEBUG_PLUGINS=0" >> $1/_run_gdb.sh
echo "gdb --args ./riscvdebugger -c $2/../targets/func_river_x1_gui.json" >> $1/_run_gdb.sh

chmod +x $1/*.sh

