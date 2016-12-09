/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SystemC CPU wrapper. To interact with the SoC simulator.
 */

#include "api_core.h"
#include "rtl_wrapper.h"
#if 1
#include "iservice.h"
#include "coreservices/iserial.h"
#endif

//#define SIMULATE_WAIT_STATES

namespace debugger {

RtlWrapper::RtlWrapper(sc_module_name name)
    : sc_module(name),
    o_clk("clk", 10, SC_NS) {

    clockCycles_ = 1000000; // 1 MHz when default resolution = 1 ps

    SC_METHOD(registers);
    sensitive << o_clk.posedge_event();

    SC_METHOD(comb);
    sensitive << i_req_mem_valid;
    sensitive << i_req_mem_addr;
    sensitive << r.nrst;
    sensitive << r.resp_mem_data_valid;
    sensitive << r.resp_mem_data;
    sensitive << r.interrupt;
    sensitive << r.wait_state_cnt;

    SC_METHOD(clk_negedge_proc);
    sensitive << o_clk.negedge_event();

    w_nrst = 0;
    v.nrst = 0;
    v.interrupt = false;
    w_interrupt = 0;
    v.resp_mem_data = 0;
    v.resp_mem_data_valid = false;
}

void RtlWrapper::clk_gen() {
    // todo: instead sc_clock
}

void RtlWrapper::comb() {

    o_nrst = r.nrst.read()[1];
    o_resp_mem_data_valid = r.resp_mem_data_valid;
    o_resp_mem_data = r.resp_mem_data;
    o_interrupt = r.interrupt;
#ifdef SIMULATE_WAIT_STATES
    if (r.wait_state_cnt.read() == 1) {
        o_req_mem_ready = 1;
    } else {
        o_req_mem_ready = 0;
    }
#else
    o_req_mem_ready = i_req_mem_valid;
#endif

    if (!r.nrst.read()[1]) {
    }
}

void RtlWrapper::registers() {
    r = v;
}

void RtlWrapper::clk_negedge_proc() {
    /** Simulation events queue */
    IFace *cb;

    // Clock queue
    clock_queue_.initProc();
    clock_queue_.pushPreQueued();
    uint64_t clk_cnt = i_timer.read();
    while ((cb = clock_queue_.getNext(clk_cnt)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(clk_cnt);
    }

    // Stepping queue (common for debug purposes)
    step_queue_.initProc();
    step_queue_.pushPreQueued();
    uint64_t step_cnt = i_step_cnt.read();
    while ((cb = step_queue_.getNext(step_cnt)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

#if (GENERATE_CORE_TRACE == 1)
    //if (!(step_cnt % 50000) && step_cnt != step_cnt_z) {
    //    printf("!!!!step_cnt = %d\n", (int)step_cnt);
    //}

    /*if (step_cnt == (6000 - 1) && step_cnt != step_cnt_z) {
        IService *uart = static_cast<IService *>(RISCV_get_service("uart0"));
        if (uart) {
            ISerial *iserial = static_cast<ISerial *>(
                        uart->getInterface(IFACE_SERIAL));
            //iserial->writeData("pnp\r\n", 5);
            //iserial->writeData("dhry\r\n", 6);
            iserial->writeData("highticks\r\n", 11);
        }
    }*/
    if (step_cnt != step_cnt_z) {
        char msg[16];
        int msg_len = 0;
        IService *uart = NULL;
        switch (step_cnt + 1) {
        case 6000:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'h';
            msg[1] = 'i';
            msg_len = 2;
            break;
        case 6500:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'g';
            msg[1] = 'h';
            msg[2] = 't';
            msg_len = 3;
            break;
        case 8200:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'i';
            msg[1] = 'c';
            msg_len = 2;
            break;
        case 8300:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'k';
            msg[1] = 's';
            msg[2] = '\r';
            msg[3] = '\n';
            msg_len = 4;
            break;
        default:;
        }
        if (uart) {
            ISerial *iserial = static_cast<ISerial *>(
                        uart->getInterface(IFACE_SERIAL));
            //iserial->writeData("pnp\r\n", 6);
            //iserial->writeData("highticks\r\n", 11);
            iserial->writeData(msg, msg_len);
        }
    }
    step_cnt_z = i_step_cnt.read();
#endif

    /** */
    v.interrupt = w_interrupt;
    v.nrst = (r.nrst.read() << 1) | w_nrst;
    v.wait_state_cnt = r.wait_state_cnt.read() + 1;


    v.resp_mem_data = 0;
    v.resp_mem_data_valid = false;
    bool w_req_fire = 0;
#ifdef SIMULATE_WAIT_STATES
    if (r.wait_state_cnt.read() == 1)
#endif
    {
        w_req_fire = i_req_mem_valid.read();
    }
    if (w_req_fire) {
        uint64_t addr = i_req_mem_addr.read();
        Reg64Type val;
        if (i_req_mem_write.read()) {
            uint8_t strob = i_req_mem_strob.read();
            uint64_t offset = mask2offset(strob);
            int size = mask2size(strob >> offset);

            addr += offset;
            val.val = i_req_mem_data.read();
            ibus_->write(addr, val.buf, size);
            v.resp_mem_data = 0;
        } else {
            ibus_->read(addr, val.buf, sizeof(val));
            v.resp_mem_data = val.val;
        }
        v.resp_mem_data_valid = true;
    }
}

uint64_t RtlWrapper::mask2offset(uint8_t mask) {
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (mask & 0x1) {
            return static_cast<uint64_t>(i);
        }
        mask >>= 1;
    }
    return 0;
}

uint32_t RtlWrapper::mask2size(uint8_t mask) {
    uint32_t bytes = 0;
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        if (!(mask & 0x1)) {
            break;
        }
        bytes++;
        mask >>= 1;
    }
    return bytes;
}

void RtlWrapper::setClockHz(double hz) {
    sc_time dt = sc_get_time_resolution();
    clockCycles_ = static_cast<int>((1.0 / hz) / dt.to_seconds() + 0.5);
}
    
void RtlWrapper::registerStepCallback(IClockListener *cb, uint64_t t) {
    step_queue_.put(t, cb);
}

void RtlWrapper::registerClockCallback(IClockListener *cb, uint64_t t) {
    clock_queue_.put(t, cb);
}

void RtlWrapper::raiseSignal(int idx) {
    switch (idx) {
    case CPU_SIGNAL_RESET:
        w_nrst = 1;
        break;
    case CPU_SIGNAL_EXT_IRQ:
        w_interrupt = true;
        break;
    default:;
    }
}

void RtlWrapper::lowerSignal(int idx) {
    switch (idx) {
    case CPU_SIGNAL_RESET:
        w_nrst = 0;
        break;
    case CPU_SIGNAL_EXT_IRQ:
        w_interrupt = false;
        break;
    default:;
    }
}



}  // namespace debugger

