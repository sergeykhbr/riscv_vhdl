/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <tfifo.h>
#include <icommand.h>
#include "utils.h"
#include "server.h"
#include "c_dpi.h"

enum EDpiStateType {
    State_Idle,
    State_WaitAccept,
    State_WaitResp
};

static EDpiStateType estate_ = State_Idle;
static dpi_sv_interface dpi_ = {0};
static TFifo<ICommand *> fifo_(10);
static DpiServer server_;
static AttributeType tcpreq_;
static AttributeType tcpresp_;
static ICommand* isrc_ = 0;
static char descr_[256];

void dpi_put_fifo_request(void *iface) {
    ICommand *t1 = reinterpret_cast<ICommand *>(iface);
    fifo_.put(&t1);
}

int dpi_load_interface(dpi_sv_interface *iface) {
    int ret = 0;
    iface->sv_func_info = 
        (sv_func_info_proc)LIB_get_proc_addr("sv_func_info");
    if (!iface->sv_func_info) {
        LIB_printf("Can't find %s\n", "sv_func_info");
        ret = -1;
    }
    return ret;
}

extern "C" int c_task_server_start() {
    AttributeType tcp_resp;
    AttributeType srv_config(Attr_Dict);
    LIB_init();

    // TODO: external JSON configuration
    srv_config["BlockingMode"].make_boolean(true);
    srv_config["HostIP"].make_string("");
    srv_config["HostPort"].make_uint64(8689);
    srv_config["ClientConfig"].make_dict();
    srv_config["ClientConfig"]["Timeout"].make_int64(1000);

    if (dpi_load_interface(&dpi_)) {
        return -1;
    }

    server_.postinit(srv_config);
    LIB_printf("%s", "Entering c_task_server_start\n");

    if (server_.run()) {
        LIB_sprintf(descr_, sizeof(descr_), "c_task_server_start: %s",
            "TCP server run successfully. See dpilib.log file for more "
            "details");
    } else {
        LIB_sprintf(descr_, sizeof(descr_), "%s",
                    "c_task_server_start: TCP server start failed");
    }
	dpi_.sv_func_info(descr_);

    //server.stop();
    LIB_printf("c_task_server_start: %s\n", "end");
    //LIB_cleanup();
    return 0;
}

extern "C" int c_task_clk_posedge(const sv_out_t *sv2c, sv_in_t *c2sv) {
    c2sv->req_valid = 0;
    if (fifo_.is_empty() && estate_ == State_Idle) {
        return 0;
    }

    if (estate_ == State_Idle) {
        fifo_.get(&isrc_);
        tcpreq_ = isrc_->getRequest();
        tcpresp_.make_list(Resp_ListSize);

        if (!tcpreq_.is_list() || tcpreq_.size() < Req_Data) {
            tcpresp_[Resp_CmdType].make_string("Error");
            tcpresp_[Resp_Data].make_string("unsupported message format");
            isrc_->setResponse(tcpresp_);
            return 0;
        }
    }

    
    /** TCP Server's messages */
    if (tcpreq_[Req_SourceName].is_equal("DpiServer")) {
        /** Managing requests: add/remove client, errors, status */
        if (tcpreq_[Req_CmdType].is_equal("HartBeat")) {
            if (tcpreq_[Req_Data].to_uint32() == 0) {
                LIB_sprintf(descr_, sizeof(descr_), "%s",
                    "Waiting simulator connection...");
                dpi_.sv_func_info(descr_);
            } else {
                // Do nothing
            }
        } else {
            LIB_sprintf(descr_, sizeof(descr_), "DpiServer: %s\n",
                tcpreq_[Req_CmdType].to_string());
            dpi_.sv_func_info(descr_);
        }
        tcpresp_[Resp_CmdType].make_string(tcpreq_[Req_CmdType].to_string());
        AttributeType &od = tcpresp_[Resp_Data];
        od.make_dict();
        od["tm"].make_floating(sv2c->tm);
        od["clkcnt"].make_uint64(sv2c->clkcnt);
        isrc_->setResponse(tcpresp_);
        return 0;
    }
    
    /** Messages from TCP clients: */
    if (!tcpreq_[Req_CmdType].is_equal("AXI4")
        && !tcpreq_[Req_CmdType].is_equal("HartBeat")) {
        tcpresp_[Resp_CmdType].make_string("Error");
        tcpresp_[Resp_Data].make_string("wrong command type");
        isrc_->setResponse(tcpresp_);
        return 0;
    }

    if (tcpreq_[Req_CmdType].is_equal("HartBeat")) {
        if (sv2c->irq_request) {
            // TODO: irq request support
            tcpresp_[Resp_CmdType].make_string("AXI4");
        } else {
            tcpresp_[Resp_CmdType].make_string("HartBeat");
        }
        AttributeType &od = tcpresp_[Resp_Data];
        od.make_dict();
        od["tm"].make_floating(sv2c->tm);
        od["clkcnt"].make_uint64(sv2c->clkcnt);
        isrc_->setResponse(tcpresp_);
        return 0;
    }

    const AttributeType& slvi = tcpreq_[Req_Data];
    switch (estate_) {
    case State_Idle:
        c2sv->req_valid = 1;
        c2sv->req_type = REQ_TYPE_AXI4;
        c2sv->slvi.addr = slvi["addr"].to_uint64();
        c2sv->slvi.we = slvi["we"].to_bool();
        if (sv2c->req_ready) {
            estate_ = State_WaitResp;
        } else {
            estate_ = State_WaitAccept;
        }
        break;
    case State_WaitAccept:
        c2sv->req_valid = 1;
        c2sv->req_type = REQ_TYPE_AXI4;
        c2sv->slvi.addr = slvi["addr"].to_uint64();
        c2sv->slvi.we = slvi["we"].to_bool();
        if (sv2c->req_ready) {
            estate_ = State_WaitResp;
        }
        break;
    case State_WaitResp:
        if (sv2c->resp_valid) {
            tcpresp_[Resp_CmdType].make_string("AXI4");
            AttributeType &od = tcpresp_[Resp_Data];
            od.make_dict();
            od["tm"].make_floating(sv2c->tm);
            od["clkcnt"].make_uint64(sv2c->clkcnt);
            od["rdata"].make_list(1);
            od["rdata"][0u].make_uint64(sv2c->slvo.rdata[0]);
            isrc_->setResponse(tcpresp_);
            estate_ = State_Idle;
        }
        break;
    default:;
    }

    return 0;
}

