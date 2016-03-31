/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU simlator class definition.
 */

#include "api_core.h"
#include "cpurtl.h"
#include "types_amba.h"

namespace debugger {

CpuRiscV_RTL::CpuRiscV_RTL(const char *name)  : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHostIO *>(this));
    registerAttribute("ParentThread", &parentThread_);
    registerAttribute("PhysMem", &phys_mem_);

    parentThread_.make_string("");
    phys_mem_.make_string("");
    stepQueue_.make_list(0);
    stepQueue_len_ = 0;

    rtl_ = new Vtop();

    tick_cnt_ = 0;
    reset_ = 1;
    state_ = IDLE;
    host_state_ = HOST_IDLE;
    memset(&hosti_, 0, sizeof(hosti_));
    hosti_.csr_resp_ready = 1;
}

CpuRiscV_RTL::~CpuRiscV_RTL() {
    delete rtl_;
}

void CpuRiscV_RTL::postinitService() {
    bool isEnable = false;
    IThread *iparent = static_cast<IThread *>(
       RISCV_get_service_iface(parentThread_.to_string(), IFACE_THREAD));
    if (iparent) {
        isEnable = iparent->isEnabled();
    }
    imemop_ = static_cast<IMemoryOperation *>(
       RISCV_get_service_iface(phys_mem_.to_string(), IFACE_MEMORY_OPERATION));

    if (!imemop_) {
        RISCV_error("Physical Memory interface '%s' not found", 
                    phys_mem_.to_string());
        return;
    }

    if (isEnable) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void CpuRiscV_RTL::predeleteService() {
    stop();
}

void CpuRiscV_RTL::busyLoop() {
    // @todo config_done event callback
    RISCV_sleep_ms(200);

    while (loopEnable_) {
        if ((++tick_cnt_) == 20) {
            reset_ = 0;
        }
        tickUpdate();

        if (tick_cnt_ & 0x1LL) {
            // Step Queue update:
            uint64_t ev_time;
            IClockListener *iclk;
            for (unsigned i = 0; i < stepQueue_len_; i++) {
                ev_time = stepQueue_[i][Queue_Time].to_uint64();
                iclk = static_cast<IClockListener *>(
                        stepQueue_[i][Queue_IFace].to_iface());
                if (ev_time <= tick_cnt_/2) {
                    iclk->stepCallback(tick_cnt_/2);
                    stepQueue_.swap_list_item(i, stepQueue_.size() - 1);
                    stepQueue_len_--;
                    i--;
                }
            }
        }
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

void CpuRiscV_RTL::tickUpdate() {
    rtl_->clk = static_cast<uint8_t>(tick_cnt_ & 0x1LL);
    rtl_->reset = reset_;
   	rtl_->io_cached_0_acquire_ready = cti_.acquire_ready;
	rtl_->io_cached_0_grant_valid   = cti_.grant_valid;
	rtl_->io_cached_0_grant_bits_addr_beat = cti_.grant_bits_addr_beat;
	rtl_->io_cached_0_grant_bits_client_xact_id 
        = cti_.grant_bits_client_xact_id;
	rtl_->io_cached_0_grant_bits_manager_xact_id 
        = cti_.grant_bits_manager_xact_id;
	rtl_->io_cached_0_grant_bits_is_builtin_type 
        = cti_.grant_bits_is_builtin_type;
	rtl_->io_cached_0_grant_bits_g_type = cti_.grant_bits_g_type;
    for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
	    rtl_->io_cached_0_grant_bits_data[i] = cti_.grant_bits_data[i];
    }
	rtl_->io_cached_0_probe_valid = cti_.probe_valid;
	rtl_->io_cached_0_probe_bits_addr_block = cti_.probe_bits_addr_block;
	rtl_->io_cached_0_probe_bits_p_type = cti_.probe_bits_p_type;
	rtl_->io_cached_0_release_ready = cti_.release_ready;
	rtl_->io_uncached_0_acquire_ready = uti_.acquire_ready;
	rtl_->io_uncached_0_grant_valid = uti_.grant_valid;
	rtl_->io_uncached_0_grant_bits_addr_beat       
        = uti_.grant_bits_addr_beat;
	rtl_->io_uncached_0_grant_bits_client_xact_id  
        = uti_.grant_bits_client_xact_id;
	rtl_->io_uncached_0_grant_bits_manager_xact_id 
        = uti_.grant_bits_manager_xact_id;
	rtl_->io_uncached_0_grant_bits_is_builtin_type 
        = uti_.grant_bits_is_builtin_type;
	rtl_->io_uncached_0_grant_bits_g_type  = uti_.grant_bits_g_type;
    for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
	    rtl_->io_uncached_0_grant_bits_data[i] = uti_.grant_bits_data[i];
    }
	rtl_->io_host_reset = hosti_.reset;
	rtl_->io_host_id = hosti_.id;
	rtl_->io_host_csr_req_valid = hosti_.csr_req_valid;
	rtl_->io_host_csr_req_bits_rw = hosti_.csr_req_bits_rw;
	rtl_->io_host_csr_req_bits_addr = hosti_.csr_req_bits_addr;
	rtl_->io_host_csr_req_bits_data = hosti_.csr_req_bits_data;
	rtl_->io_host_csr_resp_ready = hosti_.csr_resp_ready;

    rtl_->eval();

    cto_.acquire_valid = rtl_->io_cached_0_acquire_valid;
	cto_.acquire_bits_addr_block 
        = rtl_->io_cached_0_acquire_bits_addr_block;
	cto_.acquire_bits_client_xact_id 
        = rtl_->io_cached_0_acquire_bits_client_xact_id;
    cto_.acquire_bits_addr_beat = rtl_->io_cached_0_acquire_bits_addr_beat;
	cto_.acquire_bits_is_builtin_type 
        = rtl_->io_cached_0_acquire_bits_is_builtin_type;
	cto_.acquire_bits_a_type = rtl_->io_cached_0_acquire_bits_a_type;
	cto_.acquire_bits_union = rtl_->io_cached_0_acquire_bits_union;
    for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
	    cto_.acquire_bits_data[i] = rtl_->io_cached_0_acquire_bits_data[i];
    }
	cto_.grant_ready = rtl_->io_cached_0_grant_ready;
	cto_.probe_ready = rtl_->io_cached_0_probe_ready;
	cto_.release_valid = rtl_->io_cached_0_release_valid;
	cto_.release_bits_addr_beat = rtl_->io_cached_0_release_bits_addr_beat;
	cto_.release_bits_addr_block 
        = rtl_->io_cached_0_release_bits_addr_block;
	cto_.release_bits_client_xact_id 
        = rtl_->io_cached_0_release_bits_client_xact_id;
	cto_.release_bits_voluntary = rtl_->io_cached_0_release_bits_voluntary;
	cto_.release_bits_r_type = rtl_->io_cached_0_release_bits_r_type;
    for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
	    cto_.release_bits_data[i] = rtl_->io_cached_0_release_bits_data[i];
    }
	uto_.acquire_valid = rtl_->io_uncached_0_acquire_valid;
	uto_.acquire_bits_addr_block 
        = rtl_->io_uncached_0_acquire_bits_addr_block;
	uto_.acquire_bits_client_xact_id 
        = rtl_->io_uncached_0_acquire_bits_client_xact_id;
	uto_.acquire_bits_addr_beat 
        = rtl_->io_uncached_0_acquire_bits_addr_beat;
	uto_.acquire_bits_is_builtin_type 
        = rtl_->io_uncached_0_acquire_bits_is_builtin_type;
	uto_.acquire_bits_a_type = rtl_->io_uncached_0_acquire_bits_a_type;
	uto_.acquire_bits_union = rtl_->io_uncached_0_acquire_bits_union;
    for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
        uto_.acquire_bits_data[i] 
                        = rtl_->io_uncached_0_acquire_bits_data[i];
    }
	uto_.grant_ready = rtl_->io_uncached_0_grant_ready;
    hosto_.csr_req_ready = rtl_->io_host_csr_req_ready;
    hosto_.csr_resp_valid = rtl_->io_host_csr_resp_valid;
    hosto_.csr_resp_bits = rtl_->io_host_csr_resp_bits;

    if (tick_cnt_ & 0x1LL) {
        hostBridgeUpdate();
        ambaBridgeUpdate();
    }
}

