#include "hal/proto/JoystickDescriptor.h"

#include "wpi/protobuf/ProtobufCallbacks.h"

std::optional<mrc::JoystickDescriptor>
wpi::Protobuf<mrc::JoystickDescriptor>::Unpack(InputStream& Stream) {
    wpi::UnpackCallback<std::string> JoystickNameCb;
    wpi::UnpackCallback<uint8_t, MRC_MAX_NUM_AXES> AxisTypesCb;

    mrc_proto_ProtobufJoystickDescriptor Msg;
    Msg.JoystickName = JoystickNameCb.Callback();
    Msg.AxisTypes = AxisTypesCb.Callback();

    if (!Stream.Decode(Msg)) {
        return {};
    }

    auto JoystickName = JoystickNameCb.Items();
    auto AxisTypes = AxisTypesCb.Items();

    if (JoystickName.empty()) {
        return {};
    }

    mrc::JoystickDescriptor OutputData;
    OutputData.MoveName(std::move(JoystickName[0]));

    OutputData.SetAxesCount(AxisTypes.size());

    for (size_t i = 0; i < OutputData.GetAxesCount(); i++) {
        OutputData.AxesTypes()[i] = AxisTypes[i];
    }

    OutputData.SetPovsCount(Msg.PovCount);
    OutputData.SetButtonsCount(Msg.ButtonCount);

    OutputData.IsXbox = Msg.IsXbox ? 1 : 0;
    OutputData.Type = Msg.JoystickType;

    return OutputData;
}

bool wpi::Protobuf<mrc::JoystickDescriptor>::Pack(
    OutputStream& Stream, const mrc::JoystickDescriptor& Value) {
    std::string_view JoystickName = Value.GetName();
    wpi::PackCallback JoystickNameCb{&JoystickName};

    std::span<const uint8_t> AxisTypes = Value.AxesTypes();
    wpi::PackCallback AxisTypesCb{AxisTypes};

    mrc_proto_ProtobufJoystickDescriptor Msg{
        .JoystickName = JoystickNameCb.Callback(),
        .AxisTypes = AxisTypesCb.Callback(),
        .IsXbox = Value.IsXbox ? true : false,
        .JoystickType = Value.Type,
        .ButtonCount = static_cast<int32_t>(Value.GetButtonsCount()),
        .PovCount = static_cast<int32_t>(Value.GetPovsCount()),
    };

    return Stream.Encode(Msg);
}