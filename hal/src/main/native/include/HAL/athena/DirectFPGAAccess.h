#pragma once

#include "HAL/ChipObject.h"
#include <memory>


namespace hal {
    std::shared_ptr<tAI> GetAnalogInputSystem();
    std::shared_ptr<tAO> GetAnalogOutputSystem();
    std::shared_ptr<tDIO> GetDigitalSystem();
    std::shared_ptr<tRelay> GetRelaySystem();
    std::shared_ptr<tPWM> GetPWMSystem();
    std::shared_ptr<tSPI> GetSPISystem();
    std::shared_ptr<tCounter> GetCounterSubsystem();
    std::shared_ptr<tEncoder> GetEncoderSubsystem();
}
