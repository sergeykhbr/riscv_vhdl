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
    registerAttribute("CmdExecutor", &cmdexec_);

    guiConfig_.make_dict();

#if defined(_WIN32) || defined(__CYGWIN__)
    wchar_t coredir[4096];
    RISCV_get_core_folderw(coredir, sizeof(coredir));
    QString topDir = QString::fromWCharArray(coredir);
    QResource::registerResource(
        topDir + QString::fromWCharArray(L"resources/gui.rcc"));
#else
    char coredir[4096];
    RISCV_get_core_folder(coredir, sizeof(coredir));
    QString topDir(coredir);
    QResource::registerResource(
        topDir + "resources/gui.rcc");
#endif

    ui_ = NULL;
    RISCV_event_create(&config_done_, "eventGuiGonfigGone");
    RISCV_register_hap(static_cast<IHap *>(this));

    cmdwrcnt_ = 0;
    cmdrdcnt_ = 0;
    pcmdwr_ = cmdbuf_;

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
}

void GuiPlugin::postinitService() {
    iexec_ = static_cast<ICmdExecutor *>(
        RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("ICmdExecutor interface of %s not found.", 
                    cmdexec_.to_string());
    }

    ui_->postInit(&guiConfig_);
    run();
}

IService *GuiPlugin::getParentService() {
    return static_cast<IService *>(this);
}

const AttributeType *GuiPlugin::getpConfig() {
    return &guiConfig_;
}

void GuiPlugin::registerCommand(IGuiCmdHandler *iface,
                                const char *req,
                                AttributeType *resp,
                                bool silent) {
    size_t sztoend = sizeof(cmdbuf_) - (pcmdwr_ - cmdbuf_);
    size_t szwr = strlen(req) + 1;
    if (sztoend < szwr) {
        pcmdwr_ = cmdbuf_;
    }
    memcpy(pcmdwr_, req, szwr);
    cmds_[cmdwrcnt_].req = pcmdwr_;
    cmds_[cmdwrcnt_].resp = resp;
    cmds_[cmdwrcnt_].silent = silent;
    cmds_[cmdwrcnt_].iface = iface;
    pcmdwr_ += szwr;
    RISCV_memory_barrier();
    ++cmdwrcnt_;   // CMD_QUEUE_SIZE = 256
}

void GuiPlugin::removeFromQueue(IFace *iface) {
    CmdType *pcmd;
    uint8_t rd = cmdrdcnt_;
    while (rd != cmdwrcnt_) {
        pcmd = &cmds_[rd];
        if (pcmd->iface == iface) {
            pcmd->iface = 0;
            pcmd->resp = 0;
        }
        ++rd;
    }
}

void GuiPlugin::externalCommand(AttributeType *req) {
    ui_->externalCommand(req);
}

void GuiPlugin::hapTriggered(EHapType type,
                             uint64_t param,
                             const char *descr) {
    RISCV_event_set(&config_done_);
}

void GuiPlugin::busyLoop() {
    RISCV_event_wait(&config_done_);

    while (isEnabled()) {
        if (cmdwrcnt_ == cmdrdcnt_) {
            RISCV_sleep_ms(50);
            continue;
        }

        processCmdQueue();
    }
    ui_->gracefulClose();
    delete ui_;
}

bool GuiPlugin::processCmdQueue() {
    CmdType *pcmd;

    while (cmdrdcnt_ != cmdwrcnt_) {
        pcmd = &cmds_[cmdrdcnt_];
        if (pcmd->resp) {
            iexec_->exec(pcmd->req, pcmd->resp, pcmd->silent);
        }
        if (pcmd->iface) {
            pcmd->iface->handleResponse(pcmd->req);
        }
        RISCV_memory_barrier();
        ++cmdrdcnt_;
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
