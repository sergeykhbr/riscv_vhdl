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

namespace debugger {

GuiPlugin::GuiPlugin(const char *name) 
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IGui *>(this));
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IHap *>(this));
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

    info_ = 0;
    ui_ = NULL;
    RISCV_event_create(&eventCommandAvailable_, "eventCommandAvailable_");
    RISCV_event_create(&config_done_, "eventGuiGonfigGone");
    RISCV_register_hap(static_cast<IHap *>(this));
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

    ui_ = new QtWrapper(static_cast<IGui *>(this));
}

GuiPlugin::~GuiPlugin() {
    RISCV_event_close(&config_done_);
    RISCV_event_close(&eventCommandAvailable_);
    RISCV_mutex_destroy(&mutexCommand_);
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

    ui_->postInit(&guiConfig_);
    run();
}

IFace *GuiPlugin::getSocInfo() {
    return info_;
}

const AttributeType *GuiPlugin::getpConfig() {
    return &guiConfig_;
}

void GuiPlugin::registerCommand(IGuiCmdHandler *src,
                                AttributeType *cmd,
                                bool silent) {
   
    if (cmdQueueCntTotal_ >= CMD_QUEUE_SIZE) {
        RISCV_error("Command queue size %d overflow. Target not responding",
                    cmdQueueCntTotal_);
        return;
    }
    //RISCV_info("CMD %s", cmd->to_string());
    if (cmdQueueWrPos_ == CMD_QUEUE_SIZE) {
        cmdQueueWrPos_ = 0;
    }
    RISCV_mutex_lock(&mutexCommand_);
    cmdQueue_[cmdQueueWrPos_].silent = silent;
    cmdQueue_[cmdQueueWrPos_].src = src;
    cmdQueue_[cmdQueueWrPos_++].cmd = *cmd;
    cmdQueueCntTotal_++;
    RISCV_mutex_unlock(&mutexCommand_);
    RISCV_event_set(&eventCommandAvailable_);
}

void GuiPlugin::removeFromQueue(IFace *iface) {
    RISCV_mutex_lock(&mutexCommand_);
    for (unsigned i = 0; i < CMD_QUEUE_SIZE; i++) {
        if (iface == cmdQueue_[i].src) {
            cmdQueue_[i].src = NULL;
        }
    }
    RISCV_mutex_unlock(&mutexCommand_);
}

void GuiPlugin::hapTriggered(IFace *isrc, EHapType type, 
                                  const char *descr) {
    RISCV_event_set(&config_done_);
}

void GuiPlugin::busyLoop() {
    RISCV_event_wait(&config_done_);

    while (isEnabled()) {
        if (RISCV_event_wait_ms(&eventCommandAvailable_, 500)) {
            RISCV_event_clear(&eventCommandAvailable_);
            continue;
        }

        processCmdQueue();
    }
    ui_->gracefulClose();
    delete ui_;
}

bool GuiPlugin::processCmdQueue() {
    AttributeType resp;
    while (cmdQueueCntTotal_ > 0) {
        AttributeType &cmd = cmdQueue_[cmdQueueRdPos_].cmd;

        iexec_->exec(cmd.to_string(), &resp, cmdQueue_[cmdQueueRdPos_].silent);

        RISCV_mutex_lock(&mutexCommand_);
        if (cmdQueue_[cmdQueueRdPos_].src) {
            cmdQueue_[cmdQueueRdPos_].src->handleResponse(
                        const_cast<AttributeType *>(&cmd), &resp);
        }
        if ((++cmdQueueRdPos_) >= CMD_QUEUE_SIZE) {
            cmdQueueRdPos_ = 0;
        }
        cmdQueueCntTotal_--;
        RISCV_mutex_unlock(&mutexCommand_);
    }
    return false;
}

void GuiPlugin::stop() {
    IThread::stop();
}

extern "C" void plugin_init(void) {
    REGISTER_CLASS(GuiPlugin);
}

}  // namespace debugger
