"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Test automation procedure (functional model for now).
"""

import subprocess
import rpc

#subprocess.Popen("..\\win32build\\Release\\appdbg64g.exe -c ..\\..\\targets\\sysc_river_gui.json")
subprocess.Popen("..\\win32build\\Release\\appdbg64g.exe -c ..\\..\\targets\\functional_sim_gui.json")

p = rpc.Simulator()
p.connect()

p.halt()

t1 = p.simSteps()
p.step(30000 - t1)

p.cmd("uart0 'set_module soc'")
p.step(20000)

p.cmd("uart0 dhry")
p.step(20000)

#p.simTimeSec()

p.exit()
p.disconnect()
