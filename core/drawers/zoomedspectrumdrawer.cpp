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
//                          Draw zoomed spectrum
//=============================================================================

#include "zoomedspectrumdrawer.h"

#include <cstdint>
#include <iostream>
#include <algorithm>

#include "../messages/messagenewfftcalculated.h"
#include "../messages/messagekeyselectionchanged.h"
#include "../messages/messagepreliminarykey.h"
#include "../messages/messagemodechanged.h"
#include "../messages/messageprojectfile.h"
#include "../messages/messagetuningdeviation.h"
#include "../math/mathtools.h"
#include "../math/mathtools.h"
#include "../system/log.h"
#include "../piano/piano.h"

ZoomedSpectrumDrawer::ZoomedSpectrumDrawer(GraphicsViewAdapter *graphics) :
    DrawerBase(graphics),
    mPiano(nullptr),
    mNumberOfKeys(0),
    mSelectedKey(-1),
    mRecognizedKey(-1),
    mOperationMode(MODE_COUNT)
{
}

//-----------------------------------------------------------------------------
//                            Message listener
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Message listener
/// \param m : Pointer to the incoming message
///////////////////////////////////////////////////////////////////////////////

void ZoomedSpectrumDrawer::handleMessage(MessagePtr m)
{
    switch (m->getType())
    {
    case Message::MSG_MODE_CHANGED:
    {
        auto mmc(std::static_pointer_cast<MessageModeChanged>(m));
        mOperationMode = mmc->getMode();
        redraw(true);
        break;
    }
    case Message::MSG_PRELIMINARY_KEY:
    {
        if (mOperationMode==MODE_TUNING)
        {
            auto message(std::static_pointer_cast<MessagePreliminaryKey>(m));
            mRecognizedKey = message->getKeyNumber();
            redraw(true);
        }
        else mRecognizedKey=-1;
        break;
    }
    case Message::MSG_KEY_SELECTION_CHANGED:
    {
        auto mksc(std::static_pointer_cast<MessageKeySelectionChanged>(m));
        mSelectedKey = mksc->getKeyNumber();
        redraw(true);
        break;
    }
    case Message::MSG_PROJECT_FILE:
    {
        auto mpf(std::static_pointer_cast<MessageProjectFile>(m));
        mPiano = &mpf->getPiano();
        mNumberOfKeys = mPiano->getKeyboard().getNumberOfKeys();
        mSelectedKey = std::min<int>(mSelectedKey, mNumberOfKeys);
        mGraphics->clear();
        break;
    }
    case Message::MSG_TUNING_DEVIATION: {
        auto mtd(std::static_pointer_cast<MessageTuningDeviation>(m));
        mFrequencyDetectionResult = mtd->getResult();
        redraw();
        break;
    }
    default:
        break;
    }
}

//-----------------------------------------------------------------------------
//                                  Reset
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Reset: Clear the shared pointer to the FFT
///////////////////////////////////////////////////////////////////////////////

void ZoomedSpectrumDrawer::reset()
{
    mFFTData.reset();
    DrawerBase::reset();
}


//-----------------------------------------------------------------------------
//                             Drawing function
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Main drawing function
///////////////////////////////////////////////////////////////////////////////


void ZoomedSpectrumDrawer::draw()
{
    //------- Draw a horizontal and a vertical line separating the field ------


    mGraphics->drawLine(0.5, 0, 0.5, 0.8, GraphicsViewAdapter::PEN_MEDIUM_DARK_GREEN);
    mGraphics->drawLine(0,   0.8, 1, 0.8, GraphicsViewAdapter::PEN_THIN_DARK_GRAY);

    int keynumber = (mSelectedKey >= 0 ? mSelectedKey : mRecognizedKey);
    if (keynumber<0) return;

    if (not mFrequencyDetectionResult) {return;} // we need data
    if (not mPiano) {return;}           // and a piano

    // show only a quarter of the actual deviation curve
    const int specWindowSize = mFrequencyDetectionResult->tuningDeviationCurve.size() / 4;

    for (int i=1; i<specWindowSize; i++)
    {
        double x = (1.0*i)/specWindowSize;
        double y = ((i-specWindowSize/2)%10==0 ? 1 : 0.85);
        mGraphics->drawLine(x, 0.8, x, y, GraphicsViewAdapter::PEN_THIN_LIGHT_GRAY);
    }

    //---------------------- Draw tuning deviation curve -----------------

    if (specWindowSize > 0) {
        double max = *std::max_element(mFrequencyDetectionResult->tuningDeviationCurve.begin(),
                                       mFrequencyDetectionResult->tuningDeviationCurve.end());

        std::vector<GraphicsViewAdapter::Point> points;
        const int centerIndex = mFrequencyDetectionResult->tuningDeviationCurve.size() / 2;
        const int startIndex = std::max<int>(0, centerIndex - specWindowSize / 2 +
                               MathTools::roundToInteger(mFrequencyDetectionResult->positionOfMaximum) -
                               mFrequencyDetectionResult->deviationInCents);
        const int endIndex = std::min<int>(startIndex + specWindowSize,
                                           mFrequencyDetectionResult->tuningDeviationCurve.size());
        for (int i = startIndex; i < endIndex; ++i) {
            double y = mFrequencyDetectionResult->tuningDeviationCurve[i] / max;
            y *= y;
            y = 0.8 - 0.77 * y;
            double x = static_cast<double>(i - startIndex) / specWindowSize;
            points.push_back({x, y});
        }
        mGraphics->drawChart(points, GraphicsViewAdapter::PEN_THIN_RED);
    }

    //---------------------- Draw overpull marker ------------------------

    double concertpitch = mPiano->getConcertPitch();
    piano::PianoType pt =  mPiano->getPianoType();
    double overpull = mPiano->getKeyboard().computeOverpull(keynumber,concertpitch,pt);

    if (abs(overpull)>5 and abs(overpull<100))
    {
        auto overpullColor = GraphicsViewAdapter::PEN_THIN_BLUE;
        if (abs(overpull) > specWindowSize/2)
        {
            overpullColor = GraphicsViewAdapter::PEN_MEDIUM_RED;
            overpull = specWindowSize/2 * (overpull>0 ? 1:-1);
        }
        double x = 0.5 + overpull / specWindowSize;
        mGraphics->drawLine(x,0,x,0.8,overpullColor);
    }



    //----------------------- Draw tuning marker -------------------------

    double markerWidth = 0.1;
    double markerHeight = 0.1;
    double deviation = mFrequencyDetectionResult->deviationInCents;
    double mx = 0.5 - markerWidth / 2 + deviation / specWindowSize;
    double my = 0.9 - markerHeight / 2;


    auto borderline = GraphicsViewAdapter::PEN_THIN_DARK_GRAY;
    auto filling = GraphicsViewAdapter::FILL_RED;
    if (std::abs(deviation) < 5) filling = GraphicsViewAdapter::FILL_GREEN;
    else if (std::abs(deviation) < 10) filling = GraphicsViewAdapter::FILL_ORANGE;

    mGraphics->drawFilledRect(mx, my, markerWidth, markerHeight, borderline, filling);
}

