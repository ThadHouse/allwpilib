#include "hal/HMB.h"
#include "hal/ChipObject.h"
#include <FRC_FPGA_ChipObject/fpgainterfacecapi/NiFpga_HMB.h>

using namespace hal;

extern "C" {

tHMB* hmb;
uint32_t* buffer;

HAL_HMBHandle HAL_InitializeHMB(int32_t* status) {
    hmb = tHMB::create(status);

    auto cfg = hmb->readConfig(status);
    //cfg.Enables_Timestamp = 1;
    //cfg.Enables_DI = 1;
    //cfg.Enables_PWM = 1;
    //cfg.Enables_PWM_MXP = 1;
    //cfg.Enables_AIAveraged0_Low = 1;
    //cfg.Enables_AIAveraged0_High = 1;
    //cfg.Enables_Accumulator0 = 1; // 32-36
    //cfg.Enables_Accumulator1 = 1; // 48-52
    hmb->writeConfig(cfg, status);

    uint32_t session = hmb->getSystemInterface()->getHandle();
    size_t size = 0;
    *status = NiFpga_OpenHostMemoryBuffer(session, "HMB_0_RAM", (void**)&buffer,
                                         &size);
    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
    }
    asm("dmb");

    return 0;
}

void HAL_FreeHMB(HAL_HMBHandle handle) {
    delete hmb;
    hmb = nullptr;
}

void HAL_StrobeHMB(HAL_HMBHandle handle, int32_t* status) {
    asm("dmb");
}

uint64_t HAL_ReadHMBFPGATimestamp(HAL_HMBHandle handle, int32_t* status) {
    return ((uint64_t*)buffer)[120];
}

uint32_t HAL_ReadHMBDIO(HAL_HMBHandle handle, int32_t* status) {
    return buffer[64];
}

uint16_t HAL_ReadHMBPWM(HAL_HMBHandle handle, int32_t index, int32_t* status) {
    if (index < 10) {
        return buffer[192 + index];
    } else {
        return buffer[208 + index - 10];
    }
}

uint16_t HAL_ReadHMBAnalogInput(HAL_HMBHandle handle, int32_t index, int32_t* status) {
    return buffer[index];
}

uint16_t HAL_ReadHMBAveragedAnalogInput(HAL_HMBHandle handle, int32_t index, int32_t* status) {
    return buffer[16 + index];
}

uint32_t* HAL_GetHMBBuffer(HAL_HMBHandle handle, int32_t* status) {
    return buffer;
}

}
