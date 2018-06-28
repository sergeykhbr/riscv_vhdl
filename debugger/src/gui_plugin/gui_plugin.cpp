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

IService *GuiPlugin::getParentService() {
    return static_cast<IService *>(this);
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
    queue_.put(src, cmd, silent);
    RISCV_event_set(&eventCommandAvailable_);
}

void GuiPlugin::removeFromQueue(IFace *iface) {
    queue_.remove(iface);
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
    AttributeType cmd;
    IFace *iresp;
    bool silent;

    queue_.initProc();
    queue_.pushPreQueued();

    while (queue_.getNext(&iresp, cmd, silent)) {
        iexec_->exec(cmd.to_string(), &resp, silent);

        if (iresp) {
            static_cast<IGuiCmdHandler *>(iresp)->handleResponse(&cmd, &resp);
        }
        cmd.attr_free();
        resp.attr_free();
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
