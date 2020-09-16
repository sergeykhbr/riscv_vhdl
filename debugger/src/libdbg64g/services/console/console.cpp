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

#include "console.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "api_types.h"
#include "coreservices/iserial.h"

namespace debugger {

#define ENTRYSYMBOLS "cmd# "

static const int STDIN = 0;

ConsoleService::ConsoleService(const char *name) 
    : IService(name), IHap(HAP_ConfigDone), 
      portSerial_(this, "serialconsole", true) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("StepQueue", &stepQueue_);
    registerAttribute("AutoComplete", &autoComplete_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("Signals", &signals_);
    registerAttribute("InputPort", &inPort_);
    registerAttribute("DefaultLogFile", &defaultLogFile_);
    registerAttribute("VirtualKbEna", &virtualKbEna_);

    RISCV_mutex_init(&mutexConsoleOutput_);
    RISCV_event_create(&config_done_, "console_config_done");
    RISCV_register_hap(static_cast<IHap *>(this));

    isEnable_.make_boolean(true);
	stepQueue_.make_string("");
    autoComplete_.make_string("");
	signals_.make_string("");
    inPort_.make_string("");
    defaultLogFile_.make_string("");
    virtualKbEna_.make_boolean(false);

    isrc_ = NULL;
    cmdSizePrev_ = 0;
    virtKbWrCnt_ = 0;
    virtKbRdCnt_ = 0;

    cursor_.make_list(2);
    cursor_[0u].make_int64(0);
    cursor_[1].make_int64(0);

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
    // Redirect output stream to a this console
    RISCV_add_default_output(static_cast<IRawListener *>(this));
}

ConsoleService::~ConsoleService() {
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    tcsetattr(STDIN, TCSANOW, &original_settings_);
#endif
    RISCV_event_close(&config_done_);
    RISCV_mutex_destroy(&mutexConsoleOutput_);
}

void ConsoleService::postinitService() {
    if (inPort_.size()) {
        ISerial *iport = static_cast<ISerial *>
                (RISCV_get_service_iface(inPort_.to_string(), IFACE_SERIAL));
        if (iport) {
            iport->registerRawListener(
                    static_cast<IRawListener *>(&portSerial_));
        } else {
            RISCV_error("Can't connect to com-port %s.", inPort_.to_string());
        }
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }

    iautocmd_ = static_cast<IAutoComplete *>(
            RISCV_get_service_iface(autoComplete_.to_string(), 
                                    IFACE_AUTO_COMPLETE));
    if (!iautocmd_) {
        RISCV_error("Can't get IAutoComplete interface %s",
                    autoComplete_.to_string());
    }

    iexec_ = static_cast<ICmdExecutor *>(
            RISCV_get_service_iface(cmdexec_.to_string(), 
                                    IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
                    cmdexec_.to_string());
    }

    AttributeType src_list;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &src_list);
    if (src_list.is_list() && src_list.size()) {
        IService *iserv = static_cast<IService *>(src_list[0u].to_iface());
        isrc_ = static_cast<ISourceCode *>(
                    iserv->getInterface(IFACE_SOURCE_CODE));
    }
}

void ConsoleService::predeleteService() {
    RISCV_remove_default_output(static_cast<IRawListener *>(this));
}

void ConsoleService::hapTriggered(EHapType type, 
                                  uint64_t param,
                                  const char *descr) {
    RISCV_unregister_hap(static_cast<IHap *>(this));
    RISCV_event_set(&config_done_);

    // Enable logging:
    if (defaultLogFile_.size()) {
        AttributeType res;
        std::string cmd("log ");
        cmd += defaultLogFile_.to_string();
        iexec_->exec(cmd.c_str(), &res, true);
    }
}

int ConsoleService::updateData(const char *buf, int buflen) {
    writeBuffer(buf);
    return buflen;
}

void ConsoleService::busyLoop() {
    RISCV_event_wait(&config_done_);

    bool cmd_ready;
    AttributeType cmd, cmdres;

    while (isEnabled()) {
        if (!isKbHit()) {
            RISCV_sleep_ms(50);
            continue;
        }

        cmd_ready = iautocmd_->processKey(getData(), &cmd, &cursor_);
        if (cmd_ready) {
            RISCV_mutex_lock(&mutexConsoleOutput_);
            std::cout << "\r";
            RISCV_mutex_unlock(&mutexConsoleOutput_);
            
            RISCV_printf0("%s%s", ENTRYSYMBOLS, cmd.to_string());

            iexec_->exec(cmd.to_string(), &cmdres, false);

            if (!cmdres.is_nil() && !cmdres.is_invalid()) {
                RISCV_printf0("%s", cmdres.to_config().to_string());
            }
        } else {
            RISCV_mutex_lock(&mutexConsoleOutput_);
            std::cout << '\r' << ENTRYSYMBOLS << cmd.to_string();
            if (cmdSizePrev_ > cmd.size()) {
                clearLine(static_cast<int>(cmdSizePrev_ - cmd.size()));
            }
            for (int i = 0; i < cursor_[0u].to_int(); i++) {
                std::cout << '\b';
            }
            RISCV_mutex_unlock(&mutexConsoleOutput_);
        }
        std::cout.flush();
        cmdSizePrev_ = cmd.size();
    }
}

