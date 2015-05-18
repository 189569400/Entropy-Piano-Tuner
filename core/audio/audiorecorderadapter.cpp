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
//                         Adapter for audio input
//=============================================================================

// Test sine waves with: play -n synth sin 440 vol 0.1
// monitor system input with xoscope

#include "audiorecorderadapter.h"

#include <cmath>
#include <algorithm>
#include <iostream>

#include "../settings.h"
#include "../system/log.h"
#include "../messages/messagerecorderenergychanged.h"
#include "../messages/messagemodechanged.h"
#include "../messages/messagehandler.h"
#include "../math/mathtools.h"
#include "../system/eptexception.h"

//-----------------------------------------------------------------------------
//                            Various constants
//-----------------------------------------------------------------------------

const bool   AudioRecorderAdapter::VERBOSE = true;

/// Capacity of the local circular audio buffer in seconds
const int    AudioRecorderAdapter::BUFFER_SIZE_IN_SECONDS = 2;

/// Update interval in milliseconds, defining the packet size.
const int    AudioRecorderAdapter::UPDATE_IN_MILLISECONDS = 50;

/// Attack rate at which the sliding level is increased (1=instantly).
const double AudioRecorderAdapter::ATTACKRATE = 0.97;

/// Decay rate at which the sliding level goes down.
const double AudioRecorderAdapter::DECAYRATE  = 0.7;

/// Level below which retriggering (restart) is allowed.
const double  AudioRecorderAdapter::LEVEL_RETRIGGER = 0.3;

/// Level above which the recorder starts to operate.
const double  AudioRecorderAdapter::LEVEL_TRIGGER = 0.45;

/// Level above which the input gain is automatically reduced.
const double  AudioRecorderAdapter::LEVEL_CUTOFF = 0.9;


//-----------------------------------------------------------------------------
//                             Constructor
//-----------------------------------------------------------------------------

AudioRecorderAdapter::AudioRecorderAdapter() :
      mMuted(false),            // input device is not muted
      mGain(1),                 // Gain factor amplifying the PCM signal
      mCounter(0),              // Counter counting incoming PCM values
      mCounterThreshold(0),     // Counter threshold for a single packet
      mPacketM1(0),             // First intensity moment of a single packet
      mPacketM2(0),             // Second intensity moment of a single packet
      mSlidingLevel(0),         // Sliding VU level of the signal
      mStopLevel(0.1),          // Level at which recording stops
      mRecording(false),        // Flag for recording
      mRestartable(true),       // Flag if recorder is restartable (retrigger)
      mStandby(SBR_NONE),       // Flag for standby mode
      mPacketCounter(0),        // Counter for the number of packages
      mIntensityHistogram(),    // Histogram of intensities for level control
      mCurrentPacket(0)         // Local audio buffer
{
      setSamplingRate(44100);   // Set default sampling rate
}

//-----------------------------------------------------------------------------
//                           Reset noise level
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// This will clear the intensity histogramm
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::resetNoiseLevel()
{
    setVolume(1);
    mGain = 1;
    mIntensityHistogram.clear();
    mPacketCounter = 0;
}

//-----------------------------------------------------------------------------
//                           Mutes the input device
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// This will mute the input signal by sending 0 as input signal and
/// handling an input level of 0.
/// \param muted : Activate or deactivate the muting
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::setMuted(bool muted) {
    mMuted = muted;
}

//-----------------------------------------------------------------------------
//                           Set sampling rate
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// The implementation of the audio device is allowed to change the sampling
/// rate in which case this function is called. It adjusts the actual
/// maximal buffer size so that the maximal time of recording is kept constant.
///
/// \param rate : New sampling rate
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::setSamplingRate(uint16_t rate)
{
    AudioBase::setSamplingRate(rate);
    mCurrentPacket.resize(rate * BUFFER_SIZE_IN_SECONDS);
    mCounterThreshold =   rate * UPDATE_IN_MILLISECONDS / 1000;
}


//-----------------------------------------------------------------------------
//                      Read all data from the buffer
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Read all data from the internal buffer
///
/// Reads all data from the internal buffer into the packet vector and clears
/// the internal buffer. This function is called by the SignalAnalyzer.
/// \param packet : Reference to the packet where the new data is stored.
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::readAll(PacketType &packet)
{
    std::lock_guard<std::mutex> lock(mPacketAccessMutex);   // lock the function
    packet = mCurrentPacket.getOrderedData();               // copy the content
    mCurrentPacket.clear();                                 // clear audio buffer
}


//-----------------------------------------------------------------------------
//          Convert signal intensity to a VU level and vice versa
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Convert an intensity (variance) of the signal to a VU level
/// \param intensity : variance of the PCM signal
/// \return level to be displayed by the VU meter (not clipped to [0,1])
///////////////////////////////////////////////////////////////////////////////

double AudioRecorderAdapter::convertIntensityToLevel (double intensity)
{ return pow(intensity*mGain*mGain,0.25); }


