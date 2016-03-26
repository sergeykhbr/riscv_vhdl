/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      elf-file loader class implementation.
 */

#include "console.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "api_types.h"

namespace debugger {

/** Class registration in the Core */
static ConsoleServiceClass local_class_;

static const int STDIN = 0;

ConsoleService::ConsoleService(const char *name) : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IConsole *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Consumer", &consumer_);

    isEnable_.make_boolean(true);
    consumer_.make_string("");
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    struct termios new_settings;
    tcgetattr(0, &original_settings_);
    new_settings = original_settings_;
     
    /// Disable canonical mode, and set buffer size to 1 byte
    new_settings.c_lflag &= ~(ICANON | ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 0;
     
    tcsetattr(STDIN, TCSANOW, &new_settings);
    term_fd_ = fileno(stdin);
#endif
}

ConsoleService::~ConsoleService() {
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    tcsetattr(STDIN, TCSANOW, &original_settings_);
#endif
}

void ConsoleService::postinitService() {
    iconsumer_ = static_cast<IKeyListener *>
            (RISCV_get_service_iface(consumer_.to_string(),
                                    IFACE_KEY_LISTENER));
    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }

    // Redirect output stream to a this console
    RISCV_set_default_output(static_cast<IConsole *>(this));
    update();
}

void ConsoleService::predeleteService() {
    stop();
}

void ConsoleService::busyLoop() {
    while (loopEnable_ && !interrupt_) {
        if (isData()) {
            iconsumer_->keyUp(getData());
        }
        RISCV_sleep_ms(50);
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

void ConsoleService::writeBuffer(const char *buf) {
    clearLine();
    std::cout << buf;
    update();
}

void ConsoleService::writeCommand(const char *cmd) {
    clearLine();
    std::cout << "riscv# " << cmd << "\r\n";
}

void ConsoleService::setCmdString(const char *buf) {
    cmdLine_ = std::string(buf);
    update();
}

void ConsoleService::update() {
    clearLine();
    std::cout << "riscv# " << cmdLine_.c_str();
    std::cout.flush();
}

void ConsoleService::clearLine() {
    std::cout << "\r                                                      \r";
}

int ConsoleService::registerKeyListener(IFace *iface) {
    if (strcmp(iface->getFaceName(), IFACE_KEY_LISTENER)) {
        RISCV_error("Wrong interface '%s'", iface->getFaceName());
        return -1;
    }
    AttributeType t1(iface);
    keyListeners_.add_to_list(&t1);
    return 0;
}

bool ConsoleService::isData() {
#if defined(_WIN32) || defined(__CYGWIN__)
    return _kbhit() ? true: false;
#else
    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting != 0;
#endif
}

int ConsoleService::getData() {
#if defined(_WIN32) || defined(__CYGWIN__)
    return _getch();
#else
   unsigned char ch;
   //int err = 
   read(term_fd_, &ch, sizeof(ch));
   return ch;
    //return getchar();
#endif
}

}  // namespace debugger
