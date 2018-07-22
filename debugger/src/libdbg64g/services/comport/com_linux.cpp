/** Example:
* http://www.linux-tutorial.info/modules.php?name=Howto&pagename=Serial-Programming-HOWTO/x115.html#AEN125
*/
#include "api_types.h"
#include "api_core.h"
#include "attribute.h"
#include "comport.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>          // standard input/output
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>         // unix standard function definition
#include <string.h>         // strcpy, memset, memcpy
#include <fcntl.h>          // file control definition
#include <termios.h>        // POSIX terminal control definition
#include <errno.h>          // error number definition
#include <sys/ioctl.h>
#include <linux/serial.h>

#include <iostream>
#include <list>

namespace debugger {

using namespace std;

static std::string checkDriverPresence(const std::string& tty) {
    struct stat st;
    std::string devicedir = tty;

    // Append '/device' to the tty-path
    devicedir += "/device";

    // Stat the devicedir and handle it if it is a symlink
    if (lstat(devicedir.c_str(), &st)==0 && S_ISLNK(st.st_mode)) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        // Append '/driver' and return basename of the target
        devicedir += "/driver";

        if (readlink(devicedir.c_str(), buffer, sizeof(buffer)) > 0)
            return basename(buffer);
    }
    return "";
}

/*static void register_comport(list<string>& comList,
                               list<string>& comList8250,
                               const string& dir) {
    // Get the driver the device is using
    std::string driver = checkDriverPresence(dir);

    // Skip devices without a driver
    if (driver.size() > 0) {
        string devfile = string("/dev/") + basename(dir.c_str());

        // Put serial8250-devices in a seperate list
        if (driver == "serial8250") {
            comList8250.push_back(devfile);
        } else
            comList.push_back(devfile); 
    }
}

static void probe_serial8250_comports(list<string>& comList,
                                      list<string> comList8250) {
    struct serial_struct serinfo;
    list<string>::iterator it = comList8250.begin();

    // Iterate over all serial8250-devices
    while (it != comList8250.end()) {

        // Try to open the device
        int fd = open((*it).c_str(), O_RDWR | O_NONBLOCK | O_NOCTTY);

        if (fd >= 0) {
            // Get serial_info
            if (ioctl(fd, TIOCGSERIAL, &serinfo)==0) {
                // If device type is no PORT_UNKNOWN we accept the port
                if (serinfo.type != PORT_UNKNOWN)
                    comList.push_back(*it);
            }
            close(fd);
        }
        it ++;
    }
}*/

/*
 * Enumerate all files in /sys/class/tty
 *
 * For each directory /sys/class/tty/foo, check if /sys/class/tty/foo/device
 * exists using lstat().
 *
 * If it does not exist then you are dealing with some kind of virtual tty
 * device (virtual console port, ptmx, etc...) and you can discard it.
 * If it exists then retain serial port foo.
 */
void ComPortService::getListOfPorts(AttributeType *list) {
    int n;
    struct dirent **namelist;
    const char* sysdir = "/sys/class/tty/";
    AttributeType jsonPortInfo;
    list->make_list(0);

    // Scan through /sys/class/tty - it contains all tty-devices in the system
    n = scandir(sysdir, &namelist, NULL, NULL);
    if (n < 0) {
        RISCV_error("Can't scan '%s' directory", sysdir);
        return;
    } 
    while (n--) {
        if (!strcmp(namelist[n]->d_name, "..")
         || !strcmp(namelist[n]->d_name, ".")) {
            free(namelist[n]);
            continue;
        }

        // Construct full absolute file path
        std::string devicedir = sysdir;
        devicedir += namelist[n]->d_name;

        // Get the driver the device is using.
        // Skip devices without a driver
        std::string driver = checkDriverPresence(devicedir);
        if (driver.size() <= 0) {
            continue;
        }
        std::string devfile = std::string("/dev/") + basename(devicedir.c_str());

        jsonPortInfo.make_dict();
        jsonPortInfo["id"] = AttributeType(devfile.c_str());
        list->add_to_list(&jsonPortInfo);
        free(namelist[n]);
    }
    free(namelist);

    // Only non-serial8250 has been added to comList without any further testing
    // serial8250-devices must be probe to check for validity
    //probe_serial8250_comports(comList, comList8250);
}


