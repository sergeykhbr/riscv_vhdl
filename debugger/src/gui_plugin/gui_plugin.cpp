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
        QMainWindow *t = mainWindow_;
        mainWindow_ = 0;
        delete t;
    }
    threadInit_.Handle = 0;
    RISCV_break_simulation();
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
        topDir + "resources/gui.rcc");

    ui_ = NULL;
    RISCV_event_create(&eventUiInitDone_, "eventUiInitDone_");
    RISCV_event_create(&eventCommandAvailable_, "eventCommandAvailable_");
    RISCV_event_create(&eventCmdQueueEmpty_, "eventCmdQueueEmpty_");
    RISCV_event_set(&eventCmdQueueEmpty_);  // init true value
    RISCV_mutex_init(&mutexCommand_);

    cmdQueueWrPos_ = 0;
    cmdQueueRdPos_ = 0;
    cmdQueueCntTotal_ = 0;

    // Adding path to platform libraries:
    char core_path[1024];
    RISCV_get_core_folder(core_path, sizeof(core_path));
    QStringList paths = QApplication::libraryPaths();
    std::string qt_path = std::string(core_path);
    std::string qt_lib_path = qt_path;
    paths.append(QString(qt_lib_path.c_str()));
    qt_lib_path += "qtlib/";
    paths.append(QString(qt_lib_path.c_str()));
    qt_lib_path += "platforms";
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
    RISCV_event_close(&eventCmdQueueEmpty_);
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
   
    if (cmdQueueCntTotal_ >= CMD_QUEUE_SIZE) {
        RISCV_error("Command queue size %d overflow. Target not responding",
                    cmdQueueCntTotal_);
        return;
    }
    if (cmdQueueWrPos_ == CMD_QUEUE_SIZE) {
        cmdQueueWrPos_ = 0;
    }
    cmdQueue_[cmdQueueWrPos_].silent = silent;
    cmdQueue_[cmdQueueWrPos_].src = src;
    cmdQueue_[cmdQueueWrPos_++].cmd = *cmd;

    if (isEnabled()) {
        RISCV_mutex_lock(&mutexCommand_);
        cmdQueueCntTotal_++;
        RISCV_mutex_unlock(&mutexCommand_);
    }
    RISCV_event_set(&eventCommandAvailable_);
}

void GuiPlugin::waitQueueEmpty() {
    if (!isEnabled()) {
        return;
    }
    for (unsigned i = 0; i < CMD_QUEUE_SIZE; i++) {
        cmdQueue_[i].src = NULL;
    }
    cmdQueueCntTotal_ = 0;
    RISCV_event_clear(&eventCmdQueueEmpty_);
    RISCV_event_set(&eventCommandAvailable_);
    RISCV_event_wait_ms(&eventCmdQueueEmpty_, 100);
}

void GuiPlugin::busyLoop() {
    while (isEnabled()) {
        if (RISCV_event_wait_ms(&eventCommandAvailable_, 500)) {
            RISCV_event_clear(&eventCommandAvailable_);
            continue;
        }

        processCmdQueue();
        RISCV_event_set(&eventCmdQueueEmpty_);
    }
}

bool GuiPlugin::processCmdQueue() {
    AttributeType resp;
    while (cmdQueueCntTotal_ > 0) {
        AttributeType &cmd = cmdQueue_[cmdQueueRdPos_].cmd;

        iexec_->exec(cmd.to_string(), &resp, cmdQueue_[cmdQueueRdPos_].silent);

        if (cmdQueue_[cmdQueueRdPos_].src) {
            cmdQueue_[cmdQueueRdPos_].src->handleResponse(
                        const_cast<AttributeType *>(&cmd), &resp);
        }
        if ((++cmdQueueRdPos_) >= CMD_QUEUE_SIZE) {
            cmdQueueRdPos_ = 0;
        }
        RISCV_mutex_lock(&mutexCommand_);
        cmdQueueCntTotal_--;
        RISCV_mutex_unlock(&mutexCommand_);
    }
    return false;
}

void GuiPlugin::stop() {
    if (ui_->mainWindow()) {
        ui_->mainWindow()->callExit();
    }
    if (qApp) {
        //int totalWidgets = QApplication::topLevelWidgets().size();
        //RISCV_error("dbg: total widgets to close %d", totalWidgets);
        qApp->closeAllWindows();
        qApp->exit(0);
    }
    ui_->stop();
    IThread::stop();
}

extern "C" void plugin_init(void) {
    REGISTER_CLASS(GuiPlugin);
}

}  // namespace debugger
