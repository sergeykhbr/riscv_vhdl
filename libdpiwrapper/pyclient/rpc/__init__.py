"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Simulator's Python API implementation.
"""

import threading
import client

class DpiServer(object):
    def __init__(self):
        self.client = None
        self.eventDone = threading.Event()

    def connect(self):
        self.eventDone.clear()
        self.client = client.TcpClient("dpiclient", self.eventDone)
        self.client.start()
        self.eventDone.wait()

    def req_axi4(self, tr):
        req = ["DpiClient",'AXI4',tr]
        return self.client.send(req)

    def disconnect(self):
        self.client.stop()
        self.client.join()

