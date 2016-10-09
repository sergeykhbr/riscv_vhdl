#include "api_types.h"
#include "api_core.h"
#include "attribute.h"
#include <winspool.h>
#include <cstdlib>

namespace debugger {

void getSerialPortList(AttributeType *list) {
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

int openSerialPort(const char *port, int baud, void *hdl) {
    char chCom[20];
    HANDLE hFile;
  
    size_t szComLen = RISCV_sprintf(chCom, sizeof(chCom), "\\\\.\\%s", port) + 1;
 
    hFile = CreateFile(chCom,
                        GENERIC_READ|GENERIC_WRITE,
                        0,//FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,						
	                    OPEN_ALWAYS,//OPEN_EXISTING,
                        0,//FILE_ATTRIBUTE_NORMAL,
                        NULL);
 	
    *static_cast<HANDLE *>(hdl) = hFile;
    if (hFile == INVALID_HANDLE_VALUE) {
	    return -1;
    }

    // Read capabilities:
    COMMPROP CommProp;
    GetCommProperties(hFile, &CommProp );

    DCB dcb;
    GetCommState(hFile, &dcb);
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    // Parse string with the paramaters of COM connection to the dcb structure:
    //????BuildCommDCB(); 
    dcb.BaudRate = baud;
    dcb.ByteSize = 8;             
    dcb.Parity   = NOPARITY;        
    dcb.StopBits = ONESTOPBIT;   

    if (!SetCommState(hFile, &dcb)) {
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
        return -1;
    }

    PurgeComm(hFile, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT);
    return 0;
}

void closeSerialPort(void *hdl) {
    CloseHandle(*static_cast<HANDLE *>(hdl));
}

int readSerialPort(void *hdl, char *buf, int bufsz) {
    DWORD dwBytesRead;
    ReadFile(*static_cast<HANDLE *>(hdl), buf, bufsz, &dwBytesRead, NULL);
    return (int)dwBytesRead;
}

int writeSerialPort(void *hdl, char *buf, int bufsz) {
    DWORD lpdwBytesWrittens;
    WriteFile(*static_cast<HANDLE *>(hdl), 
                buf, bufsz, &lpdwBytesWrittens, NULL);
    return (int)lpdwBytesWrittens;
}

void cleanSerialPort(void *hdl) {
    PurgeComm(*static_cast<HANDLE *>(hdl), PURGE_TXCLEAR|PURGE_RXCLEAR);
}

}  // namespace debugger
