#include "api_types.h"
#include "api_core.h"
#include "attribute.h"
#include "comport.h"
#include <winspool.h>
#include <cstdlib>

namespace debugger {

void ComPortService::getSerialPortList(AttributeType *list) {
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
    char temp[4] = {0};
    int port = -1;
    PORT_INFO_1 *pPortInfo;
    pPortInfo = (PORT_INFO_1 *)lpPorts;

 
    char comName[16];
    char chCom[20];
    size_t szComLen;
    for (dw = 0; dw < Ports_Count; dw++) {
    
        if (strstr(pPortInfo->pName, "com") == 0) {
            continue;
        }
        temp[0] = pPortInfo->pName[3];
		if (pPortInfo->pName[4] != ':' && pPortInfo->pName[4] != '\0') {
			temp[1] = pPortInfo->pName[4];
        }
        if (pPortInfo->pName[5] != ':' && pPortInfo->pName[5] != '\0') {
		    temp[2] = pPortInfo->pName[5];
        }

		port = strtoul(temp, NULL, 0);;
        szComLen = RISCV_sprintf(chCom, sizeof(chCom), "\\\\.\\COM%d", port) + 1;


		HANDLE h = CreateFile(chCom, GENERIC_READ | GENERIC_WRITE, 
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

int ComPortService::openSerialPort(const char *port, int baud, void *hdl) {
    char chCom[20];
    char chConfig[64];
    HANDLE hFile;
  
    size_t szComLen = RISCV_sprintf(chCom, sizeof(chCom), "\\\\.\\%s", port) + 1;
    /** COMx[:][baud=b][parity=p][data=d][stop=s][to={on|off}]
     *         [xon={on|off}][odsr={on|off}][octs={on|off}][dtr={on|off|hs}]
     *         [rts={on|off|hs|tg}][idsr={on|off}]
     */
    // baud=115200 parity=N data=8 stop=1
    RISCV_sprintf(chConfig, sizeof(chConfig), "%d:n,8,1", baud);
 
    hFile = CreateFile(chCom,
                        GENERIC_READ|GENERIC_WRITE,
                        0,//FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,						
	                    OPEN_ALWAYS,//OPEN_EXISTING,
                        0,//FILE_ATTRIBUTE_NORMAL,
                        NULL);
 	
    *static_cast<HANDLE *>(hdl) = hFile;
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_ACCESS_DENIED) {
            RISCV_error("%s is locked by another device", chCom);
        } else {
            RISCV_error("Can't open port %s", chCom);
        }
	    return -1;
    }

    // Read capabilities:
    COMMPROP CommProp;
    DCB dcb;
    GetCommProperties(hFile, &CommProp );
    FillMemory(&dcb, sizeof(dcb), 0);

    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB(chConfig, &dcb)) {   
        RISCV_error("Can't BuildCommDCB(%s,)", chConfig);
        CloseHandle(hFile);
        return -1;
    }

    if (!SetCommState(hFile, &dcb)) {
        RISCV_error("Can't set port %s state", chCom);
        CloseHandle(hFile);
        return -1;
    }

    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout		 = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier  = MAXDWORD;//0;
    CommTimeOuts.ReadTotalTimeoutConstant    = 500;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant   = 0;//1000;

    SetCommTimeouts(hFile, &CommTimeOuts);
    if(!SetCommMask(hFile, EV_RXCHAR)) {
        RISCV_error("Can't set port %s timeouts", chCom);
        CloseHandle(hFile);
        return -1;
    }

    RISCV_info("Serial port %s opened", chCom);

    PurgeComm(hFile, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT);
    return 0;
}

void ComPortService::closeSerialPort(void *hdl) {
    CloseHandle(*static_cast<HANDLE *>(hdl));
}

int ComPortService::readSerialPort(void *hdl, char *buf, int bufsz) {
    DWORD dwBytesRead;
    ReadFile(*static_cast<HANDLE *>(hdl), buf, bufsz, &dwBytesRead, NULL);
    return (int)dwBytesRead;
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
