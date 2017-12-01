"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      Test automation procedure (functional model for now).
"""

import sys,rpc
pump = rpc.Simulator()
pump.connect()

pump.halt()

pump.step(3)
pump.go_msec(50.0)
pump.simSteps()
pump.simTimeSec()

pump.disconnect()
