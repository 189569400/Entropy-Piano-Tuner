/*****************************************************************************
 * Copyright 2016 Haye Hinrichsen, Christoph Wick
 *
 * This file is part of Entropy Piano Tuner.
 *
 * Entropy Piano Tuner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Entropy Piano Tuner is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Entropy Piano Tuner. If not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

#include "optionspageaudiomidipage.h"
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "midi/midisystem.h"
#include "midi/midimanager.h"

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(midi::MidiDeviceID)

namespace options {

PageAudioMidi::PageAudioMidi(OptionsDialog *optionsDialog, MidiAdapter *midiInterface)
    : mMidiInterface(midiInterface) {
    QGridLayout *inputLayout = new QGridLayout;
    this->setLayout(inputLayout);

    inputLayout->setColumnStretch(1, 1);

    inputLayout->addWidget(new QLabel(tr("Midi device")), 0, 0);
    inputLayout->addWidget(mDeviceSelection = new QComboBox(), 0, 1);
    inputLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), 20, 0);

    updateMidiInputDevices();

    // notify if changes are made
    QObject::connect(mDeviceSelection, SIGNAL(currentIndexChanged(int)), optionsDialog, SLOT(onChangesMade()));

    midi::manager().addListener(this);
}

void PageAudioMidi::apply() {
    if (mDeviceSelection->currentIndex() >= 0) {
        midi::MidiDeviceID midiPort = mDeviceSelection->currentData().value<midi::MidiDeviceID>();
        if (!midiPort) {
            midi::MidiResult r = midi::manager().deleteAllInputDevices();
            if (r != midi::OK) {
                LogW("Could not delete all input devices. Code: %d", r);
            } else {
                LogI("Disconnected from midi devices.");
            }
        } else {
            midi::MidiResult r;
            midi::MidiInputDevicePtr device;
            std::tie(r, device) = midi::manager().createInputDevice(midiPort);
            if (r != midi::OK) {
                QMessageBox::warning(this, tr("MIDI error"),
                                     tr("Could not connect to midi device '%1'. Error code: %2'").arg(
                                         QString::fromStdString(midiPort->humanReadable()),
                                         QString::number(r)));
                LogW("Could not connect to midi device '%s'. Error code: %d", midiPort->humanReadable().c_str(), r);
            } else {
                LogI("Connected to midi device '%s'", midiPort->humanReadable().c_str());
            }
        }
    }
}

void PageAudioMidi::updateMidiInputDevices() {
    bool oldBlock = mDeviceSelection->blockSignals(true);

    mDeviceSelection->clear();
    mDeviceSelection->addItem(tr("Disabled"), QVariant::fromValue(midi::MidiDeviceID()));

    int curIndex = 0;
    std::vector<midi::MidiDeviceID> inputDevices = midi::manager().listAvailableInputDevices();
    for (midi::MidiDeviceID device : inputDevices) {
        mDeviceSelection->addItem(QString::fromStdString(device->humanReadable()), QVariant::fromValue(device));
        if (device->equals(midi::manager().getConnectedInputDeviceID())) {
            curIndex = mDeviceSelection->count() - 1;
        }
    }

    if (inputDevices.size() == 0) {
        this->setDisabled(true);
    }
    mDeviceSelection->setCurrentIndex(curIndex);

    mDeviceSelection->blockSignals(oldBlock);
}

}  // namespace midi
