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
REGISTER_CLASS(ConsoleService)

static const int STDIN = 0;

ConsoleService::ConsoleService(const char *name) 
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IConsole *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Consumer", &consumer_);
    registerAttribute("LogFile", &logFile_);
    registerAttribute("Serial", &serial_);

    RISCV_mutex_init(&mutexConsoleOutput_);
    RISCV_event_create(&config_done_, "config_done");
    RISCV_register_hap(static_cast<IHap *>(this));

    isEnable_.make_boolean(true);
    consumer_.make_string("");
    logFile_.make_string("");
    serial_.make_string("");

    logfile_ = NULL;
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
    if (logfile_) {
        fclose(logfile_);
    }
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    tcsetattr(STDIN, TCSANOW, &original_settings_);
#endif
    RISCV_event_close(&config_done_);
    RISCV_mutex_destroy(&mutexConsoleOutput_);
}

void ConsoleService::postinitService() {
    ISerial *uart = static_cast<ISerial *>
            (RISCV_get_service_iface(serial_.to_string(), IFACE_SERIAL));
    if (uart) {
        uart->registerRawListener(static_cast<IRawListener *>(this));
    }

    iconsumer_ = static_cast<IKeyListener *>
            (RISCV_get_service_iface(consumer_.to_string(),
                                    IFACE_KEY_LISTENER));
    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
    if (logFile_.size()) {
        enableLogFile(logFile_.to_string());
    }

    // Redirect output stream to a this console
    RISCV_set_default_output(static_cast<IConsole *>(this));
    std::cout << "riscv# " << cmdLine_.c_str();
    std::cout.flush();
}

void ConsoleService::predeleteService() {
    stop();
}

void ConsoleService::hapTriggered(EHapType type) {
    RISCV_event_set(&config_done_);
}

void ConsoleService::busyLoop() {
    RISCV_event_wait(&config_done_);

    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["ScriptFile"].size() > 0) {
        const char *script_name = (*glb)["ScriptFile"].to_string();
        FILE *script = fopen(script_name, "r");
        if (!script) {
            RISCV_error("Script file '%s' not found", script_name);
        } else if (iconsumer_) {
            bool comment = false;
            bool crlf = false;
            char symb[2];
            while (!feof(script)) {
                fread(symb, 1, 1, script);
                if (symb[0] == '/') {
                    fread(&symb[1], 1, 1, script);
                    if (symb[1] == '/') {
                        comment = true;
                    } else {
                        fseek(script, -1, SEEK_CUR);
                    }
                }

                if (!comment) {
                    if (crlf && symb[0] == '\n') {
                        crlf = false;
                    } else {
                        iconsumer_->keyUp(symb[0]);
                    }
                } else if (symb[0] == '\r') {
                    crlf = true;
                    comment = false;
                } else if (symb[0] == '\n') {
                    comment = false;
                }
            }
            RISCV_info("Script '%s' was finished", script_name);
        }
    }


    while (isEnabled()) {
        if (isData()) {
            iconsumer_->keyUp(getData());
        }
        RISCV_sleep_ms(50);
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

void ConsoleService::updateData(const char *buf, int buflen) {
    for (int i = 0; i < buflen; i++) {
        if (buf[i] == '\r' || buf[i] == '\n') {
            if (serial_input_.size()) {
                serial_input_ = "<serialconsole> " + serial_input_ + "\n";
                writeBuffer(serial_input_.c_str());
            }
            serial_input_ .clear();
        } else {
            serial_input_ += buf[i];
        }
    }
}

void ConsoleService::writeBuffer(const char *buf) {
    RISCV_mutex_lock(&mutexConsoleOutput_);
    clearLine();
    std::cout << buf;
    std::cout << "riscv# " << cmdLine_.c_str();
    std::cout.flush();

    if (logfile_) {
        fwrite(buf, strlen(buf), 1, logfile_);
        fflush(logfile_);
    }
    RISCV_mutex_unlock(&mutexConsoleOutput_);
}

void ConsoleService::writeCommand(const char *cmd) {
    RISCV_mutex_lock(&mutexConsoleOutput_);
    clearLine();
    std::cout << "riscv# " << cmd << "\r\n";
    if (logfile_) {
        int len = sprintf(tmpbuf_, "riscv# %s\n", cmd);
        fwrite(tmpbuf_, len, 1, logfile_);
        fflush(logfile_);
    }
    RISCV_mutex_unlock(&mutexConsoleOutput_);
}

void ConsoleService::setCmdString(const char *buf) {
    RISCV_mutex_lock(&mutexConsoleOutput_);
    if (strlen(buf) < cmdLine_.size()) {
        clearLine();
    } else {
        std::cout << "\r";
    }
    cmdLine_ = std::string(buf);

    std::cout << "riscv# " << cmdLine_.c_str();
    std::cout.flush();
    RISCV_mutex_unlock(&mutexConsoleOutput_);
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

void ConsoleService::enableLogFile(const char *filename) {
    if (logfile_) {
        fclose(logfile_);
        logfile_ = NULL;
    }
    logfile_ = fopen(filename, "w");
    if (!logfile_) {
        RISCV_error("Can not open file '%s'", filename);
    }
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
