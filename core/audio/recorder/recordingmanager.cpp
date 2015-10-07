/*****************************************************************************
 * Copyright 2015 Haye Hinrichsen, Christoph Wick
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

//=============================================================================
//                           Recording manager
//=============================================================================

#include "recordingmanager.h"

#include "../../messages/messageprojectfile.h"
#include "../../messages/messagekeyselectionchanged.h"
#include "../../messages/messagemodechanged.h"

RecordingManager::RecordingManager  (AudioRecorderAdapter *audioRecorder)
 : mAudioRecorder (audioRecorder),
   mPiano(nullptr),
   mOperationMode(MODE_IDLE),
   mSelectedKey(nullptr),
   mKeyNumberOfA4(88),
   mNumberOfSelectedKey(-1)
{
   mAudioRecorder->getStroboscope()->setFramesPerSecond(FPS_SLOW);
}


//-----------------------------------------------------------------------------
//               Message handler  (for standby and stroboscope)
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Listen to messages and take action accordingly
/// \param m : The incoming message
///////////////////////////////////////////////////////////////////////////////

void RecordingManager::handleMessage(MessagePtr m)
{
    switch (m->getType())
    {
        case Message::MSG_PROJECT_FILE:
        {
            // In case that e.g. a new file was opened:
            auto mpf(std::static_pointer_cast<MessageProjectFile>(m));
            mPiano = &mpf->getPiano();
            mKeyNumberOfA4 = mPiano->getKeyboard().getKeyNumberOfA4();
            mSelectedKey = nullptr;
            mNumberOfSelectedKey = -1;
            updateStroboscopicFrequencies();
            break;
        }
        case Message::MSG_SIGNAL_ANALYSIS_ENDED:
        {
            // Tell the audio recorder to wait after completed analysis
            // in order to avoid self-recording of the echo sound
            mAudioRecorder->setWaitingFlag (false);
            break;
        }
        case Message::MSG_MODE_CHANGED:
        {
            // Change of the operation mode:
            auto mmc(std::static_pointer_cast<MessageModeChanged>(m));
            mOperationMode = mmc->getMode();
            switch (mOperationMode)
            {
                case MODE_TUNING:
                    mAudioRecorder->getStroboscope()->start();
                    mAudioRecorder->setStandby(false);
                break;
                case MODE_RECORDING:
                    mAudioRecorder->getStroboscope()->stop();
                    mAudioRecorder->setStandby(false);
                break;
                default:
                    mAudioRecorder->getStroboscope()->stop();
                    mAudioRecorder->setStandby(true);
                break;
            }
            break;
        }
        case Message::MSG_KEY_SELECTION_CHANGED:
        {
            // If a key is selected update the stroboscopic frequencies
            if (mOperationMode == MODE_TUNING)
            {
                auto message(std::static_pointer_cast<MessageKeySelectionChanged>(m));
                mSelectedKey = message->getKey();
                mNumberOfSelectedKey = message->getKeyNumber();
                updateStroboscopicFrequencies();
            }
            break;
        }
        case Message::MSG_RECORDING_STARTED:
        {
            if (mOperationMode == MODE_TUNING)
                mAudioRecorder->getStroboscope()->setFramesPerSecond(FPS_FAST);
            break;
        }
        case Message::MSG_RECORDING_ENDED:
        {
            if (mOperationMode == MODE_TUNING)
                mAudioRecorder->getStroboscope()->setFramesPerSecond(FPS_SLOW);
            break;
        }
        default:
            break;
    }
}


//-----------------------------------------------------------------------------
//                 Update the stroboscopic frequencies
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Update the stroboscopic frequencies
///
/// This function transmits a vector of frequencies of the lowest partials
/// of the selected key to the stroboscope. The function determines how
/// many partials are shown in the stroboscope.
///////////////////////////////////////////////////////////////////////////////

void RecordingManager::updateStroboscopicFrequencies()
{
    std::vector<double> ftab;
    if (mSelectedKey)
    {        
        const double fc = mSelectedKey->getComputedFrequency();
        if (fc > 0)
        {
            const Key::PeakListType peaks = mSelectedKey->getPeaks();
            // This is the formula determining the number of partials shown in the stroboscope:
            const int numberOfStroboscopicPartials = std::max(1,1+(mKeyNumberOfA4+6+24-mNumberOfSelectedKey)/6);

            // if peaks are available
            if (peaks.size()>0)
            {
                const double f1 = peaks.begin()->first;
                int N = 0;

                if (f1>0) for (auto &e : peaks)
                {
                    if (++N > numberOfStroboscopicPartials) break;
                    // Push partials adjusted by concert pitch
                    ftab.push_back(e.first/f1*mPiano->getConcertPitch()/440.0*fc);
                }
            }
            else // if no peaks are available
            {
                // Push partials with the expected harmonic spectrum
                const double B = mPiano->getExpectedInharmonicity (fc);
                for (int n=1; n<=3; ++n) ftab.push_back(n*fc*sqrt((1+B*n*n)/1+B));
            }
        }
    }
    mAudioRecorder->getStroboscope()->setFrequencies(ftab);
}
