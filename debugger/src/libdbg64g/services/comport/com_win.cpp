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

void ComPortService::previousProps(void *hdl) {
    HANDLE hFile = *reinterpret_cast<HANDLE*>(hdl);
    COMMPROP CommProp;
    DCB dcb;
    COMMTIMEOUTS touts;
    COMSTAT comStat;
    DWORD   dwErrors;
    DWORD dwModemStatus;

    GetCommProperties(hFile, &CommProp);
    GetCommState(hFile, &dcb);
    GetCommTimeouts(hFile, &touts);
    ClearCommError(hFile, &dwErrors, &comStat);
    GetCommModemStatus(hFile, &dwModemStatus);

#if 1
    RISCV_printf0("DWORD fCtsHold : 1 =%x", comStat.fCtsHold);
    RISCV_printf0("DWORD fDsrHold : 1 =%x", comStat.fDsrHold);
    RISCV_printf0("DWORD fRlsdHold : 1 =%x", comStat.fRlsdHold);
    RISCV_printf0("DWORD fXoffHold : 1 =%x", comStat.fXoffHold);
    RISCV_printf0("DWORD fXoffSent : 1 =%x", comStat.fXoffSent);
    RISCV_printf0("DWORD fEof : 1 =%x", comStat.fEof);
    RISCV_printf0("DWORD fTxim : 1 =%x", comStat.fTxim);
    RISCV_printf0("DWORD fReserved : 25 =%x", comStat.fReserved);
    RISCV_printf0("DWORD cbInQue =%d", comStat.cbInQue);
    RISCV_printf0("DWORD cbOutQue =%d", comStat.cbOutQue);

    RISCV_printf0("dwModemStatus =%08x", dwModemStatus);

#else
    RISCV_printf0("CommProp.wPacketLength=%x", (int)CommProp.wPacketLength);
    RISCV_printf0("CommProp.wPacketVersion=%x", (int)CommProp.wPacketVersion);
    RISCV_printf0("CommProp.dwServiceMask=%x", CommProp.dwServiceMask);
    RISCV_printf0("CommProp.dwReserved1=%x", CommProp.dwReserved1);
    RISCV_printf0("CommProp.dwMaxTxQueue=%x", CommProp.dwMaxTxQueue);
    RISCV_printf0("CommProp.dwMaxRxQueue=%x", CommProp.dwMaxRxQueue);
    RISCV_printf0("CommProp.dwMaxBaud=%x", CommProp.dwMaxBaud);
    RISCV_printf0("CommProp.dwProvSubType=%x", CommProp.dwProvSubType);
    RISCV_printf0("CommProp.dwProvCapabilities=%x", CommProp.dwProvCapabilities);
    RISCV_printf0("CommProp.dwSettableParams=%x", CommProp.dwSettableParams);
    RISCV_printf0("CommProp.dwSettableBaud=%x", CommProp.dwSettableBaud);
    RISCV_printf0("CommProp.wSettableData=%x", (int)CommProp.wSettableData);
    RISCV_printf0("CommProp.wSettableStopParity=%x", (int)CommProp.wSettableStopParity);
    RISCV_printf0("CommProp.dwCurrentTxQueue=%x", CommProp.dwCurrentTxQueue);
    RISCV_printf0("CommProp.dwCurrentRxQueue=%x", CommProp.dwCurrentRxQueue);
    RISCV_printf0("CommProp.dwProvSpec1=%x", CommProp.dwProvSpec1);
    RISCV_printf0("CommProp.dwProvSpec2=%x", CommProp.dwProvSpec2);
    RISCV_printf0("CommProp.wcProvChar[0]=%x", (int)CommProp.wcProvChar[0]);

    RISCV_printf0("DWORD DCBlength=%x", dcb.DCBlength);      /* sizeof(DCB)                     */
    RISCV_printf0("DWORD BaudRate=%x", dcb.BaudRate);       /* Baudrate at which running       */
    RISCV_printf0("DWORD fBinary: 1=%x", dcb.fBinary);     /* Binary Mode (skip EOF check)    */
    RISCV_printf0("DWORD fParity: 1=%x", dcb.fParity);     /* Enable parity checking          */
    RISCV_printf0("DWORD fOutxCtsFlow:1=%x", dcb.fOutxCtsFlow); /* CTS handshaking on output       */
    RISCV_printf0("DWORD fOutxDsrFlow:1=%x", dcb.fOutxDsrFlow); /* DSR handshaking on output       */
    RISCV_printf0("DWORD fDtrControl:2=%x", dcb.fDtrControl);  /* DTR Flow control                */
    RISCV_printf0("DWORD fDsrSensitivity:1=%x", dcb.fDsrSensitivity); /* DSR Sensitivity              */
    RISCV_printf0("DWORD fTXContinueOnXoff: 1=%x", dcb.fTXContinueOnXoff); /* Continue TX when Xoff sent */
    RISCV_printf0("DWORD fOutX: 1=%x", dcb.fOutX);       /* Enable output X-ON/X-OFF        */
    RISCV_printf0("DWORD fInX: 1=%x", dcb.fInX);        /* Enable input X-ON/X-OFF         */
    RISCV_printf0("DWORD fErrorChar: 1=%x", dcb.fErrorChar);  /* Enable Err Replacement          */
    RISCV_printf0("DWORD fNull: 1=%x", dcb.fNull);       /* Enable Null stripping           */
    RISCV_printf0("DWORD fRtsControl:2=%x", dcb.fRtsControl);  /* Rts Flow control                */
    RISCV_printf0("DWORD fAbortOnError:1=%x", dcb.fAbortOnError); /* Abort all reads and writes on Error */
    RISCV_printf0("DWORD fDummy2:17=%x", dcb.fDummy2);     /* Reserved                        */
    RISCV_printf0("WORD wReserved=%x", dcb.wReserved);       /* Not currently used              */
    RISCV_printf0("WORD XonLim=%x", dcb.XonLim);          /* Transmit X-ON threshold         */
    RISCV_printf0("WORD XoffLim=%x", dcb.XoffLim);         /* Transmit X-OFF threshold        */
    RISCV_printf0("BYTE ByteSize=%x", dcb.ByteSize);        /* Number of bits/byte, 4-8        */
    RISCV_printf0("BYTE Parity=%x", dcb.Parity);          /* 0-4=None,Odd,Even,Mark,Space    */
    RISCV_printf0("BYTE StopBits=%x", dcb.StopBits);        /* 0,1,2 = 1, 1.5, 2               */
    RISCV_printf0("char XonChar=%x", dcb.XonChar);         /* Tx and Rx X-ON character        */
    RISCV_printf0("char XoffChar=%x", dcb.XoffChar);        /* Tx and Rx X-OFF character       */
    RISCV_printf0("char ErrorChar=%x", dcb.ErrorChar);       /* Error replacement char          */
    RISCV_printf0("char EofChar=%x", dcb.EofChar);         /* End of Input character          */
    RISCV_printf0("char EvtChar=%x", dcb.EvtChar);         /* Received Event character        */
    RISCV_printf0("WORD wReserved1=%x", dcb.wReserved1);      /* Fill for now.                   */

    RISCV_printf0("DWORD ReadIntervalTimeout=%x", touts.ReadIntervalTimeout);          /* Maximum time between read chars. */
    RISCV_printf0("DWORD ReadTotalTimeoutMultiplier=%x", touts.ReadTotalTimeoutMultiplier);   /* Multiplier of characters.        */
    RISCV_printf0("DWORD ReadTotalTimeoutConstant=%d", touts.ReadTotalTimeoutConstant);     /* Constant in milliseconds.        */
    RISCV_printf0("DWORD WriteTotalTimeoutMultiplier=%x", touts.WriteTotalTimeoutMultiplier);  /* Multiplier of characters.        */
    RISCV_printf0("DWORD WriteTotalTimeoutConstant=%x", touts.WriteTotalTimeoutConstant);    /* Constant in milliseconds.        */
#endif
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
    RISCV_sprintf(chConfig, sizeof(chConfig), "baud=%d parity=N data=8 stop=1", baud);
 
    hFile = CreateFile(chCom,
                        GENERIC_READ|GENERIC_WRITE,
                        0,//FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,						
	                    //OPEN_ALWAYS,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
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
    GetCommProperties(hFile, &CommProp);
    FillMemory(&dcb, sizeof(dcb), 0);

    dcb.DCBlength = sizeof(dcb);
    if (!BuildCommDCB(chConfig, &dcb)) {   
        RISCV_error("Can't BuildCommDCB(%s,)", chConfig);
        CloseHandle(hFile);
        return -1;
    }
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(hFile, &dcb)) {
        RISCV_error("Can't set port %s state", chCom);
        CloseHandle(hFile);
        return -1;
    }

    COMMTIMEOUTS CommTimeOuts;
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
    CommTimeOuts.ReadTotalTimeoutConstant    = 500;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant   = 0;//1000;
#endif

    if(!SetCommTimeouts(hFile, &CommTimeOuts)) {
        RISCV_error("Can't set port %s timeouts", chCom);
        CloseHandle(hFile);
        return -1;
    }
    
    DWORD dwStoredFlags = EV_BREAK | EV_CTS  | EV_DSR | EV_ERR | EV_RING |
                EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY;
    if(!SetCommMask(hFile, dwStoredFlags)) {
        RISCV_error("Can't set mask %s", chCom);
        CloseHandle(hFile);
        return -1;
    }

    //SetupComm(hFile, 0,0);

    RISCV_info("Serial port %s opened", chCom);

    PurgeComm(hFile, PURGE_TXCLEAR|PURGE_TXABORT);
    PurgeComm(hFile, PURGE_RXCLEAR|PURGE_RXABORT);

    previousProps(hdl);

    return 0;
}

void ComPortService::closeSerialPort(void *hdl) {
    CloseHandle(*static_cast<HANDLE *>(hdl));
}

int ComPortService::readSerialPort(void *hdl, char *buf, int bufsz) {
    HANDLE hFile = *static_cast<HANDLE *>(hdl);
    DWORD dwBytesRead;
    BOOL success = ReadFile(hFile, buf, bufsz, &dwBytesRead, NULL);
    if (!success) {
        return -1;
    }
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
