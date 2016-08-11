/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      elf-file loader class implementation.
 */

#include "console.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "api_types.h"
#include "coreservices/iserial.h"
#include "coreservices/isignal.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(ConsoleService)

#define ENTRYSYMBOLS "riscv# "

#define KB_UP       0x48
#define KB_DOWN     80
#define KB_LEFT     75
#define KB_RIGHT    77
#define KB_ESCAPE   27

static const int STDIN = 0;
static const uint8_t ARROW_PREFIX = 0xe0;
static const uint8_t UNICODE_BACKSPACE = 0x7f;

ConsoleService::ConsoleService(const char *name) 
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IConsole *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("LogFile", &logFile_);
    registerAttribute("StepQueue", &stepQueue_);
    registerAttribute("Signals", &signals_);
    registerAttribute("Serial", &serial_);
    registerAttribute("History", &history_);
    registerAttribute("HistorySize", &history_size_);

    RISCV_mutex_init(&mutexConsoleOutput_);
    RISCV_event_create(&config_done_, "config_done");
    RISCV_register_hap(static_cast<IHap *>(this));

    isEnable_.make_boolean(true);
    logFile_.make_string("");
	stepQueue_.make_string("");
	signals_.make_string("");
    serial_.make_string("");
    consoleListeners_.make_list(0);
    history_.make_list(0);
    history_size_.make_int64(4);
    history_idx_ = 0;

    logfile_ = NULL;
    iclk_ = NULL;

    symb_seq_msk_ = 0xFF;
    symb_seq_ = 0;

#ifdef DBG_ZEPHYR
    tst_cnt_ = 0;
#endif

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

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
    if (logFile_.size()) {
        enableLogFile(logFile_.to_string());
    }

    iclk_ = static_cast<IClock *>
	    (RISCV_get_service_iface(stepQueue_.to_string(), IFACE_CLOCK));

    ISignal *itmp = static_cast<ISignal *>
        (RISCV_get_service_iface(signals_.to_string(), IFACE_SIGNAL));
    if (itmp) {
        itmp->registerSignalListener(static_cast<ISignalListener *>(this));
    }

    history_idx_ = history_.size();

#ifdef DBG_ZEPHYR
    if (iclk_) {
	    iclk_->registerStepCallback(static_cast<IClockListener *>(this), 550000);
	    iclk_->registerStepCallback(static_cast<IClockListener *>(this), 12000000);
	    iclk_->registerStepCallback(static_cast<IClockListener *>(this), 20000000);//6000000);
	    iclk_->registerStepCallback(static_cast<IClockListener *>(this), 35000000);
	}
#endif

    // Redirect output stream to a this console
    RISCV_set_default_output(static_cast<IConsole *>(this));
}

void ConsoleService::predeleteService() {
    stop();
}

void ConsoleService::stepCallback(uint64_t t) {
#ifdef DBG_ZEPHYR
    if (iclk_ == NULL) {
        return;
    }
    IService *uart = static_cast<IService *>(RISCV_get_service("uart0"));
    if (uart) {
        ISerial *iserial = static_cast<ISerial *>(
                    uart->getInterface(IFACE_SERIAL));
        switch (tst_cnt_) {
        case 0:
            //iserial->writeData("ping", 4);
            iserial->writeData("dhry", 4);
            break;
        case 1:
            iserial->writeData("ticks", 5);
            break;
        case 2:
            iserial->writeData("help", 4);
            break;
        case 3:
            iserial->writeData("pnp", 4);
            break;
        default:;
        }
        tst_cnt_++;
    }
#endif
}



void ConsoleService::hapTriggered(EHapType type) {
    RISCV_event_set(&config_done_);
}