void ConsoleService::writeBuffer(const char *buf) {
    size_t sz = strlen(buf);
    if (!sz) {
        return;
    }
    RISCV_mutex_lock(&mutexConsoleOutput_);
    std::cout << '\r';
    clearLine(70);
    std::cout << buf;
    if (buf[sz-1] != '\r' && buf[sz-1] != '\n') {
        std::cout << "\r\n";
    }
    std::cout << ENTRYSYMBOLS << cmdLine_.c_str();
    for (int i = 0; i < cursor_[0u].to_int(); i++) {
        std::cout << '\b';
    }
    std::cout.flush();

    RISCV_mutex_unlock(&mutexConsoleOutput_);
}

void ConsoleService::clearLine(int num) {
    for (int i = 0; i < num; i++) {
        std::cout << ' ';
    }
    for (int i = 0; i < num; i++) {
        std::cout << '\b';
    }
}

bool ConsoleService::isKbHit() {
    if (virtualKbEna_.to_bool()) {
        if (virtKbRdCnt_ == virtKbWrCnt_) {
            return false;
        }
        return true;
    }
#if defined(_WIN32) || defined(__CYGWIN__)
    return _kbhit() ? true: false;
#else
    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting != 0;
#endif
}

void ConsoleService::setVirtualKbHit(char s) {
    virtKbChar_[virtKbWrCnt_++] = s;
    virtKbChar_[virtKbWrCnt_] = '\0';
}

char ConsoleService::getVirtualKbChar() {
    int t1 = virtKbRdCnt_;
    if (virtKbRdCnt_ != virtKbWrCnt_) {
        ++virtKbRdCnt_;
    }
    return virtKbChar_[t1];
}

char ConsoleService::getChar() {
    if (virtualKbEna_.to_bool()) {
        return getVirtualKbChar();
    }
#if defined(_WIN32) || defined(__CYGWIN__)
    return _getch();
#else
    char ret;
    read(term_fd_, &ret, 1);
    return ret;
#endif
}

uint32_t ConsoleService::getData() {
    Reg64Type tbuf;
    tbuf.val = 0;
    int pos = 0;
    while (isKbHit()) {
        tbuf.val <<= 8;
        tbuf.buf[0] = static_cast<uint8_t>(getChar());
        pos++;
    }
    //printf("\nkey_code=%08x\n", tbuf.buf32[0]);
    switch (tbuf.buf32[0]) {
    case 0x000d:
    case 0x000a:
    case 0x0d0a:
        tbuf.buf32[0] = KB_Return;
        break;
    case 0x001b:
        tbuf.buf32[0] = KB_Escape;
        break;
#if defined(_WIN32) || defined(__CYGWIN__)
    case 0x0008:
        tbuf.buf32[0] = KB_Backspace;
        break;
    case VK_UP:
    case 0x4800:
        tbuf.buf32[0] = KB_Up;
        break;
    case VK_LEFT:
    case 0x4b00:
        tbuf.buf32[0] = KB_Left;
        break;
    case VK_RIGHT:
    case 0x4d00:
        tbuf.buf32[0] = KB_Right;
        break;
    case VK_DOWN:
    case 0x5000:
        tbuf.buf32[0] = KB_Down;
        break;
    case VK_DELETE:
    case 0x5300:
        tbuf.buf32[0] = KB_Delete;
        break;
    case VK_TAB:
    case 0x5301:
        tbuf.buf32[0] = KB_Tab;
        break;
#else
    case 0x007f:
        tbuf.buf32[0] = KB_Backspace;
        break;
    case 0x1b5b41:
    case 0x410000:
        tbuf.buf32[0] = KB_Up;
        break;
    case 0x1b5b44:
    case 0x440000:
        tbuf.buf32[0] = KB_Left;
        break;
    case 0x1b5b43:
    case 0x430000:
        tbuf.buf32[0] = KB_Right;
        break;
    case 0x1b5b42:
    case 0x420000:
        tbuf.buf32[0] = KB_Down;
        break;
    case 0x7e000000:
        tbuf.buf32[0] = KB_Delete;
        break;
    case 0x0009:
        tbuf.buf32[0] = KB_Tab;
        break;
#endif
    default:;
    }
    return tbuf.buf32[0];
}

void ConsoleService::processCommandLine() {
    RISCV_mutex_lock(&mutexConsoleOutput_);
    char tmpStr[256];
    char *pStr = tmpStr;
    if (cmdLine_.size() >= 256) {
        pStr = new char [cmdLine_.size() + 1];
    }
    strcpy(pStr, cmdLine_.c_str());
    cmdLine_.clear();

    std::cout << "\r\n" ENTRYSYMBOLS;
    RISCV_mutex_unlock(&mutexConsoleOutput_);

    if (pStr[0] == '\0') {
        return;
    }

    AttributeType t1;
    t1.from_config(pStr);
    if (t1.is_list()) {
        if (strcmp(t1[0u].to_string(), "wait") == 0) {
            RISCV_sleep_ms(static_cast<int>(t1[1].to_int64()));
        } else if (strcmp(t1[0u].to_string(), "uart0") == 0) {
            std::string strCmd(t1[1].to_string());
            size_t idx = strCmd.find("\\r", 0);
            if (idx != std::string::npos) {
                strCmd.replace(idx, 2, "\r");
            }
            IService *uart = 
                static_cast<IService *>(RISCV_get_service("uart0"));
            if (uart) {
                ISerial *iserial = static_cast<ISerial *>(
                            uart->getInterface(IFACE_SERIAL));
                iserial->writeData(strCmd.c_str(),
                                   static_cast<int>(strCmd.size()));
            }
        }
    }
    if (pStr != tmpStr) {
        delete [] pStr;
    }
}


}  // namespace debugger
