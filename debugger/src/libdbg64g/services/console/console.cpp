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
/*
static struct termios stored_settings;
     
void set_keypress(void) {
    struct termios new_settings;
     
    tcgetattr(0,&stored_settings);
     
    new_settings = stored_settings;
     
    /// Disable canonical mode, and set buffer size to 1 byte
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
     
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}
     
void reset_keypress(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}
*/

ConsoleService::ConsoleService(const char *name) : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IConsole *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Consumer", &consumer_);

    isEnable_.make_boolean(true);
    consumer_.make_string("");
}

ConsoleService::~ConsoleService() {
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
    threadInit_.Handle = NULL;
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
#endif
}

int ConsoleService::getData() {
#if defined(_WIN32) || defined(__CYGWIN__)
    return _getch();
#else
#endif
}

}  // namespace debugger