void ConsoleService::updateSignal(int start, int width, uint64_t value) {
    char sx[128];
    RISCV_sprintf(sx, sizeof(sx), "<led[%d:%d]> %02" RV_PRI64 "xh\n", 
                    start + width - 1, start, value);
    writeBuffer(sx);
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

void ConsoleService::busyLoop() {
    RISCV_event_wait(&config_done_);

    processScriptFile();
    while (isEnabled()) {
        if (isData()) {
            addToCommandLine(getData());
        }
        RISCV_sleep_ms(50);
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

void ConsoleService::processScriptFile() {
    enum EScriptState {
        SCRIPT_normal,
        SCRIPT_comment,
        SCRIPT_command
    } scr_state;
    scr_state = SCRIPT_normal;

    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["ScriptFile"].size() == 0) {
        return;
    }
    const char *script_name = (*glb)["ScriptFile"].to_string();
    FILE *script = fopen(script_name, "r");
    if (!script) {
        RISCV_error("Script file '%s' not found", script_name);
        return;
    } 
    fseek(script, 0, SEEK_END);
    long script_sz = ftell(script);
    if (script_sz == 0) {
        return;
    }
    char *script_buf = new char [script_sz + 1];
    fseek(script, 0, SEEK_SET);
    fread(script_buf, 1, script_sz, script);
    script_buf[script_sz] = '\0';
    fclose(script);

    bool crlf = false;
    for (long i = 0; i < script_sz; i++) {

        switch (scr_state) {
        case SCRIPT_normal:
            if (crlf && script_buf[i] == '\n') {
                crlf = false;
            } else if (script_buf[i] == '/'
                    && script_buf[i + 1] == '/') {
                scr_state = SCRIPT_comment;
                i++;
            } else {
                addToCommandLine(script_buf[i]);
            }
            break;
        case SCRIPT_command:
            addToCommandLine(script_buf[i]);
        case SCRIPT_comment:
            if (script_buf[i] == '\r' || script_buf[i] == '\n') {
                scr_state = SCRIPT_normal;
            }
        default:;
        }

        crlf = script_buf[i] == '\r';
    }
    delete [] script_buf;
    RISCV_info("Script '%s' was finished", script_name);
}

void ConsoleService::writeBuffer(const char *buf) {
    size_t sz = strlen(buf);
    if (!sz) {
        return;
    }
    RISCV_mutex_lock(&mutexConsoleOutput_);
    clearLine();
    std::cout << buf;
    if (buf[sz-1] != '\r' && buf[sz-1] != '\n') {
        std::cout << "\r\n";
    }
    std::cout << ENTRYSYMBOLS << cmdLine_.c_str();
    std::cout.flush();

    if (logfile_) {
        fwrite(buf, strlen(buf), 1, logfile_);
        fflush(logfile_);
    }
    RISCV_mutex_unlock(&mutexConsoleOutput_);
}

void ConsoleService::clearLine() {
    std::cout << "\r                                                      \r";
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

void ConsoleService::registerConsoleListener(IFace *iface) {
    AttributeType t1(iface);
    consoleListeners_.add_to_list(&t1);
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

bool ConsoleService::convertToWinKey(uint8_t symb) {
    bool ret = true;
    symb_seq_ <<= 8;
    symb_seq_ |= symb;
    symb_seq_ &= symb_seq_msk_;

#if defined(_WIN32) || defined(__CYGWIN__)
    if (symb_seq_ == ARROW_PREFIX) {
        ret = false;
        symb_seq_msk_ = 0xFFFF;
    } else {
        symb_seq_msk_ = 0xFF;
    }
#else
    if (symb_seq_ == UNICODE_BACKSPACE) {
        symb_seq_ = '\b';
        symb_seq_msk_ = 0xFF;
    } else if (symb_seq_ == 0x1b) {
        symb_seq_msk_ = 0xFFFF;
        ret = false;
    } else if (symb_seq_ == 0x1b5b) {
        symb_seq_msk_ = 0xFFFFFF;
        ret = false;
    } else if (symb_seq_ == 0x1b5b41) {
        symb_seq_ = (ARROW_PREFIX << 8) | KB_UP;
        symb_seq_msk_ = 0xFF;
    } else if (symb_seq_ == 0x1b5b42) {
        symb_seq_ = (ARROW_PREFIX << 8) | KB_DOWN;
        symb_seq_msk_ = 0xFF;
    } else if (symb_seq_ == 0x1b5b43) {
        //symb_seq_ = (ARROW_PREFIX << 8) | KB_RIGHT;
        //symb_seq_msk_ = 0xFF;
        ret = false;
    } else if (symb_seq_ == 0x1b5b44) {
        //symb_seq_ = (ARROW_PREFIX << 8) | KB_LEFT;
        //symb_seq_msk_ = 0xFF;
        ret = false;
    } else {
        symb_seq_msk_ = 0xFF;
    }
#endif
    if (symb_seq_ == ((uint32_t('\r') << 8) | '\n')) {
        symb_seq_ = 0;
    }
    return ret;
}


void ConsoleService::addToCommandLine(int val) {
    bool set_history_end = true;
    uint8_t symb = static_cast<uint8_t>(val);
    if (!convertToWinKey(symb)) {
        return;
    }

    switch (symb_seq_) {
    case 0:
        break;
    case (ARROW_PREFIX << 8) | KB_UP:
        set_history_end = false;
        if (history_idx_ == history_.size()) {
            unfinshedLine_ = cmdLine_;
        }
        if (history_idx_ > 0) {
            history_idx_--;
        }
        cmdLine_ = std::string(history_[history_idx_].to_string());
        clearLine();
        std::cout << ENTRYSYMBOLS << cmdLine_;
        break;
    case (ARROW_PREFIX << 8) | KB_DOWN:
        set_history_end = false;
        if (history_idx_ == (history_.size() - 1)) {
            history_idx_++;
            cmdLine_ = unfinshedLine_;
        } else if (history_idx_ < (history_.size() - 1)) {
            history_idx_++;
            cmdLine_ = std::string(history_[history_idx_].to_string());
        }
        clearLine();
        std::cout << ENTRYSYMBOLS << cmdLine_;
        break;
    case '\b':// 1. Backspace button:
        if (cmdLine_.size()) {
            RISCV_mutex_lock(&mutexConsoleOutput_);
            cmdLine_.erase(cmdLine_.size() - 1);
            std::cout << "\b \b";
            RISCV_mutex_unlock(&mutexConsoleOutput_);
        }
        break;
    case '\n':
    case '\r':// 2. Enter button:
        processCommandLine();
        break;
    default:;
        RISCV_mutex_lock(&mutexConsoleOutput_);
        cmdLine_ += symb;
        std::cout << symb;
        RISCV_mutex_unlock(&mutexConsoleOutput_);
    }

    if (set_history_end) {
        history_idx_ = history_.size();
    }
}

void ConsoleService::processCommandLine() {
    symb_seq_ = 0;
    addToHistory(cmdLine_.c_str());

    RISCV_mutex_lock(&mutexConsoleOutput_);
    char tmpStr[256];
    char *pStr = tmpStr;
    if (cmdLine_.size() >= 256) {
        pStr = new char [cmdLine_.size() + 1];
    }
    strcpy(pStr, cmdLine_.c_str());
    cmdLine_.clear();

    std::cout << "\r\n" ENTRYSYMBOLS;
    if (logfile_) {
        int len = sprintf(tmpbuf_, ENTRYSYMBOLS "%s\n", pStr);
        fwrite(tmpbuf_, len, 1, logfile_);
        fflush(logfile_);
    }
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
    } else {
        for (unsigned i = 0; i < consoleListeners_.size(); i++) {
            IConsoleListener *ilstn = 
            static_cast<IConsoleListener *>(consoleListeners_[i].to_iface());
            ilstn->udpateCommand(pStr);
        }
    }
    if (pStr != tmpStr) {
        delete [] pStr;
    }
}

void ConsoleService::addToHistory(const char *cmd) {
    unsigned found = history_.size();
    for (unsigned i = 0; i < history_.size(); i++) {
        if (strcmp(cmd, history_[i].to_string()) == 0) {
            found = i;
            break;
        }
    }
    if (found  ==  history_.size()) {
        AttributeType new_cmd(cmd);
        history_.add_to_list(&new_cmd);

        unsigned min_size = static_cast<unsigned>(history_size_.to_int64());
        if (history_.size() >= 2*min_size) {
            history_.trim_list(0, min_size);
        }
    } else if (found < (history_.size() - 1)) {
        history_.swap_list_item(found, history_.size() - 1);
    }
    history_idx_ = history_.size();
}

}  // namespace debugger
