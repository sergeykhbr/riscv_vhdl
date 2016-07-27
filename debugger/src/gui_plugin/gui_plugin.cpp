/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      GUI for the RISC-V debugger.
 */

#include "api_core.h"
#include "gui_plugin.h"
#include "coreservices/iserial.h"
#include "coreservices/irawlistener.h"
#include <string>
#include <QtWidgets/QApplication>
//#include <QtCore/QtPlugin>


//Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

namespace debugger {

const char *gui_default_ = 
"["
    "['ConsoleHistory',['csr mcpuid','regs']],"
    "['Serial','uart0'],"
    "['Gpio','gpio0'],"
    "['PollingMs',500]"
"]"
;

void GuiPlugin::UiThreadType::busyLoop() {
    int argc = 0;
    char *argv[] = {0};

    // Only one instance of QApplication is possible that is accessible
    // via global pointer qApp for all widgets.
    if (!qApp) {
        // Creating GUI thread
        QApplication app(argc, argv);
        app.setQuitOnLastWindowClosed(true);

        DbgMainWindow mainWindow(igui_, eventInitDone_);
        mainWindow.show();

        // Start its'own event thread
        app.exec();
    }
}

GuiPlugin::GuiPlugin(const char *name) : IService(name) {
    /// Interface registration
    registerInterface(static_cast<IGui *>(this));
    registerAttribute("GuiConfig", &guiConfig_);
    registerAttribute("Tap", &tap_);
    
    guiConfig_.make_list(0);
    tap_.make_string("");
    mainWindow_ = NULL;

    char coredir[4096];
    RISCV_get_core_folder(coredir, sizeof(coredir));
    QString topDir(coredir);
    QResource::registerResource(
        topDir + "../../src/gui_plugin/resources/gui.rcc");

    ui_ = NULL;
    RISCV_event_create(&eventUiInitDone_, "eventUiInitDone_");
    RISCV_event_create(&eventCommandAvailable_, "eventCommandAvailable_");
    RISCV_mutex_init(&mutexCommand_);

    cmdQueueWrPos_ = 0;
    cmdQueueRdPos_ = 0;
    cmdQueueCntTotal_ = 0;


    // Adding path to platform libraries:
    QStringList paths = QApplication::libraryPaths();
    std::string qt_path = std::string(getenv("QT_PATH"));
    if (!qt_path.length()) {
        RISCV_error("%s env.variable not set. Cannot start GUI.", 
                    "QT_PATH");
        RISCV_event_set(&eventUiInitDone_);
    } else {
        std::string qt_lib_path = qt_path + "bin";
        paths.append(QString(qt_lib_path.c_str()));
        paths.append(QString("platforms"));
        QApplication::setLibraryPaths(paths);

        ui_ = new UiThreadType(static_cast<IGui *>(this), 
                                &eventUiInitDone_);
        ui_->run();
    }
}

GuiPlugin::~GuiPlugin() {
    if (ui_) {
        delete ui_;
    }
    RISCV_event_close(&eventUiInitDone_);
    RISCV_event_close(&eventCommandAvailable_);
    RISCV_mutex_destroy(&mutexCommand_);
}

void GuiPlugin::initService(const AttributeType *args) {
    IService::initService(args);
    RISCV_event_wait(&eventUiInitDone_);
}

void GuiPlugin::postinitService() {
    if (guiConfig_.size() == 0) {
        guiConfig_.from_config(gui_default_);
    }
    if (mainWindow_) {
        mainWindow_->setConfiguration(guiConfig_);
    }
    itap_ = static_cast<ITap *>(
        RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    if (!itap_) {
        RISCV_error("Tap interface of %s not found.", 
                    tap_.to_string());
    }
    run();
}

void GuiPlugin::predeleteService() {
    stop();
}

void GuiPlugin::registerMainWindow(void *iwindow) {
    mainWindow_ = reinterpret_cast<DbgMainWindow *>(iwindow);
}

void GuiPlugin::registerWidgetInterface(IFace *iface) {
    registerInterface(iface);
}

void GuiPlugin::unregisterWidgetInterface(IFace *iface) {
    unregisterInterface(iface);
}

void GuiPlugin::registerCommand(IGuiCmdHandler *src, AttributeType *cmd) {
    if (cmdQueueCntTotal_ >= CMD_QUEUE_SIZE) {
        RISCV_error("Command queue size %d overflow.", cmdQueueCntTotal_);
        return;
    }
    RISCV_mutex_lock(&mutexCommand_);
    if (cmdQueueWrPos_ == CMD_QUEUE_SIZE) {
        cmdQueueWrPos_ = 0;
    }
    cmdQueue_[cmdQueueWrPos_].src = src;
    cmdQueue_[cmdQueueWrPos_++].cmd = *cmd;

    cmdQueueCntTotal_++;
    RISCV_mutex_unlock(&mutexCommand_);
    RISCV_event_set(&eventCommandAvailable_);
}

void GuiPlugin::busyLoop() {
    while (loopEnable_) {
        RISCV_event_wait(&eventCommandAvailable_);
        RISCV_event_clear(&eventCommandAvailable_);

        /** Exit event detected */
        if (processCmdQueue()) {
            loopEnable_ = false;
            RISCV_break_simulation();
        }
    }
}

bool GuiPlugin::processCmdQueue() {
    int total = cmdQueueCntTotal_;
    for (int i = 0; i < total; i++) {
        if (cmdQueueRdPos_ == CMD_QUEUE_SIZE) {
            cmdQueueRdPos_ = 0;
        }
        if (!cmdQueue_[cmdQueueRdPos_].cmd.is_list()) {
            cmdQueueRdPos_++;
            continue;
        }
        const AttributeType &cmd = cmdQueue_[cmdQueueRdPos_].cmd;
        if (!cmd[0u].is_string()) {
            cmdQueueRdPos_++;
            continue;
        }

        if (strcmp(cmd[0u].to_string(), "exit") == 0) {
            cmdQueueRdPos_++;
            return true;
        }

        /** Memory read access: */
        if (strcmp(cmd[0u].to_string(), "read") == 0) {
            int bytes = static_cast<int>(cmd[2].to_uint64());
            uint8_t obuf[32] = {0};
            itap_->read(cmd[1].to_uint64(), bytes, obuf);
            AttributeType resp;
            if (bytes <= 8) {
                resp.make_uint64(reinterpret_cast<uint64_t>(obuf));
            } else {
                resp.make_data(bytes, obuf);
            }
            if (cmdQueue_[cmdQueueRdPos_].src) {
                cmdQueue_[cmdQueueRdPos_].src->handleResponse(
                            const_cast<AttributeType *>(&cmd), &resp);
            }
        }

        /** Memory write access: */
        if (strcmp(cmd[0u].to_string(), "write") == 0) {
            int bytes = static_cast<int>(cmd[2].to_uint64());
            uint64_t val64 = 0;
            uint8_t *ibuf = reinterpret_cast<uint8_t *>(&val64);
            if (bytes <= 8) {
                val64 = cmd[3].to_uint64();
            } else {
                ibuf = const_cast<uint8_t *>(cmd[3].data());
            }
            itap_->write(cmd[1].to_uint64(), bytes, ibuf);
        }
        cmdQueueRdPos_++;
    }
    RISCV_mutex_lock(&mutexCommand_);
    cmdQueueCntTotal_ -= total;
    RISCV_mutex_unlock(&mutexCommand_);
    return false;
}

void GuiPlugin::stop() {
    qApp->exit();
    IThread::stop();
}

void GuiPlugin::breakSignal() {
    qApp->exit();
    IThread::breakSignal();
}

extern "C" void plugin_init(void) {
    REGISTER_CLASS(GuiPlugin);
}

}  // namespace debugger
