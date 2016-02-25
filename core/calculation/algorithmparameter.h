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

#ifndef ALGORITHMPARAMETER_H
#define ALGORITHMPARAMETER_H

#include <string>
#include <vector>
#include <limits>

class AlgorithmParameter
{
public:
    using StringParameterList = std::vector<std::pair<std::string, std::string> >;
public:
    enum Type {
        TYPE_UNSET,
        TYPE_DOUBLE,
        TYPE_INT,
        TYPE_LIST,
    };

public:
    const std::string &getID() const {return mID;}
    std::string &getID() {return mID;}

    const std::string &getLabel() const {return mLabel;}
    std::string &getLabel() {return mLabel;}

    const std::string &getDescription() const {return mDescription;}
    std::string &getDescription() {return mDescription;}

    Type getType() const {return mParameterType;}
    void setType(Type t) {mParameterType = t;}

    // double parameters
    // ----------------------------------------------------------------------------
    double getDoubleDefaultValue() const {return mDoubleDefaultValue;}
    double &getDoubleDefaultValue() {return mDoubleDefaultValue;}

    double getDoubleMinValue() const {return mDoubleMinValue;}
    double &getDoubleMinValue() {return mDoubleMinValue;}

    double getDoubleMaxValue() const {return mDoubleMaxValue;}
    double &getDoubleMaxValue() {return mDoubleMaxValue;}

    int    getDoublePrecision() const {return mDoublePrecision;}
    int    &getDoublePrecision() {return mDoublePrecision;}

    // int parameters
    // ----------------------------------------------------------------------------
    int getIntDefaultValue() const {return mIntDefaultValue;}
    int &getIntDefaultValue() {return mIntDefaultValue;}

    int getIntMinValue() const {return mIntMinValue;}
    int &getIntMinValue() {return mIntMinValue;}

    int getIntMaxValue() const {return mIntMaxValue;}
    int &getIntMaxValue() {return mIntMaxValue;}

    // string parameters
    // ----------------------------------------------------------------------------
    const std::string &getStringDefaultValue() const {return mStringDefaultValue;}
    std::string &getStringDefaultValue() {return mStringDefaultValue;}

    const StringParameterList &getStringList() const {return mStringList;}
    StringParameterList &getStringList() {return mStringList;}

    // global parameters
    // ----------------------------------------------------------------------------
    bool displayLineEdit() const {return mDisplayLineEdit;}
    bool &displayLineEdit() {return mDisplayLineEdit;}

    bool displaySpinBox() const {return mDisplaySpinBox;}
    bool &displaySpinBox() {return mDisplaySpinBox;}

    bool displaySlider() const {return mDisplaySlider;}
    bool &displaySlider() {return mDisplaySlider;}

    bool displaySetDefaultButton() const {return mDisplaySetDefaultButton;}
    bool &displaySetDefaultButton() {return mDisplaySetDefaultButton;}

    bool readOnly() const {return mReadOnly;}
    bool &readOnly() {return mReadOnly;}

    double updateInterval() const {return mUpdateInterval;}
    double &updateInterval() {return mUpdateInterval;}
    void setUpdateInterval(double i) {mUpdateInterval = i;}

private:
    Type mParameterType = TYPE_UNSET;
    std::string mID;

    std::string mLabel;
    std::string mDescription;

    double mDoubleDefaultValue = 0;
    double mDoubleMinValue = std::numeric_limits<double>::min();
    double mDoubleMaxValue = std::numeric_limits<double>::max();
    int    mDoublePrecision = 0;

    int    mIntDefaultValue = 0;
    int    mIntMinValue = std::numeric_limits<int>::min();
    int    mIntMaxValue = std::numeric_limits<int>::max();

    std::string mStringDefaultValue;
    StringParameterList mStringList;

    bool   mDisplayLineEdit = false;
    bool   mDisplaySpinBox  = true;
    bool   mDisplaySlider   = true;
    bool   mDisplaySetDefaultButton = true;

    bool   mReadOnly        = false;

    double mUpdateInterval = -1;
};

#endif // ALGORITHMPARAMETER_H
