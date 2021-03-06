/*****************************************************************************
 * Copyright 2018 Haye Hinrichsen, Christoph Wick
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
//     Trivial algorithm for explaining how new algorithms can be built
//=============================================================================

#ifndef EXAMPLEALGORITHM_H
#define EXAMPLEALGORITHM_H

#include "core/calculation/algorithmplugin.h"

// each algorithm has to be in its own namespace to prevent naming conflits
ALGORITHM_H_START(examplealgorithm)

///////////////////////////////////////////////////////////////////////////////
/// \brief Basic class to show how to implement own algorithms.
///
/// This class will calculate an equidistant tuning curve.
///////////////////////////////////////////////////////////////////////////////

class ExampleAlgorithm : public Algorithm
{
public:
    // constructor to copy the original piano
    ExampleAlgorithm(const Piano &piano, const AlgorithmFactoryDescription &description);

protected:
    // the implementation of the worker function (sole required function)
    virtual void algorithmWorkerFunction() override final;

private:
    const double mConcertPitchParam;
};

ALGORITHM_H_END(ExampleAlgorithm)


#endif // EXAMPLEALGORITHM_H
