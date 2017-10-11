#include "HAL/athena/DirectFPGAAccess.h"
#include "AnalogInternal.h"
#include "DigitalInternal.h"

namespace hal {
    std::shared_ptr<tAI> GetAnalogInputSystem() {
        int32_t status = 0;
        initializeAnalog(&status);
        return analogInputSystem;
    }

    std::shared_ptr<tAO> GetAnalogOutputSystem() {
        int32_t status = 0;
        initializeAnalog(&status);
        return analogOutputSystem;
    }

    std::shared_ptr<tDIO> GetDigitalSystem() {
        int32_t status = 0;
        intializeDigital(&status);
        return digitalSystem;
    }

    std::shared_ptr<tRelay> GetRelaySystem() {
        int32_t status = 0;
        intializeDigital(&status);
        return relaySystem;
    }

    std::shared_ptr<tPWM> GetPWMSystem() {
        int32_t status = 0;
        intializeDigital(&status);
        return pwmSystem;
    }

    std::shared_ptr<tSPI> GetSPISystem() {
        int32_t status = 0;
        intializeDigital(&status);
        return spiSystem;
    }
}