void CpuRiscV_RTL::hostBridgeUpdate() {
    switch (host_state_) {
    case HOST_IDLE:
        hosti_.csr_req_valid = 0;
        break;
    case HOST_WAIT_REQREADY:
        if (hosto_.csr_req_ready) {
            hosti_.csr_req_valid = 0;
            host_state_ = HOST_WAIT_RESPVALID;
        }
        break;
    case HOST_WAIT_RESPVALID:
        if (hosto_.csr_resp_valid) {
            host_state_ = HOST_IDLE;
        }
        break;
    default:
        host_state_ = HOST_IDLE;
    }
}

void CpuRiscV_RTL::ambaBridgeUpdate() {
    cti_.acquire_ready = 0;
    uti_.acquire_ready = 0;
    cti_.grant_valid = 0;
    uti_.grant_valid = 0;
    cti_.probe_valid = 0;
    uti_.probe_valid = 0;
    cti_.release_ready = 1;
    uti_.release_ready = 1;
    cti_.grant_bits_is_builtin_type = 1;
    uti_.grant_bits_is_builtin_type = 1;
    cti_.grant_bits_g_type          = GRANT_ACK_RELEASE;
    uti_.grant_bits_g_type          = GRANT_ACK_RELEASE;
    cti_.grant_bits_addr_beat = 0;
    uti_.grant_bits_addr_beat = 0;
    cti_.grant_bits_client_xact_id = 0;
    uti_.grant_bits_client_xact_id = 0;
    cti_.grant_bits_manager_xact_id = 0;
    uti_.grant_bits_manager_xact_id = 0;

    memset(cti_.grant_bits_data, 0, sizeof(uti_.grant_bits_data));
    memset(uti_.grant_bits_data, 0, sizeof(uti_.grant_bits_data));
    memset(&memop_, 0, sizeof(memop_));

    uint8_t write0_;
    uint32_t wmask0_;
    uint32_t axi_sz0_;
    uint32_t byte_addr0_;
    int32_t beat_cnt0_;

    procedureDecodeTileAcquire(cto_.acquire_bits_a_type,
                                cto_.acquire_bits_is_builtin_type,
                                cto_.acquire_bits_union,
                                write0_,
                                wmask0_,
                                axi_sz0_,
                                byte_addr0_,
                                beat_cnt0_);

    uint8_t write1_;
    uint32_t wmask1_;
    uint32_t axi_sz1_;
    uint32_t byte_addr1_;
    int32_t beat_cnt1_;

    procedureDecodeTileAcquire(uto_.acquire_bits_a_type,
                                uto_.acquire_bits_is_builtin_type,
                                uto_.acquire_bits_union,
                                write1_,
                                wmask1_,
                                axi_sz1_,
                                byte_addr1_,
                                beat_cnt1_);

    switch (state_) {
    case IDLE:
        if (cto_.acquire_valid) {
            tile_reg(&cto_, write0_, wmask0_, axi_sz0_, 
                                        byte_addr0_, beat_cnt0_);
            state_ = CACHED;
            cti_.acquire_ready = 1;
        } else if (uto_.acquire_valid) {
            tile_reg(&uto_, write1_, wmask1_, axi_sz1_, 
                                        byte_addr1_, beat_cnt1_);
            state_ = UNCACHED;
            uti_.acquire_ready = 1;
        }
        break;
    case CACHED:
        processMemopyOperation(cto_.acquire_bits_data, &cti_);
        break;
    case UNCACHED:
        processMemopyOperation(uto_.acquire_bits_data, &uti_);
        break;
    default:;
    }
}