///////////////////////////////////////////////////////////////////////////////
/// \brief Convert a VU level to the corresponding intensity.
/// This function is the inverse of convertIntensityToLevel.
/// \param level : Level used to display in a VU meter
/// \return Corresponding intensity
///////////////////////////////////////////////////////////////////////////////

double AudioRecorderAdapter::convertLevelToIntensity (double level)
{ return pow(level,4.0)/mGain/mGain; }


//-----------------------------------------------------------------------------
//                Push raw PCM data from the implementation
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief The implementation calls this function when new data is available.
///
/// This function is called by the implementation when newly read data is
/// ready to be pushed to the local buffer in the adapter. The implementation
/// has to convert the actual PCM data format to floating point values
/// in range [-1,1]. Then it has to call the present function.
///
/// The raw signal is multiplied by a gain factor mGain which is adjusted
/// dynamically during the recording process.
///
/// Since the PacketDataType defined in AudioBase (e.g. double) may differ
/// from the preferred data type in the implementation (e.g. float in
/// pulse audio), we define the function as a template for float or double.
///
/// \param data : Vector containing the raw pcm data to be copied
///////////////////////////////////////////////////////////////////////////////

template <class floatingpoint>
void AudioRecorderAdapter::pushRawData(const std::vector<floatingpoint> &data)
{
    std::lock_guard<std::mutex> lock(mPacketAccessMutex);

    for (auto &pcmvalue : data)
    {
        mCurrentPacket.push_back(pcmvalue);
        mPacketM1 += pcmvalue;
        mPacketM2 += pcmvalue*pcmvalue;

        if (++mCounter > mCounterThreshold) // if packet is complete
        {
            // Compute the intensity (variance) of the packet
            double intensity = (mPacketM2-mPacketM1*mPacketM1/mCounter)/mCounter;
            mPacketM1 = mPacketM2 = mCounter = 0;

            // Convert intensity to the corresponding VU level
            double level = MathTools::restrictToInterval(
                        convertIntensityToLevel(intensity),0,1);

            // Compute a sliding level with fast attack and slow deay
            if (level > mSlidingLevel)
                  mSlidingLevel += (level-mSlidingLevel)*ATTACKRATE;
            else  mSlidingLevel -= (mSlidingLevel-level)*DECAYRATE;

            if (mMuted)
            {
                // if muted, only send 0
                MessageHandler::send<MessageRecorderEnergyChanged>(MessageRecorderEnergyChanged::LevelType::LEVEL_INPUT, 0);
                controlRecordingState(0);
            }
            else
            {
                // Send it to the GUI VU meter
                MessageHandler::send<MessageRecorderEnergyChanged>(MessageRecorderEnergyChanged::LevelType::LEVEL_INPUT, mSlidingLevel);

                // Switch recording process on and off if necessary
                controlRecordingState(mSlidingLevel);

                // Adjust the input gain and noise cutoff automatically
                automaticControl (intensity,level);
            }
        }
    }
}

// The above template function shall be realized for double and float
template void AudioRecorderAdapter::pushRawData(const std::vector<double> &data);
template void AudioRecorderAdapter::pushRawData(const std::vector<float> &data);


void AudioRecorderAdapter::handleMessage(MessagePtr m)
{
    switch (m->getType())
    {
        case Message::MSG_SIGNAL_ANALYSIS_ENDED:
            mStandby -= mStandby & SBR_WAITING_FOR_ANALYISIS;
            break;
        case Message::MSG_MODE_CHANGED:
        {
            auto mmc(std::static_pointer_cast<MessageModeChanged>(m));
            switch (mmc->getMode())
            {
                case MODE_RECORDING:
                case MODE_TUNING:
                    mStandby -= mStandby & SBR_DEACTIVATED_BY_OPERATION_MODE;
                    break;
                default:
                    mStandby |= SBR_DEACTIVATED_BY_OPERATION_MODE;
                break;
            }
            break;
        }
        default:
            break;
    }
}


