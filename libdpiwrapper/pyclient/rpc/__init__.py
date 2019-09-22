"""
  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
      http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
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

