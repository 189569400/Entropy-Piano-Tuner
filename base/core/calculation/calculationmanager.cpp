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

//=============================================================================
//                          Calculation manager
//=============================================================================

#include "calculationmanager.h"

#include <iostream>
#include <cmath>
#include <dirent.h>

#include "../config.h"
#include "../system/log.h"
#include "../system/timer.h"
#include "../messages/messagehandler.h"
#include "../messages/messagecaluclationprogress.h"
#include "../messages/messagechangetuningcurve.h"
#include "algorithmfactory.h"
#include "algorithminformationparser.h"

//-----------------------------------------------------------------------------
//                               Constructor
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructor, resets member variables
///////////////////////////////////////////////////////////////////////////////

CalculationManager::CalculationManager()
{
    loadAlgorithms("algorithms");
}

CalculationManager::~CalculationManager()
{
    stop();
}


CalculationManager &CalculationManager::getSingleton()
{
    static CalculationManager THE_ONE_AND_ONLY;
    return THE_ONE_AND_ONLY;
}


void CalculationManager::loadAlgorithms(const std::string &algorithmsDir)
{
    auto has_suffix= [](const std::string &str, const std::string &suffix)
    {
        return str.size() >= suffix.size() &&
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    DIR *dir = opendir(algorithmsDir.c_str());
    struct dirent *ent;
    if (!dir) {
        LogW("Algorithm plugins directory could not be opened.");
        return;
    }
    while ((ent = readdir(dir)) != nullptr) {
        const std::string filename = ent->d_name;
        if (has_suffix(filename, ".so") || has_suffix(filename, ".dll") || has_suffix(filename, ".dylib")) {
          LogI("Reading algorithm %s", ent->d_name);


        }
    }

    closedir(dir);
}

//-----------------------------------------------------------------------------
//                           Start calculation
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Start the calculation thread. By calling this function, the current
/// piano is passed by reference and copied to the local mPiano member variable.
/// \param piano : Reference to the piano instance.
///////////////////////////////////////////////////////////////////////////////

void CalculationManager::start(const Piano &piano)
{
    // stop the old algorithm to be sure
    stop();

    // create and start new algorithm
    mCurrentAlgorithm = std::move(mAlgorithms[getCurrentAlgorithmInformation()->getId()]->createAlgorithm(piano));
    mCurrentAlgorithm->start();
}


//-----------------------------------------------------------------------------
//                             Stop calculation
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Stop the calculation thread.
///////////////////////////////////////////////////////////////////////////////

void CalculationManager::stop()
{
    if (mCurrentAlgorithm) {
        mCurrentAlgorithm->stop();
        mCurrentAlgorithm.reset();
    }
}

void CalculationManager::registerFactory(const std::string &name, AlgorithmFactoryBase* factory)
{
    if (mAlgorithms.count(name) == 1) {
        EPT_EXCEPT(EptException::ERR_DUPLICATE_ITEM, "An algorithm with name '" + name + "' already exists.");
    }

#if EPT_EXCLUDE_EXAMPLE_ALGORITHM
    // exclude example algorithm(s)
    if (name.find("example") != std::string::npos) {
        return;
    }
#endif

    mAlgorithms[name] = factory;
}

std::shared_ptr<const AlgorithmInformation> CalculationManager::loadAlgorithmInformation(const std::string &algorithmName) const
{
    // open the xml file for this algorithm and return the information in the current language
    AlgorithmInformationParser parser;
    return parser.parse(algorithmName);
}

bool CalculationManager::hasAlgorithm(const std::string &id) const {
    return mAlgorithms.count(id) == 1;
}

std::string CalculationManager::getDefaultAlgorithmId() const {
    EptAssert(hasAlgorithm("entropyminimizer"), "Default algorithm does not exits.");
    return "entropyminimizer";
}

const std::shared_ptr<const AlgorithmInformation> CalculationManager::getCurrentAlgorithmInformation() {
    if (!mCurrentAlgorithmInformation) {
        // load default
        LogI("Loading default algorithm information");
        mCurrentAlgorithmInformation = loadAlgorithmInformation(getDefaultAlgorithmId());
    }
    return mCurrentAlgorithmInformation;
}
