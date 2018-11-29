#include "frc/shuffleboard/SendableCameraWrapper.h"

constexpr const char* PROTOCOL = "camera_server://";

using namespace frc;

SendableCameraWrapper::SendableCameraWrapper(cs::VideoSource source) :
  SendableBase(false) {
    auto name = source.GetName();
    SetName(name);
    m_uri = std::string(PROTOCOL) + name;
}

SendableCameraWrapper SendableCameraWrapper::Wrap(cs::VideoSource source) {

}

void SendableCameraWrapper::InitSendable(SendableBuilder& builder) {
  builder.AddStringProperty(".ShuffleboardURI", [=](){ return this->m_url; }, nullptr);
}