void CpuRiscV_RTL::procedureDecodeTileAcquire( uint32_t a_type,
                                            uint8_t built_in,
                                            uint32_t u,
                                            uint8_t &write,
                                            uint32_t &wmask,
                                            uint32_t &axi_sz,
                                            uint32_t &byte_addr,
                                            int32_t &beat_cnt) {

    if (built_in == 1) {
        //-- Cached request
        switch (a_type) {
        case ACQUIRE_GET_SINGLE_DATA_BEAT:
            write = 0;
            wmask = 0;
            byte_addr = BITS32(u,12,9);
            axi_sz = opSizeToXSize[BITS32(u,8,6)];
            beat_cnt = 0;
            break;
        case ACQUIRE_PREFETCH_BLOCK:
        case ACQUIRE_GET_BLOCK_DATA:
            //-- cache line size / data bits width
            write = 0;
            wmask = 0;
            byte_addr = 0;
            axi_sz = CFG_NASTI_ADDR_OFFSET;
            beat_cnt = 3;
            break;
        case ACQUIRE_PUT_SINGLE_DATA_BEAT:
            //-- Single beat data.
            write = 1;
            wmask = BITS32(u,16,1);
            byte_addr = 0;
            axi_sz = CFG_NASTI_ADDR_OFFSET;
            beat_cnt = 0;
            break;
        case ACQUIRE_PUT_BLOCK_DATA:
            //-- Multibeat data.
            write = 1;
            wmask = 0xFFFF;
            byte_addr = 0;
            axi_sz = CFG_NASTI_ADDR_OFFSET;
            beat_cnt = 3;
            break;
        case ACQUIRE_PUT_ATOMIC_DATA:
            //-- Single beat data. 64 bits width
            write = 1;
            if (BIT32(u,12) == 0) {
                wmask = 0x00FF;
            } else {
                wmask = 0xFF00;
            }
            byte_addr = 0;
            axi_sz = opSizeToXSize[BITS32(u,8,6)];
            beat_cnt = 0; 
            break;
        default:
            write = 0;
            wmask = 0;
            byte_addr = 0;
            axi_sz = 0;
            beat_cnt = 0;
        }
    } else { // built_in = '0'
        //-- Cached request
        switch (a_type) {
        case CACHED_ACQUIRE_SHARED:
            write = 0;
            wmask = 0;
            byte_addr = BITS32(u,12,9);
            axi_sz = opSizeToXSize[BITS32(u,8,6)];
            beat_cnt = 0;
            break;
        case CACHED_ACQUIRE_EXCLUSIVE:
            //-- Single beat data.
            write = 1;
            wmask = BITS32(u,16,1);
            byte_addr = 0;
            axi_sz = CFG_NASTI_ADDR_OFFSET;
            beat_cnt = 0;
            break;
        default:
            write = 0;
            wmask = 0;
            byte_addr = 0;
            axi_sz = 0;
            beat_cnt = 0;
        }
    }
}

