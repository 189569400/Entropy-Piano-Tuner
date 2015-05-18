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

//======================================================================
//                           Midi adapter
//======================================================================

#include "midiadapter.h"

#include <iostream>
#include <sstream>

#include "../system/prerequisites.h"
#include "../messages/messagehandler.h"
#include "../messages/messagemidievent.h"


//-------------------------send MIDI message ---------------------------

/// This function sends a new data set received from the MIDI implementation
/// as a message to the message handler.

void MidiAdapter::send (Data &data)
{
    std::cout << "MidiAdapter::MidiEvent: " << (int)(data.event) << " " << data.byte1 << " " << data.byte2 << "\t" << data.deltatime << std::endl;
    MessageHandler::send<MessageMidiEvent>(data);
}