//-----------------------------------------------------------------------------
//           Automatic noise estimation and threshold control
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Automatic noise estimation and threshold adjustment
///
/// This function constructs a histogram of the intensities of all incoming
/// packages. Typically this histogram has a pronounced peak at the left
/// edge, representing the background noise of the microphone in periods
/// of silence. The function locates the edges of the spectrum and adjusts
/// the noise threshold as well as the gain factor.
///
/// \param intensity : Intensity (variance) of the incoming packet.
/// \param level : Actual level (shown on the VU meter) of the packet.
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::automaticControl (double intensity, double level)
{
    if (intensity == 0) return;

    // maximal raw data intensity
    if (intensity > 0.9) {
        // the volume change is sent to system
        setVolume(getVolume() * 0.9);
    }

    // level is the signal scaled with mGain

    // if the signal gets quieter mGain can programatically increase the volume

    // if level exceeds cutoff turn gain down
    if (level > LEVEL_CUTOFF) mGain *= 0.9;

    // compute intensity histogram in decibel
    double dB = 4.34294 * log(intensity);
    mIntensityHistogram[MathTools::roundToInteger(dB)]++;

    // after 10 packets if the histogram has enough entries
    if ((++mPacketCounter)%10==0 and mIntensityHistogram.size()>5)
    {
//        std::ofstream os("9-histogram.dat");
//        for (auto e : mIntensityHistogram)
//                    os << e.first << " " << e.second << std::endl;
//        os.close();

        // determine left and right boundary in a soft way
        double norm=0, MIN=0, MAX=0;
        for (auto &e : mIntensityHistogram)
        {
            norm += e.second;
            MIN += pow(fabs(static_cast<double>(e.first)),10) * e.second;
            MAX += exp(e.first) * e.second;
        }
        if (norm==0) return;
        double dBoff = -pow(MIN/norm,1.0/10) + 4;
        double dBmax = log(MAX/norm);

        // slowly adjust the gain according to the upper edge
        double gain_target = exp(-dBmax/4.34294/2.0);
        mGain += 0.02*(gain_target-mGain);

        // slowly adjust the noise cutoff according to the lower edge
        double newStopLevel = convertIntensityToLevel(exp (dBoff/4.34294));
        if (newStopLevel < LEVEL_RETRIGGER  and fabs(newStopLevel-mStopLevel)>0.01)
        {
            // notify that stop level changed (reposition off level in mainwindow)
            mStopLevel += 0.1*(newStopLevel-mStopLevel);
            MessageHandler::send<MessageRecorderEnergyChanged>(
                        MessageRecorderEnergyChanged::LevelType::LEVEL_OFF, mStopLevel);
        }

        // After 10 packages decrease the whole histogram by a factor.
        // This ensures that new data gradually overwrites older data.
        if (mPacketCounter >= 10) for (auto e: mIntensityHistogram) e.second /= 1.1;
    }
}


//-----------------------------------------------------------------------------
//              Control the beginning and the end of recording
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Control the beginning and the end of recording.
///
/// This function detects the beginning and the end of the recording period.
/// To this end it counts how many packages have a larger energy than the
/// recording trigger treshold. If there are several of them in a row, the
/// function sends a message that the recording started to all other modules.
///
/// The recording process can also be restarted while it is still running
/// provided that the energy falls below the mEnergyRt-threshold.
/// \param E : energy of the actual package
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::controlRecordingState (double level)
{
    // check for recording end
    // ------------------------------------------------------------------------

    if (mRecording and level < mStopLevel)
    {
        mRecording   = false;
        mRestartable = true;
        MessageHandler::send(Message::MSG_RECORDING_ENDED);
        if (VERBOSE) std::cout << "AudioRecorderAdapter: Recording stopped"
                               << std::endl;
    }

    // check for recording start (only if not in standby mode)
    // ------------------------------------------------------------------------

    if (mStandby != SBR_NONE) {
        // the analyzer is still working, dont do anything here
        return;
    }

    // If the level falls below LEVEL_RETRIGGER, the status of
    // the adapter is declared as restartable:
    if (not mRestartable and mSlidingLevel < LEVEL_RETRIGGER) mRestartable = true;

    // If the adapter is restartable or crosses the trigger recording starts
    if (level>LEVEL_TRIGGER and mRestartable)
    {
        if (VERBOSE)
        {
            if (mRecording) std::cout <<
            "AudioRecorderAdapter: Recording retriggered" << std::endl;
            else std::cout <<
            "AudioRecorderAdapter: Recording started" << std::endl;
        }
        mRecording   = true;
        mRestartable = false;
        mStandby |= SBR_WAITING_FOR_ANALYISIS;
        MessageHandler::send(Message::MSG_RECORDING_STARTED);
    }
}


//-----------------------------------------------------------------------------
//               Cut away silence at the beginning of the buffer
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Cut trailing silence
///
/// The local buffer of the adapter has a length of a few seconds. When the
/// recording starts, the buffer will contain a period of silence before
/// the key was hit. This function removes this part of the vector.
/// \param packet : The packet with the audio PCM data (call by reference)
///////////////////////////////////////////////////////////////////////////////

void AudioRecorderAdapter::cutSilence (PacketType &packet)
{
    // determine the maximum amplitude in the packet
    double maxamplitude = 0;
    for (auto &y : packet) if (fabs(y)>maxamplitude) maxamplitude = fabs(y);
    double trigger = std::min(0.2,maxamplitude*maxamplitude/100);

    int w = getSamplingRate() / 40;           // section width of 0.025 sec
    int sections = packet.size() / w;         // number of sections
    if (sections < 2) return;                 // required: at least two sections
    size_t entries_to_delete = 0;             // number of sections to be deleted
    for (int sec = 0; sec < sections; ++sec)  // scan sectors
    {
        double energy = 0;
        for (int i = sec*w; i < (sec+1)*w; ++i) energy += packet[i] * packet[i];
        if (energy / w < trigger) entries_to_delete += w;
        else break;
    }
    // Remove the first silent sectors:
    EptAssert(entries_to_delete <= packet.size(),"inconsistent numbers of elements");
    if (entries_to_delete > 0)
        packet.erase(packet.begin(), packet.begin() + entries_to_delete);
}
