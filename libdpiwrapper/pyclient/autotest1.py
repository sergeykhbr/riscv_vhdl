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

from time import sleep
import rpc

p = rpc.DpiServer()
p.connect()

i = 0
while True:
    # bytes possible values: 1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32
    #resp = p.req_axi4({"addr":0x0 + 2*i, "we":1, "bytes":2, "wdata":[0x2211]})
    resp = p.req_axi4({"addr":0x0 + 2*i, "we":0, "bytes":2})
    print "resp={0}".format(resp)
    i = i + 1
    #sleep(0.5)
