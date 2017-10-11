#include "HAL/athena/DirectFPGAAccess.h"

namespace hal {
    std::shared_ptr<tAI> GetAnalogInputSystem() {
        return nullptr;
    }

    std::shared_ptr<tAO> GetAnalogOutputSystem() {
        return nullptr;
    }

    std::shared_ptr<tDIO> GetDigitalSystem() {
        return nullptr;
    }

    std::shared_ptr<tRelay> GetRelaySystem() {
        return nullptr;
    }

    std::shared_ptr<tPWM> GetPWMSystem() {
        return nullptr;
    }

    std::shared_ptr<tSPI> GetSPISystem() {
        return nullptr;
    }
}
