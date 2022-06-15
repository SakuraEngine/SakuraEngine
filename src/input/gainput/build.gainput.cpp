// 重定义太严重了 先不unity

// #include "gainput.cpp"
// #include "GainputAllocator.cpp"
// #include "GainputInputDeltaState.cpp"
// #include "GainputInputDevice.cpp"
// #include "GainputInputManager.cpp"
// #include "GainputInputMap.cpp"
// #include "GainputInputState.cpp"
// #include "GainputMapFilters.cpp"

// #include "keyboard/GainputInputDeviceKeyboard.cpp"
// #include "touch/GainputInputDeviceTouch.cpp"
// #include "pad/GainputInputDevicePad.cpp"
// #include "mouse/GainputInputDeviceMouse.cpp"
// #include "builtin/GainputInputDeviceBuiltIn.cpp"

// #include "hid/GainputHID.cpp"
// #include "hid/GainputHIDWhitelist.cpp"

#ifdef GAINPUT_DEV
    // #include "dev/GainputDev.cpp"
    // #include "dev/GainputMemoryStream.cpp"
    // #include "dev/GainputNetAddress.cpp"
    // #include "dev/GainputNetConnection.cpp"
    // #include "dev/GainputNetListener.cpp"
#endif

#ifdef GAINPUT_ENABLE_ALL_GESTURES
    // #include "gestures/GainputButtonStickGesture.cpp"
    // #include "gestures/GainputDoubleClickGesture.cpp"
    // #include "gestures/GainputHoldGesture.cpp"
    // #include "gestures/GainputPinchGesture.cpp"
    // #include "gestures/GainputRotateGesture.cpp"
    // #include "gestures/GainputSimultaneouslyDownGesture.cpp"
    // #include "gestures/GainputTapGesture.cpp"
#endif

#ifdef GAINPUT_ENABLE_RECORDER
    // #include "recorder/GainputInputPlayer.cpp"
    // #include "recorder/GainputInputRecorder.cpp"
    // #include "recorder/GainputInputRecording.cpp"
#endif