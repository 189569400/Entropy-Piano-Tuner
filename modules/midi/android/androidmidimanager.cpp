#include "androidmidimanager.h"
#include "midisystem.h"
#include "androidnativewrapper.h"

namespace midi {

AndroidMidiManager::AndroidMidiManager() {

}

AndroidMidiManager::~AndroidMidiManager() {

}

MidiResult AndroidMidiManager::init_impl() {
    initAndroidManagerJNI(&mUsbMidiDriver);

    if (mUsbMidiDriver == 0) {
        return BACKEND_CREATION_ERROR;
    }

    return OK;
}

MidiResult AndroidMidiManager::exit() {
    releaseAndroidManagerJNI(mUsbMidiDriver);
    mUsbMidiDriver = 0;

    return OK;
}

std::vector<MidiDeviceID> AndroidMidiManager::listAvailableInputDevices() const {
    std::vector<MidiDeviceID> devices;
    return devices;
}

std::vector<MidiDeviceID> AndroidMidiManager::listAvailableOutputDevices() const {
    std::vector<MidiDeviceID> devices;
    return devices;
}

AndroidMidiManager::MidiInDevRes AndroidMidiManager::createInputDevice_impl(const MidiDeviceID id) {
    return std::make_pair(OK, MidiInputDevicePtr());
}

AndroidMidiManager::MidiOutDevRes AndroidMidiManager::createOutputDevice_impl(const MidiDeviceID id) {
    return std::make_pair(OK, MidiOutputDevicePtr());
}

MidiResult AndroidMidiManager::deleteDevice_impl(MidiInputDevicePtr device) {
    return OK;
}

MidiResult AndroidMidiManager::deleteDevice_impl(MidiOutputDevicePtr device) {
    return OK;
}

}  // namespace midi
