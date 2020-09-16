"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Simulator's Python API implementation.
"""

import threading
import client
from events import ConsoleSubStringEvent
from .configurator import PlatformConfig

class Simulator(object):
    def __init__(self):
        self.client = None
        self.eventDone = threading.Event()
        self.doxy = None

    def connect(self):
        self.eventDone.clear()
        self.client = client.TcpClient("rpcclient", self.eventDone)
        self.client.start()
        self.eventDone.wait()
        PlatformConfig(self.client).instantiate(self)

    def setDoxyTracer(self, doxy):
        self.doxy = doxy

    def cmd(self, cmd):
        req = ["Command",str(cmd)]
        return self.client.send(req)

    def disconnect(self):
        self.client.stop()
        self.client.join()

    def pressButton(self, btn, add_to_doxy=True, comment=""):
        if self.doxy and add_to_doxy:
            self.doxy.pressButton(btn, comment)
        button = getattr(self, btn)
        button.press()
        return button

    def releaseButton(self, btn, add_to_doxy=True, comment=""):
        if self.doxy and add_to_doxy:
            self.doxy.releaseButton(btn, comment)
        button = getattr(self, btn)
        button.release()
        return button

    def clickButton(self, btn, add_to_doxy=True, press_time=150.0, release_time=100.0, comment=""):
        self.pressButton(btn, add_to_doxy=add_to_doxy, comment=comment)
        self.go_msec(press_time)
        self.releaseButton(btn, add_to_doxy=add_to_doxy, comment=comment)
        if release_time:
            self.go_msec(release_time)

    def saveScreenShot(self, comment=""):
        if self.doxy:
            self.doxy.saveScreenShot()

    def log(self, file):
        req = ["Command","log {0}".format(file)]
        return self.client.send(req)
     
    def loadsrec(self, file):
        req = ["Command","loadsrec {0}".format(file)]
        return self.client.send(req)

    def loadmap(self, file):
        req = ["Command","loadmap {0}".format(file)]
        return self.client.send(req)

    def symb2addr(self, symb):
        req = ["Symbol",["ToAddr",symb]]
        return self.client.send(req)

    def br_add(self, location):
        req = ["Breakpoint",["Add",location]]
        return self.client.send(req)

    def br_rm(self, location):
        req = ["Breakpoint",["Remove",location]]
        return self.client.send(req)

    def go(self):
        req = ["Command","c"]
        return self.client.send(req)

    def go_until(self, location):
        req = ["Control",["GoUntil",location]]
        return self.client.send(req)

    def go_msec(self, ms):
        if self.doxy:
            self.doxy.go_msec_before(ms)
        req = ["Control",["GoMsec",float(ms)]]
        ret = self.client.send(req)
        if self.doxy:
            self.doxy.go_msec_after(ms)
        return ret

    def go_substr(self, template, timeout=None):
        """
        Run simulation until debug console output doesn't print string
        containting 'template' substring or timeout happened.
        In a case of timeout exception will be generated.
        """
        ev1 = ConsoleSubStringEvent(template)
        self.client.registerConsoleListener(ev1)
        if not self.isON():
            self.power_on()
        elif self.isHalt():
            self.go()
        ev1.wait(timeout)
        self.halt()
        self.client.unregisterConsoleListener(ev1)

    def halt(self):
        req = ["Command","s"]
        return self.client.send(req)

    def step(self, count):
        if self.doxy:
            self.doxy.step(count)
        req = ["Control",["Step",count]]
        return self.client.send(req)

    def power_on(self):
        req = ["Control",["PowerOn"]]
        return self.client.send(req)

    def power_off(self):
        req = ["Control",["PowerOff"]]
        return self.client.send(req)

    def setSyringe(self, diam, k1, k2):
        req = ["Syringe",[float(diam),k1,k2]]
        return self.client.send(req)

    def isON(self):
        req = ["Status","IsON"]
        return self.client.send(req)

    def isHalt(self):
        req = ["Status","IsHalt"]
        return self.client.send(req)

    def simSteps(self):
        """
        Get simulated number of steps. One step equals to one CPU instruction.
        """
        req = ["Status","Steps"]
        return self.client.send(req)

    def simTimeSec(self):
        """
        Get simulated time in seconds. It's computed as number of steps
        multiplied on Bus frequency in Hz.
        """
        req = ["Status","TimeSec"]
        return self.client.send(req)

    def exit(self):
        self.client.send(['Command', 'halt'])
        self.client.send(['Command', 'exit'])
