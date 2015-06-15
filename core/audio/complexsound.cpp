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
//                        Class for a complex sound
//=============================================================================

#include "complexsound.h"

#include "../math/mathtools.h"
#include "../system/log.h"


//-----------------------------------------------------------------------------
//                               Constructor
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructor, initializes the member variables
///////////////////////////////////////////////////////////////////////////////

ComplexSound::ComplexSound() :
    mSampleRate(0),
    mSineWave(),
    mSpectrum(),
    mStereo(0.5),
    mSampleLength(0),
    mWaveForm(),
    mReady(false),
    mHash(0)
{}


//-----------------------------------------------------------------------------
//                         Initialize a new ComplexSound
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Initialize a new complex sound
///
/// This function initializes a newly created complex sound, depending on the
/// parameters which characterize the sound. After initialization, the
/// computation of the PCM wave form is started in an independent thread.
///
/// \param spectrum : The power spectrum of the sound (frequency - power)
/// \param stereo : Stereo location in the range 0 (left) to 1 (right)
/// \param samplerate : Sample rate to be used for generating PCM
/// \param sinewave : Reference to a pre-calculated sine wave
/// \param time : Duration of the PCM waveform to be generated in seconds
/// \param waitingtime : Technical sleep time before compuatation (seconds).
///////////////////////////////////////////////////////////////////////////////

void ComplexSound::init (const double frequency,
                         const Spectrum &spectrum,
                         const double stereo,
                         const int samplerate,
                         const WaveForm &sinewave,
                         const double playingtime,
                         const double waitingtime)
{
    if (frequency<=0) return;
    std::lock_guard<std::mutex> lock(mMutex);
    mSampleRate = samplerate;
    mSineWave = sinewave;
    mSpectrum.clear();
    // The spectrum is shifted in such a way that the fundamental
    // frequency f1 is scaled down to the parameter 'frequency'.
    if (spectrum.size()>0)
    {
        double f1 = spectrum.begin()->first;
        for (auto &s : spectrum) mSpectrum[s.first*frequency/f1]=s.second;
    }
    mStereo = stereo;
    mWaveForm.clear();
    mSampleLength = MathTools::roundToInteger(mSampleRate * playingtime);
    mReady = false;
    mHash = computeHashTag(frequency,spectrum);
    mWaitingTime = waitingtime;
    start();                        // start the thread (workerFunction)
}


//-----------------------------------------------------------------------------
//                Generate the PCM waveform of a complex sound
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Generate the PCM waveform of a complex sound.
///
/// This function generates the PCM waveform according to the specified power
/// spectrum for the predetermined length. By default a stereo singnal is
/// generated (format LRLR...).
///
/// The wave consists of a superposition of sine functions. In order to avoid
/// an initial "klick" all sine waves are randomly phase-shifted. Moreover,
/// the phase between the two stereo channels is phase-shifted to mimic the
/// phase delay in the real world. This leads to a more stereo-like sound.
///////////////////////////////////////////////////////////////////////////////

