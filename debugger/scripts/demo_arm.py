TEST_NAME = "Tests N1: demo game"
TEST_DESCRIPTION = \
    "This test do the following steps:\n"\
    "  - Load elf-image into simulated target.\n"\
    "  - Start simulation.\n"\
    "  - Use button to select Tetris menu.\n"\
    "  - Start and play Tetris game.\n"\
    "  - Generate test reproduction documentation.\n"

import sys
sys.path.append(".\\modules")

import subprocess
import rpc
import doxytracer

subprocess.Popen("..\\win32build\\Release\\appdbg64g.exe -c ..\\..\\targets\\stm32l4xx_gui.json")

sim = rpc.Simulator()
sim.connect()

# Prepare documenetation generator
doxy = doxytracer.DoxyTracer(sim, "STM32L4xx platform description")
sim.setDoxyTracer(doxy)

doxy.addPage(TEST_NAME)
doxy.addParagraph(TEST_DESCRIPTION)

doxy.addSection("Device Initial State")
doxy.addParagraph(\
"General settings before the test started:\n"\
"  -# <b>Power On:</b>          {0}\n"\
.format(sim.isON()))

doxy.addSection("User Actions")

sim.clickButton("BTN_P7", comment="to power-on device.")
sim.go_msec(5100)
sim.clickButton("BTN_0", comment="to select menu Keyboard test")
sim.go_msec(500)
sim.saveScreenShot()
sim.clickButton("BTN_0", comment="to select menu Tetris")
sim.go_msec(200)
sim.saveScreenShot()

sim.clickButton("BTN_P2", comment="to start Tetris demo game")
sim.go_msec(500)
sim.saveScreenShot()

sim.clickButton("BTN_5", comment="to rotate figure")
sim.go_msec(200)
sim.clickButton("BTN_4", comment="to shift left")
sim.clickButton("BTN_4", comment="to shift left again")
sim.go_msec(200)
sim.saveScreenShot()
sim.clickButton("BTN_0", comment="to drop figure down")
sim.go_msec(200)
sim.saveScreenShot()

sim.go()
sim.halt()

doxy.addSection("Test statistics")
doxy.addParagraph(\
"General test information:\n"\
"  -# <b>Duration:</b>          {0} sec\n"\
"  -# <b>Test Code Coverage</b>:  {1} %\n"\
"  -# <b>Overall Code Coverage</b>:  {2} %\n"\
.format(sim.simTimeSec(), sim.cmd("coverage"), sim.cmd("coverage")))

# Generate documentation in current folder
doxy.generate("./")

sim.exit()
sim.disconnect()

subprocess.Popen("cd generated & doxygen.exe ..\\demo\\Doxyfile & cd latex & make.bat & refman.pdf", shell=True)

