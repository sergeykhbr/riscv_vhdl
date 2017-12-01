"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Simulator's Python API implementation.
"""

import threading
import client
from events import ConsoleSubStringEvent

class Simulator(object):
    def __init__(self):
        self.client = None
        self.eventDone = threading.Event()

    def connect(self):
        self.eventDone.clear()
        self.client = client.TcpClient("rpcclient", self.eventDone)
        self.client.start()
        self.eventDone.wait()

    def disconnect(self):
        self.client.stop()
        self.client.join()

    def log(self, file):
        req = ["Command","log {0}".format(file)]
        return self.client.send(req)
     
    def loadsrec(self, file):
        req = ["Command","loadsrec {0}".format(file)]
        return self.client.send(req)

    def loadmap(self, file):
        req = ["Command","loadmap {0}".format(file)]
        return self.client.send(req)

    def pressButton(self, btn):
        req = ["Button",["Press",btn]]
        return self.client.send(req)

    def releaseButton(self, btn):
        req = ["Button",["Release",btn]]
        return self.client.send(req)

    def clickButton(self, btn):
        self.pressButton(btn)
        self.go_msec(50.0)
        self.releaseButton(btn)
        self.go_msec(50.0)

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
        req = ["Control",["GoMsec",float(ms)]]
        return self.client.send(req)

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

    def microphone(self, ena):
        """
        Enable\Disable integrated microphone.
        """
        req = ["Control",["Microphone",bool(ena)]]
        return self.client.send(req)

    def battery(self, voltage=None):
        """
        Request battery level when voltage equals to None or 
        Change battery level in Volts
        """
        if voltage:
            req = ["Control",["Battery",float(voltage)]]
        else:
            req = ["Control",["Battery"]]
        return self.client.send(req)

    def getIndicator(self):
        """
        Read state of the User's indicator
        """
        req = ["Attribute",["userindicator","State"]]
        resp = self.client.send(req)
        if resp in [0,1,2,3]:
             Colors = ["OFF", "RED", "GREEN", "YELLOW"]
             return Colors[resp]
        else:
             return resp