void ComplexSound::generateWaveform()
{
    std::lock_guard<std::mutex> lock(mMutex);
    const double leftvol  = sqrt(0.9-0.8*mStereo);
    const double rightvol = sqrt(0.1+0.8*mStereo);
    mWaveForm.resize(2*mSampleLength);
    mWaveForm.assign(2*mSampleLength,0);
    std::default_random_engine generator;
    std::uniform_int_distribution<int> flatDistribution(0,mSampleLength);


    double norm = 0;
    for (auto &s : mSpectrum) norm += s.second;
    if (norm<=0) return;

    const int64_t SineLength = mSineWave.size();
    for (auto &mode : mSpectrum)
    {
        const double f = mode.first;
        const double volume = sqrt(mode.second/norm);

        if (f>24 and f<10000 and volume>0.001)
        {
            const int64_t periods = round((mSampleLength * f) / mSampleRate);
            const int64_t phasediff = round(periods * mSampleRate *
                                            (0.5-mStereo) / 500);
            const int64_t leftphase  = flatDistribution(generator);;
            const int64_t rightphase = leftphase + phasediff;
            for (int64_t i=0; i<mSampleLength; ++i)
            {
                mWaveForm[2*i] += volume * leftvol *
                    mSineWave[((i*periods*SineLength)/mSampleLength+leftphase)%SineLength];
                mWaveForm[2*i+1] += volume * rightvol *
                    mSineWave[((i*periods*SineLength)/mSampleLength+rightphase)%SineLength];
            }
        }
        if (cancelThread()) return;
    }
    mReady = true;
    LogI ("Created waveform with %d partials", (int)(mSpectrum.size()));

//    if (id !=0) return;
//    std::ofstream os("0000-waveform.dat");
//    for (int64_t i=0; i<SampleLength; ++i)  os << mWaveForms[0][2*i] << std::endl;
//    os << "&" << std::endl;
//    for (int64_t i=0; i<SampleLength; ++i)  os << mWaveForms[0][2*i+1] << std::endl;
//    os.close();
}


//-----------------------------------------------------------------------------
//           Worker function executing the thread for PCM generation
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief ComplexSound::workerFunction
///
/// This function executes the thread which is started in the initialization
/// procedure.
///
/// The function does not compute the new waveform immediately, instead it
/// waits for mWaitingTime (double in seconds) before the computation is
/// started. Whenever the the same thread is started once again during the
/// waiting period, the current thread is cancelled. This ensures that only
/// the newest thread survives. On the other hand, a high CPU load in
/// situations with many changes (during tuning) is avoided.
///
/// To further avoid an overload of the CPU, only a certain maximal number
/// of threads is allows to be active at the same time. The worker function
/// manages this thread limitation and calls the function generateWafeform().
///////////////////////////////////////////////////////////////////////////////

std::atomic<int> ComplexSound::numberOfThreads(0); // static thread counter

void ComplexSound::workerFunction()
{
    // First wait and cancel if retriggered
    for (double t=0; t<1000.0*mWaitingTime and not cancelThread(); t+=0.001) msleep (1);
    if (cancelThread()) return;

    // Avoid CPU overload by limiting the number of threads
    const int maxNumberOfThreads = 8;
    while (numberOfThreads >= maxNumberOfThreads) msleep(1);
    if (cancelThread()) return;
    numberOfThreads++;
    generateWaveform();
    numberOfThreads--;
}


//-----------------------------------------------------------------------------
//                      Private: compute hash value
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Compute hash value of the spectrum
///
/// As soon as the frequency of a key changes, the waveform of the ComplexSound
/// has to be computed once again. However, if the spectrum happens to
/// coincide with the actual spectrum, then there is no need for a renewed
/// computation. Since the frequency of the spectral lines may change during
/// tuning, the hash tag is computed exclusively from the fundamental
/// frequency and the peaks heights.
///
/// Since during computation a comparison of the spectrum would be difficult
/// because of mutex blocking, the class saves a hash tag of the spectrum as
/// a necessary condition for coincidence.
///
/// \param frequency : Fundamental frequency in Hz
/// \param spectrum : Reference to the spectrum.
/// \return Corresponding hash tag.
///////////////////////////////////////////////////////////////////////////////

long ComplexSound::computeHashTag (const double frequency, const Spectrum &spectrum)
{
    long hash = std::hash<double>() (frequency);
    for (auto &f : spectrum) hash ^= std::hash<double>() (f.second);
    return hash;
}


//----------------------------------------------------------------------------
//     Return the PCM waveform of the complex sound (constant volume)
//----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Get the current PCM waveform of the complex sound
/// \return PCM vector in stereo (LRLRLR...), zero size if not yet computed
///////////////////////////////////////////////////////////////////////////////

ComplexSound::WaveForm ComplexSound::getWaveForm()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (not mReady) return WaveForm();
    else return mWaveForm;
}

