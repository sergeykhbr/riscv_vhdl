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

        mainWindow_ = new DbgMainWindow(igui_, eventInitDone_);
        mainWindow_->show();

        // Start its'own event thread
        app.exec();
        delete mainWindow_;
        mainWindow_ = 0;
    }
}

GuiPlugin::GuiPlugin(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IGui *>(this));
    registerInterface(static_cast<IThread *>(this));
    registerAttribute("WidgetsConfig", &guiConfig_);
    registerAttribute("SocInfo", &socInfo_);
    registerAttribute("CommandExecutor", &cmdExecutor_);

    guiConfig_.make_dict();
    socInfo_.make_string("");
    cmdExecutor_.make_string("");

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

    if (ui_->mainWindow()) {
        ui_->mainWindow()->postInit(&guiConfig_);
    }
    run();
}

void GuiPlugin::predeleteService() {
    stop();
}

IFace *GuiPlugin::getSocInfo() {
    return info_;
}

void GuiPlugin::registerCommand(IGuiCmdHandler *src,
                                AttributeType *cmd,
                                bool silent) {
    if (cmd->is_equal("exit")) {
        /** This is a special case when queue is full (for example target 
          * not connected) and we cannot gracefully close application
          * without this workaround.
          */
        AttributeType resp;
        RISCV_mutex_lock(&mutexCommand_);
        cmdQueueCntTotal_ = 0;
        RISCV_mutex_unlock(&mutexCommand_);
        iexec_->exec(cmd->to_string(), &resp, true);
    } else if (cmdQueueCntTotal_ >= CMD_QUEUE_SIZE) {
        RISCV_error("Command queue size %d overflow. Target not responding",
                    cmdQueueCntTotal_);
        return;
    }
    RISCV_mutex_lock(&mutexCommand_);
    if (cmdQueueWrPos_ == CMD_QUEUE_SIZE) {
        cmdQueueWrPos_ = 0;
    }
    cmdQueue_[cmdQueueWrPos_].silent = silent;
    cmdQueue_[cmdQueueWrPos_].src = src;
    cmdQueue_[cmdQueueWrPos_++].cmd = *cmd;

    if (isEnabled()) {
        cmdQueueCntTotal_++;
    }
    RISCV_mutex_unlock(&mutexCommand_);
    RISCV_event_set(&eventCommandAvailable_);
}

void GuiPlugin::busyLoop() {
    while (isEnabled()) {
        RISCV_event_wait(&eventCommandAvailable_);
        RISCV_event_clear(&eventCommandAvailable_);

        processCmdQueue();
    }
    /** If the debugger was closed via RISCV_ API call then call Main Form
     * closing event manually. */
    if (ui_->mainWindow()) {
        ui_->mainWindow()->closeForm();
    }
    ui_->stop();
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

bool GuiPlugin::processCmdQueue() {
    AttributeType resp;
    while (cmdQueueCntTotal_) {
        RISCV_mutex_lock(&mutexCommand_);
        if (cmdQueueCntTotal_) {    // Another check under mutex to avoid 'exit' collision
            cmdQueueCntTotal_--;
        }
        RISCV_mutex_unlock(&mutexCommand_);

        AttributeType &cmd = cmdQueue_[cmdQueueRdPos_].cmd;

        iexec_->exec(cmd.to_string(), &resp, cmdQueue_[cmdQueueRdPos_].silent);

        if (cmdQueue_[cmdQueueRdPos_].src) {
            cmdQueue_[cmdQueueRdPos_].src->handleResponse(
                        const_cast<AttributeType *>(&cmd), &resp);
        }
        if ((++cmdQueueRdPos_) >= CMD_QUEUE_SIZE) {
            cmdQueueRdPos_ = 0;
        }
    }
    return false;
}

void GuiPlugin::stop() {
    breakSignal();
    IThread::stop();
}

void GuiPlugin::breakSignal() {
    IThread::breakSignal();
    //int totalWidgets = QApplication::topLevelWidgets().size();
    //RISCV_error("dbg: total widgets to close %d", totalWidgets);
    if (qApp) {
        qApp->exit();
    }
    RISCV_mutex_lock(&mutexCommand_);
    cmdQueueCntTotal_ = 0;
    RISCV_mutex_unlock(&mutexCommand_);
    RISCV_event_set(&eventCommandAvailable_);
}

extern "C" void plugin_init(void) {
    REGISTER_CLASS(GuiPlugin);
}

}  // namespace debugger
