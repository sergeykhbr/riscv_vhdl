"""
 @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 @author     Sergey Khabarov - sergeykhbr@gmail.com
 @brief      General thread safe methods.
"""

import sys

def safe_print(content):
    #print "{0}".format(content)
    sys.stdout.write(content + '\n')
