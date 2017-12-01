"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Simulator API asynchronous events implementation.
"""

import threading

class ConsoleSubStringEvent(object):
    def __init__(self, template):
        self.template = template
        self.event = threading.Event()
        self.event.clear()

    def callback(self, s):
        if self.template in s:
             self.event.set()

    def wait(self, sec=None):
        if self.event.wait(sec) != True:
             raise ValueError('ConsoleSubString timeout')