void CpuRiscV_RTL::tile_reg(tile_cached_out_type *itile,
                            uint8_t wWrite,
                            uint32_t wbMask,
                            uint32_t wbAxiSize,
                            uint32_t wbByteAddr,
                            int32_t wbBeatCnt) {
    r_write_ = wWrite;
    if (wWrite) {
        r_wr_addr_ = (itile->acquire_bits_addr_block << 6) 
                   | (itile->acquire_bits_addr_beat << 4);
        r_wr_addr_incr_ = XSizeToBytes[wbAxiSize];
        r_wr_beat_cnt_ = wbBeatCnt;
        r_wr_xsize_ = opSizeToXSize[BITS32(itile->acquire_bits_union,8,6)];
        r_wr_xact_id_ = itile->acquire_bits_client_xact_id;
        if (itile->acquire_bits_is_builtin_type) {
            r_wr_g_type_ = GRANT_ACK_NON_PREFETCH_PUT;
        } else {
            r_wr_g_type_ = CACHED_GRANT_EXCLUSIVE_ACK;
        }
        r_wmask_ = wbMask;
        for (uint64_t i = 0; i < CFG_NASTI_DATA_BYTES/4; i++) {
            r_wdata_[i] = itile->acquire_bits_data[i];
        }
    } else {
        r_rd_addr_ = (itile->acquire_bits_addr_block << 6) 
                   | (itile->acquire_bits_addr_beat << 4);
        r_rd_addr_incr_ = XSizeToBytes[wbAxiSize];
        r_rd_beat_cnt_ = wbBeatCnt;
        r_rd_xsize_ = opSizeToXSize[BITS32(itile->acquire_bits_union,8,6)];
        r_rd_xact_id_ = itile->acquire_bits_client_xact_id;
        if (itile->acquire_bits_is_builtin_type) {
            if (wbBeatCnt == 0) {
                r_rd_g_type_ = GRANT_SINGLE_BEAT_GET;
            } else {
                r_rd_g_type_ = GRANT_BLOCK_GET;
            }
        } else {
            r_rd_g_type_ = CACHED_GRANT_SHARED;
        }
    }
}