int ComPortService::openPort(const char *port, AttributeType settings) {
    closePort();
    int fd = open(port, O_RDWR | O_NOCTTY);// | O_NONBLOCK);// | O_NDELAY );
    //fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
    if (fd < 0) {
        RISCV_error("fopen() failed", NULL);
        return -1;
    }

    struct termios options;
    memset(&options, 0, sizeof(options));

    /**
     *    struct termios {
     *        tcflag_t  c_iflag;       // input modes
     *        tcflag_t  c_oflag;       // output modes
     *        tcflag_t  c_cflag;       // control modes
     *        tcflag_t  c_lflag;       // local modes
     *        speed_t   c_ispeed;      // input speed
     *        speed_t   c_ospeed;      // output speed
     *        cc_t      c_cc[NCCS];    // control characters
     *    };
     * Input flags c_iflags:
     *    ICRNL  - map CR to NL on input (NL = LF)
     *    IGNCR  - ignore CR
     *    INLCR  - map NL to CR
     *    IXON   - enable start/stop output control
     *    IXOFF  - enable start/stop input control
     *    IXANY  - allow any character to restart output
     *    ISTRIP - strip character to seven bits
     *    IGNPAR - ignore characters withparity error
     *    INPCK  - enable input parity checking
     *    PARMRK - mark parity error by inserting '\377', '\0'
     *    BRKINT - send SIGINT to the terminal when receiving break condition
     *    IGNBRK - ignore break condition
     */
    options.c_iflag = IGNPAR;

    /* Output mode c_oflag:
     *    OPOST  - perform output processing
     *    ONCLR  - transform NL to CR NL
     *    XTABS  - Transform TAB into spaces
     *    ONOEOT - discard EOT (^D) character
     */
    options.c_oflag = 0;          //

    /* Control modes c_cflag:
     *    CLOCAL - ignore modem status lines
     *    CREAD  - enable receiver
     *    CSIZE  - number of bits mask. Possible values: CS5, CS6, CS7, CS8
     *    CSTOPB - send 2 stop bits instead of one.
     *    PARENB - enable parity generation
     *    PARODD - generate odd parity if parity is generated
     *    HUPCL  - drop modem control lines on the last close of the terminal line
     */
    options.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;

    /* Local modes c_lflag:
     *    ECHO   - enable echoing of input characters
     *    ECHOE  - if ICANNON an ECHO are set then echo ERASE and KILL as one
     *             or more backspace-space-backspace sequences (to wipe entire
     *             line)
     *    ECHOK  - output an NL after the KILL character
     *    ECHONL - echo NL even if ECHO is not set
     *    ICANON - cannonical input. This enables line oriented input (not raw case).
     *    IEXTEN - enable implementation defined input extensions
     *    ISIG   - enable signal character INTR, QUIT, SUSP
     *    NOFLSH - disable flushing of the input/output queues that is normally
     *             done if a signal is sent
     *    TOSTOP - send a SIGTTOU if job control is implemented
     */
    options.c_lflag = 0;          // no signaling chars, no echo

    /* Available fields:
     *    VEOF, VEOL, VERASE, VINTR, VKILL, VMIN, VQUIT, VTIME, VSUSP, VSTART,
     *    VSTOP, VREPRINT, VLNEXT and VDISCARD
     */
    options.c_cc[VTIME] = 0;      // inter-character timer unesed
    options.c_cc[VMIN] = 0;       // blocking read until 5 chars received


    tcflush(fd, TCIOFLUSH);
    // TCSANOW   - change occurs immediatly
    // TCSADRAIN - change occurs after all parameters are written
    // TCSAFLUSH - ..
    if(tcsetattr(fd, TCSANOW, &options) == -1) {
        RISCV_error("tcsetattr() failed", NULL);
        return -1;
    }

    // Empty buffers
    tcflush(fd, TCIOFLUSH);
    prtHandler_ = fd;
    return 0 ;
}

void ComPortService::closePort() {
    if (prtHandler_) {
        close(prtHandler_);
    }
    prtHandler_ = 0;
}

int ComPortService::readSerialPort(void *hdl, char *buf, int bufsz) {
    int sz = read(*((int *)hdl), buf, bufsz-1);
    buf[sz] = 0;
    return sz;
}

int ComPortService::writeSerialPort(void *hdl, char *buf, int bufsz) {
    return write(*((int *)hdl), buf, bufsz);
}

void ComPortService::cleanSerialPort(void *hdl) {
    tcflush(*((int *)hdl), TCIOFLUSH);
}

}  // namespace debugger
