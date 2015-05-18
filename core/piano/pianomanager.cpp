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
//                              Piano-Manager
//=============================================================================

#include "pianomanager.h"

#include <iostream>

#include "../system/eptexception.h"
#include "../messages/messagehandler.h"
#include "../messages/messagefinalkey.h"
#include "../messages/messagemodechanged.h"
#include "../messages/messagekeyselectionchanged.h"
#include "../messages/messagechangetuningcurve.h"
#include "../messages/messageprojectfile.h"
#include "../messages/messagekeydatachanged.h"
#include "../adapters/modeselectoradapter.h"
#include "../piano/key.h"

//-----------------------------------------------------------------------------
//			                      Constructor
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructor, resets member variables.
///////////////////////////////////////////////////////////////////////////////

PianoManager::PianoManager() :
    mPiano(),
    mSelectedKey(-1),
    mForcedRecording(false),
    mOperationMode(OperationMode::MODE_IDLE)
{}


//-----------------------------------------------------------------------------
//			                       Clear
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Clear all keys and send a message to clear recording.
///////////////////////////////////////////////////////////////////////////////

void PianoManager::clear()
{
    mPiano.clearKeys();
    MessageHandler::send(Message::MSG_CLEAR_RECORDING);
}


//-----------------------------------------------------------------------------
//			            Message listener and dispatcher
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Message listener and dispatcher.
/// \param m : Pointer to the message.
///////////////////////////////////////////////////////////////////////////////

void PianoManager::handleMessage(MessagePtr m)
{
    EptAssert(m, "Message has to exist!");

    switch (m->getType())
    {
    case Message::MSG_PROJECT_FILE:
    {
        auto message(std::static_pointer_cast<MessageProjectFile>(m));
        switch (message->getFileMessageType())
        {
        case MessageProjectFile::FILE_CREATED:
        case MessageProjectFile::FILE_OPENED:
            // on open or create (new file) we select the next nex
            MessageHandler::sendUnique<MessageKeySelectionChanged>(findNextKey(-1), nullptr);
            break;
        default:
            break;
        }
        break;
    }
    case Message::MSG_MODE_CHANGED:
    {
        auto message(std::static_pointer_cast<MessageModeChanged>(m));
        mOperationMode = message->getMode();
        if (mOperationMode==MODE_RECORDING)
        {
            int nextkey = findNextKey(-1);
            MessageHandler::send<MessageKeySelectionChanged>(nextkey, mPiano.getKeyPtr(nextkey));
        }
    }
    break;
    case Message::MSG_KEY_SELECTION_CHANGED:
    {
        auto message(std::static_pointer_cast<MessageKeySelectionChanged>(m));
        mSelectedKey = message->getKeyNumber();
        mForcedRecording = message->isForced();
    }
    break;
    case Message::MSG_FINAL_KEY:
    {
        auto message(std::static_pointer_cast<MessageFinalKey>(m));
        auto keyptr = message->getFinalKey(); // get shared pointer to the new key
        int keynumber = message->getKeyNumber();
        std::cout << "PianoManager: received final key k=" << keynumber << std::endl;
        EptAssert(keynumber >= 0 && keynumber < mPiano.getKeyboard().getNumberOfKeys(),"Range of keynumber");
        handleNewKey(keynumber,keyptr);
    }
    break;
    case Message::MSG_CHANGE_TUNING_CURVE:
    {
        auto message(std::static_pointer_cast<MessageChangeTuningCurve>(m));
        int keynumber = message->getKeyNumber();
        double frequency = message->getFrequency();
//        std::cout << "PianoManager noticed change of tuing curve: k="
//                  << keynumber << ", f=" << frequency << std::endl;
        EptAssert(keynumber >= 0 and keynumber < mPiano.getKeyboard().getNumberOfKeys(), "range of keynumber");
        mPiano.getKey(keynumber).setComputedFrequency(frequency);
        MessageHandler::send<MessageKeyDataChanged>(keynumber, mPiano.getKeyPtr(keynumber));
    }
    break;
    default:
    break;
    }
}

//-----------------------------------------------------------------------------
//              Take action of a new key has been identified
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Handle the event that a key has been successfully indentified.
///
/// This function decides whether to accept or reject a keystroke identified
/// by the FFTAnalyzer and then sets the key in the mPiano.
/// \param keynumber : Number of the key that was recognized.
/// \param keyptr : Shared pointer to the key object.
///////////////////////////////////////////////////////////////////////////////

void PianoManager::handleNewKey (int keynumber, std::shared_ptr<Key> keyptr)
{
    if (mOperationMode == MODE_RECORDING)
    {
        if (keynumber==mSelectedKey or mForcedRecording)
        {
            std::cout << "PianoManager: Sucessfully inserted new key spectrum" << std::endl;
            mPiano.setKey(mSelectedKey,*keyptr);
            // notify, that key data changed (e.g. tuning curve will redraw)
            MessageHandler::send<MessageKeyDataChanged>(mSelectedKey, mPiano.getKeyPtr(mSelectedKey));

#if CONFIG_ENABLE_XMGRACE
            std::ofstream os ("4-quality.dat");
            for (auto &p : mPiano.getKeyboard().getKeys())
                os << p.getRecognitionQuality() << std::endl;
            os.close();
#endif
        }
    }
    else if (mOperationMode == MODE_TUNING and
             (keynumber==mSelectedKey or mForcedRecording))
    {
        double f = keyptr->getTunedFrequency();
        Key* ptr = mPiano.getKeyPtr(mSelectedKey);
        ptr->setTunedFrequency(f);
        MessageHandler::send<MessageKeyDataChanged>(mSelectedKey, ptr);

    }
}


//-----------------------------------------------------------------------------
//                             Find next key
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Find the next key to be recorded.
///
/// Once a key has been recorded successfully, this function determines who
/// ist next.
/// \param keynumber : Number of the current key.
/// \return Number of the next key to be recorded.
///////////////////////////////////////////////////////////////////////////////

int PianoManager::findNextKey (int keynumber) // -1 if none
{
    const int K = mPiano.getKeyboard().getNumberOfKeys();
    if (keynumber < K-1)
    {
        if (mPiano.getKey(keynumber+1).isRecorded() == false) return keynumber+1;
    }
    // Search for incomplete key from the left
    for (int k=0; k<K; ++k) {
        if (mPiano.getKey(k).isRecorded() == false) {
            return k;
        }
    }
    return -1;
}