void CpuRiscV_RTL::processMemopyOperation(uint32_t *wdata, 
                                     tile_cached_in_type *otile) {
    if (r_write_) {
        memop_.rw = 1;
        memop_.addr = r_wr_addr_;
        memop_.wstrb = r_wmask_;
        memop_.bytes = r_wr_xsize_;
        for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
            memop_.wpayload[i] = r_wdata_[i];
        }
        memop_.xsize = CFG_NASTI_DATA_BYTES;
        imemop_->transaction(&memop_);

        otile->grant_valid                = 1;
        otile->grant_bits_addr_beat       = BITS32(r_wr_addr_, 5, 4);
        otile->grant_bits_client_xact_id  = r_wr_xact_id_;
        otile->grant_bits_g_type          = r_wr_g_type_;

        if (r_wr_beat_cnt_ == 0) {
            state_ = IDLE;
        } else {
            r_wr_beat_cnt_ -= 1;
            r_wr_addr_ += r_wr_beat_cnt_;
            for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
                r_wdata_[i] = wdata[i];
            }
        }
    } else {
        memop_.rw = 0;
        memop_.addr = r_rd_addr_;
        memop_.bytes = r_rd_xsize_;
        memop_.xsize = CFG_NASTI_DATA_BYTES;
        imemop_->transaction(&memop_);

        otile->grant_valid                = 1;
        otile->grant_bits_addr_beat       = BITS32(r_rd_addr_, 5, 4);
        otile->grant_bits_client_xact_id  = r_rd_xact_id_;
        otile->grant_bits_g_type          = r_rd_g_type_;
        for (uint64_t i = 0; i < CFG_NASTI_DATA_WORDS32; i++) {
            otile->grant_bits_data[i] = memop_.rpayload[i];
        }

        if (r_rd_beat_cnt_ == 0) {
            state_ = IDLE;
        } else {
            r_rd_addr_ += r_rd_addr_incr_;
            r_rd_beat_cnt_ -= 1;
        }
    }
}

void CpuRiscV_RTL::registerStepCallback(IClockListener *cb, uint64_t t) {
    AttributeType time(Attr_UInteger, t);
    AttributeType face(cb);
    if (stepQueue_len_ < stepQueue_.size()) {
        stepQueue_[stepQueue_len_][Queue_Time] = time;
        stepQueue_[stepQueue_len_][Queue_IFace] = face;
        stepQueue_len_++;
    } else {
        AttributeType item;
        item.make_list(Queue_Total);
        item[Queue_Time] = time;
        item[Queue_IFace] = face;
        stepQueue_.add_to_list(&item);
        stepQueue_len_++;
    }
}

uint64_t CpuRiscV_RTL::write(uint16_t adr, uint64_t val) {
    bool no_response = true;
    bool wait_response = false;
    while (no_response) {
        tick_cnt_++;
        tickUpdate();

        if ((tick_cnt_ & 0x1LL) == 0) {
            continue;
        }

        if (!wait_response && host_state_ == HOST_IDLE) {
            wait_response = true;
	        hosti_.csr_req_valid = 1;
	        hosti_.csr_req_bits_rw = 1;
	        hosti_.csr_req_bits_addr = adr;
	        hosti_.csr_req_bits_data = val;
            if (hosto_.csr_req_ready) {
                host_state_ = HOST_WAIT_RESPVALID;
            } else {
                host_state_ = HOST_WAIT_REQREADY;
            }
        }
        if (wait_response && hosto_.csr_resp_valid) {
            break;
        }
    }
    return hosto_.csr_resp_bits;
}

uint64_t CpuRiscV_RTL::read(uint16_t adr, uint64_t *val) {
    bool no_response = true;
    bool wait_response = false;
    while (no_response) {
        tick_cnt_++;
        tickUpdate();

        if ((tick_cnt_ & 0x1LL) == 0) {
            continue;
        }

        if (!wait_response && host_state_ == HOST_IDLE) {
            wait_response = true;
	        hosti_.csr_req_valid = 1;
	        hosti_.csr_req_bits_rw = 1;
	        hosti_.csr_req_bits_addr = adr;
	        hosti_.csr_req_bits_data = 0;
            if (hosto_.csr_req_ready) {
                host_state_ = HOST_WAIT_RESPVALID;
            } else {
                host_state_ = HOST_WAIT_REQREADY;
            }
        }
        if (wait_response && hosto_.csr_resp_valid) {
            break;
        }
    }
    return *val = hosto_.csr_resp_bits;
}

}  // namespace debugger

