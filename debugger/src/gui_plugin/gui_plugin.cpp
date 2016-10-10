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

GuiPlugin::GuiPlugin(const char *name) 
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IGui *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("WidgetsConfig", &guiConfig_);
    registerAttribute("SocInfo", &socInfo_);
    registerAttribute("CommandExecutor", &cmdExecutor_);

    RISCV_register_hap(static_cast<IHap *>(this));
    
    guiConfig_.make_dict();
    socInfo_.make_string("");
    cmdExecutor_.make_string("");
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
    char core_path[1024];
    RISCV_get_core_folder(core_path, sizeof(core_path));
    QStringList paths = QApplication::libraryPaths();
    std::string qt_path = std::string(core_path);
    std::string qt_lib_path = qt_path;// + "qtlib";
    paths.append(QString(qt_lib_path.c_str()));
    paths.append(QString("platforms"));
    QApplication::setLibraryPaths(paths);

    ui_ = new UiThreadType(static_cast<IGui *>(this), 
                            &eventUiInitDone_);
    ui_->run();
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
    iexec_ = static_cast<ICmdExecutor *>(
        RISCV_get_service_iface(cmdExecutor_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("ICmdExecutor interface of %s not found.", 
                    cmdExecutor_.to_string());
    }

    info_ = static_cast<ISocInfo *>(
        RISCV_get_service_iface(socInfo_.to_string(), IFACE_SOC_INFO));
    if (!iexec_) {
        RISCV_error("ISocInfo interface of %s not found.", 
                    socInfo_.to_string());
    }

    if (mainWindow_) {
        mainWindow_->postInit(guiConfig_);
    }
    run();
}

void GuiPlugin::predeleteService() {
    stop();
}

void GuiPlugin::hapTriggered(IFace *isrc, EHapType type, const char *descr) {
    mainWindow_->configDone();
}

IFace *GuiPlugin::getSocInfo() {
    return info_;
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

void GuiPlugin::registerCommand(IGuiCmdHandler *src,
                                AttributeType *cmd,
                                bool silent) {
    if (cmdQueueCntTotal_ >= CMD_QUEUE_SIZE) {
        RISCV_error("Command queue size %d overflow.", cmdQueueCntTotal_);
        return;
    }
    RISCV_mutex_lock(&mutexCommand_);
    if (cmdQueueWrPos_ == CMD_QUEUE_SIZE) {
        cmdQueueWrPos_ = 0;
    }
    cmdQueue_[cmdQueueWrPos_].silent = silent;
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
    AttributeType resp;
    int total = cmdQueueCntTotal_;
    for (int i = 0; i < total; i++) {
        if (cmdQueueRdPos_ == CMD_QUEUE_SIZE) {
            cmdQueueRdPos_ = 0;
        }
        AttributeType &cmd = cmdQueue_[cmdQueueRdPos_].cmd;
        if (!cmd.is_string()) {
            /** Script not tested yet */
            cmdQueueRdPos_++;
            continue;
        }

        if (cmd.is_equal("exit")) {
            cmdQueueRdPos_++;
            return true;
        }
        iexec_->exec(cmd.to_string(), &resp, cmdQueue_[cmdQueueRdPos_].silent);

        if (cmdQueue_[cmdQueueRdPos_].src) {
            cmdQueue_[cmdQueueRdPos_].src->handleResponse(
                        const_cast<AttributeType *>(&cmd), &resp);
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
