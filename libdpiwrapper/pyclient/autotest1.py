"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Test automation procedure (functional model for now).
"""

from time import sleep
import rpc

p = rpc.DpiServer()
p.connect()

while True:
    resp = p.req_axi4({"addr":0x400, "we":0})
    print "resp={0}".format(resp)
    sleep(0.5)