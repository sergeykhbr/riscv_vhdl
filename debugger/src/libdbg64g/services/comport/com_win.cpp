/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "api_types.h"
#include "api_core.h"
#include "attribute.h"
#include "comport.h"
#include <winspool.h>
#include <cstdlib>

namespace debugger {

void ComPortService::getListOfPorts(AttributeType *list) {
    list->make_list(0);

    AttributeType portInfo;
    DWORD  Ports_MemSize = 0;
    DWORD  Ports_Count   = 0;
    BYTE*  lpPorts       = NULL;

    //Getting Ports_MemSize value...
    EnumPorts(NULL,
              1,
              lpPorts,
              0,
              &Ports_MemSize,
              &Ports_Count);


    //Getting lpPorts...
    lpPorts = new BYTE[Ports_MemSize];
    EnumPorts(NULL,
              1,
              lpPorts,
              Ports_MemSize,
              &Ports_MemSize,
              &Ports_Count);


    //Forming List Of Ports...
    DWORD dw;
    wchar_t temp[4] = {0};
    int port = -1;
    PORT_INFO_1 *pPortInfo;
    pPortInfo = (PORT_INFO_1 *)lpPorts;

 
    char comName[16];
    char chCom[20];
    wchar_t wchCom[20];
    size_t szComLen;
    size_t converted;
    for (dw = 0; dw < Ports_Count; dw++) {
    
        if (wcsstr(pPortInfo->pName, L"com") == 0) {
            continue;
        }
        temp[0] = pPortInfo->pName[3];
		if (pPortInfo->pName[4] != ':' && pPortInfo->pName[4] != '\0') {
			temp[1] = pPortInfo->pName[4];
        }
        if (pPortInfo->pName[5] != ':' && pPortInfo->pName[5] != '\0') {
		    temp[2] = pPortInfo->pName[5];
        }

		port = wcstoul(temp, NULL, 0);;
        szComLen = RISCV_sprintf(chCom, sizeof(chCom),
                                 "\\\\.\\COM%d", port) + 1;
        mbstowcs_s(&converted, wchCom, chCom, sizeof(chCom));

		HANDLE h = CreateFileW(wchCom, GENERIC_READ | GENERIC_WRITE, 
                              0, NULL, OPEN_EXISTING, 
                              FILE_ATTRIBUTE_NORMAL,NULL);

		if (h == INVALID_HANDLE_VALUE) {
            pPortInfo++;
		    continue;
		}

        portInfo.make_dict();
        RISCV_sprintf(comName, sizeof(comName), "'COM%d'", port);
        portInfo["id"].make_string(comName);
		list->add_to_list(&portInfo);
        
        CloseHandle(h);
        pPortInfo++;
    }
  
    delete [] lpPorts;
}

int ComPortService::openPort(const char *port, AttributeType settings) {
    char chCom[20];
    wchar_t wchCom[20];
    char chConfig[64];
    wchar_t wchConfig[64];
    HANDLE hFile;
    COMMPROP CommProp;
    DCB dcb;
    COMMTIMEOUTS CommTimeOuts;
    DWORD dwStoredFlags;
    DWORD  Errors;
    COMSTAT  Stat;
    size_t converted;
  
    RISCV_sprintf(chCom, sizeof(chCom), "\\\\.\\%s", port);
    mbstowcs_s(&converted, wchCom, chCom, sizeof(chCom));
    RISCV_sprintf(chConfig, sizeof(chConfig),
                  "baud=%d parity=N data=8 stop=1", settings[0u].to_int());
    mbstowcs_s(&converted, wchConfig, chConfig, sizeof(chConfig));
 
    hFile = CreateFileW(wchCom,
                        GENERIC_READ|GENERIC_WRITE,
                        0,//FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,						
	                    //OPEN_ALWAYS,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
 	
    prtHandler_ = hFile;
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_ACCESS_DENIED) {
            RISCV_error("%s is locked by another device", chCom);
        } else {
            RISCV_error("Can't open port %s", chCom);
        }
	    return -1;
    }

    // Read capabilities:
    GetCommProperties(hFile, &CommProp);
    FillMemory(&dcb, sizeof(dcb), 0);

    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCBW(wchConfig, &dcb)) {   
        RISCV_error("Can't BuildCommDCB(%s,)", chConfig);
        CloseHandle(hFile);
        return -1;
    }
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    Sleep(100);
    if (!SetCommState(hFile, &dcb)) {
        RISCV_error("Can't set port %s state", chCom);
        CloseHandle(hFile);
        return -1;
    }

#if 0
    /** ...A value of MAXDWORD , combined with zero values for both 
     * the ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier members,
     * specifies that the read operation is to return immediately with the
     * characters that have already been received, even if no characters 
     * have been received...
     **/
    CommTimeOuts.ReadIntervalTimeout		 = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier  = 0;
    CommTimeOuts.ReadTotalTimeoutConstant    = 0;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant   = 0;
#else
    CommTimeOuts.ReadIntervalTimeout		 = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier  = MAXDWORD;//0;
    CommTimeOuts.ReadTotalTimeoutConstant    = 100;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant   = 0;//1000;
#endif

    if(!SetCommTimeouts(hFile, &CommTimeOuts)) {
        RISCV_error("Can't set port %s timeouts", chCom);
        CloseHandle(hFile);
        return -1;
    }
    
    dwStoredFlags = EV_BREAK | EV_CTS  | EV_DSR | EV_ERR | EV_RING |
                EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY;
    if(!SetCommMask(hFile, dwStoredFlags)) {
        RISCV_error("Can't set mask %s", chCom);
        CloseHandle(hFile);
        return -1;
    }

    RISCV_info("Serial port %s opened", chCom);

    RISCV_sleep_ms(100);
    ClearCommError(hFile, &Errors, &Stat);
    PurgeComm(hFile, PURGE_RXCLEAR | PURGE_TXCLEAR);
    PurgeComm(hFile, PURGE_RXABORT | PURGE_TXABORT);

    return 0;
}

void ComPortService::closePort() {
    if (prtHandler_) {
        CloseHandle(*static_cast<HANDLE *>(prtHandler_));
    }
    prtHandler_ = 0;
}

int ComPortService::readSerialPort(void *hdl, char *buf, int bufsz) {
    HANDLE hFile = *static_cast<HANDLE *>(hdl);
    DWORD dwBytesRead;
    BOOL success = ReadFile(hFile, buf, bufsz, &dwBytesRead, NULL);
    if (!success) {
        return -1;
    }
    buf[dwBytesRead] = 0;
    return static_cast<int>(dwBytesRead);
}

int ComPortService::writeSerialPort(void *hdl, char *buf, int bufsz) {
    DWORD lpdwBytesWrittens;
    WriteFile(*static_cast<HANDLE *>(hdl), 
                buf, bufsz, &lpdwBytesWrittens, NULL);
    return (int)lpdwBytesWrittens;
}

void ComPortService::cleanSerialPort(void *hdl) {
    PurgeComm(*static_cast<HANDLE *>(hdl), PURGE_TXCLEAR|PURGE_RXCLEAR);
}

}  // namespace debugger
