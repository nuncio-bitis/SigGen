/*
 * This file is part of the DataGatheringSystem distribution
 *   (https://github.com/nuncio-bitis/SigGen
 * Copyright (c) 2022 James P. Parziale.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Signal.h
 *
 *  Created on: Mar 2, 2019
 *      Author: jparziale
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_
//*****************************************************************************

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <math.h>
#include <fftw3.h>
#include <tinyxml2.h>

//*****************************************************************************

// Maximum number of sinusoids to add together for one signal
#define MAX_SIG 64

#ifndef M_PI
#define M_PI (2.0 * asin(1.0))
#endif

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define SWAP(a, b) \
    tempr = (a);   \
    (a) = (b);     \
    (b) = tempr

enum SampleRate
{
    eSPS_7200  = 7200,
    eSPS_8000  = 8000,
    eSPS_9600  = 9600,
    eSPS_10000 = 10000,
    eSPS_11050 = 11050,
    eSPS_22100 = 22100,
    eDefaultSampleRate = eSPS_9600,
    eSPS_FIRST = eSPS_7200,
    eSPS_LAST  = eSPS_22100,
    eSPS_INVALID = 0
};

//*****************************************************************************

class Signal
{
public:
    Signal();
    Signal(tinyxml2::XMLElement *sigTree);
    virtual ~Signal();

    int LoadFile(const char *sigFileName);

    std::string name() { return m_SigName; };
    std::string description() { return m_Description; };
    SampleRate sampleRate() { return m_SampleRate; };
    unsigned int numSamples() { return m_numSamples; };
    unsigned int numTones() { return m_numTones; };

    int SetSampleRate(SampleRate Sr);

    int GenerateSignal(std::vector<double> &data);
    int GenerateSilence(std::vector<double> &data, unsigned int ms);

    void printInfo();

private:
    struct SigDesc
    {
        double amp;
        double freq;
        double phase;
        std::vector<unsigned int> harmonics;
        std::vector<double> harmonicAmps;
    };

    int LoadSeqDesc(tinyxml2::XMLElement *rootSig);
    int LoadSigDesc(tinyxml2::XMLElement *rootSig);

    tinyxml2::XMLDocument m_doc;
    int xmlErrorId = 0;

    std::string m_SigName;
    std::string m_Description;
    SampleRate m_SampleRate = eSPS_INVALID;
    unsigned int m_numSamples = 0;
    unsigned int m_duration = 0;

    SigDesc m_Signal[MAX_SIG];
    unsigned int m_numTones = 0;

    // Use fixed 20% rise and fall times as default
    const double cRiseFallFactorDefault = 0.0;
    const double cRiseFallFactorMin = 0.0;
    const double cRiseFallFactorMax = 0.8;

    double m_maxAmp = 0.0;
    double m_RiseFactor;
    double m_FallFactor;
};

//*****************************************************************************
#endif /* SIGNAL_H_ */
