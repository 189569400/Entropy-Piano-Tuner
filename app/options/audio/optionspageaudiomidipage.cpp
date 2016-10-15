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
#include <QCheckBox>

namespace options {

PageAudioMidi::PageAudioMidi(OptionsDialog *optionsDialog, QMidiAutoConnector *autoConnector)
    : mAutoConnector(autoConnector) {
    QGridLayout *inputLayout = new QGridLayout;
    this->setLayout(inputLayout);

    inputLayout->setColumnStretch(1, 1);

    inputLayout->addWidget(new QLabel(tr("Midi device")), 0, 0);
    inputLayout->addWidget(mDeviceSelection = new QComboBox(), 0, 1);

    auto acCheckBox = new QCheckBox(tr("Auto connect"), this);
    acCheckBox->setChecked(mAutoConnector->isAutoConnectEnabled(QMidi::MidiInput));
    connect(acCheckBox, &QCheckBox::toggled, mAutoConnector, &QMidiAutoConnector::setAutoConnectToInput);
    connect(mAutoConnector, &QMidiAutoConnector::autoConnectToInputChanged, acCheckBox, &QCheckBox::setChecked);

    inputLayout->addWidget(acCheckBox, 1, 0);

    inputLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), 20, 0);


    updateMidiInputDevices();

    // notify if changes are made
    QObject::connect(mDeviceSelection, SIGNAL(currentIndexChanged(int)), optionsDialog, SLOT(onChangesMade()));

    // create system notifier to listen to new devices
    auto systemNotifier = new QMidiSystemNotifier(this);
    connect(systemNotifier, &QMidiSystemNotifier::inputDeviceAttached, this, &PageAudioMidi::inputDeviceAttached);
    connect(systemNotifier, &QMidiSystemNotifier::inputDeviceDetached, this, &PageAudioMidi::inputDeviceDetached);

    connect(systemNotifier, &QMidiSystemNotifier::inputDeviceCreated, this, &PageAudioMidi::inputDeviceCreated);
}

void PageAudioMidi::apply() {
    if (mDeviceSelection->currentIndex() >= 0) {
        QMidiDeviceInfo info = mDeviceSelection->currentData().value<QMidiDeviceInfo>();
        if (info.isNull()) {
            mAutoConnector->setAutoConnect(QMidi::MidiInput, false);
            mAutoConnector->disconnectFromAll(QMidi::MidiInput);
        } else {
            mAutoConnector->connectWith(info, QMidi::MidiInput);
        }
    }
}

void PageAudioMidi::updateMidiInputDevices() {
    bool oldBlock = mDeviceSelection->blockSignals(true);

    mDeviceSelection->clear();
    mDeviceSelection->addItem(tr("Disabled"), QVariant::fromValue(QMidiDeviceInfo()));

    int curIndex = 0;
    auto inputDevices = QMidiDeviceInfo::availableDevices(QMidi::MidiInput);
    const QMidiInput *connectedDevice = mAutoConnector->device<QMidiInput>();
    QMidiDeviceInfo connectedDeviceInfo = (connectedDevice) ? connectedDevice->deviceInfo() : QMidiDeviceInfo();

    for (const QMidiDeviceInfo &info : inputDevices) {
        mDeviceSelection->addItem(info.deviceName(), QVariant::fromValue(info));
        if (info == connectedDeviceInfo) {
            curIndex = mDeviceSelection->count() - 1;
        }
    }

    setDisabled(inputDevices.size() == 0);

    mDeviceSelection->setCurrentIndex(curIndex);

    mDeviceSelection->blockSignals(oldBlock);
}

void PageAudioMidi::inputDeviceCreated(const QMidiInput *d) {
    if (mDeviceSelection->currentData().value<QMidiDeviceInfo>() == d->deviceInfo()) {
        return;
    }

    for (int i = 0; i < mDeviceSelection->count(); ++i) {
        if (mDeviceSelection->itemData(i).value<QMidiDeviceInfo>() == d->deviceInfo()) {
            mDeviceSelection->setCurrentIndex(i);
            break;
        }
    }
}

}  // namespace midi
