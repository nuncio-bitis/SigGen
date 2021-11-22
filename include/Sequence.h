/*
 * Sequence.h
 *
 *  Created on: Mar 5, 2019
 *      Author: jparziale
 */

#ifndef SEQUENCE_H_
#define SEQUENCE_H_
//*****************************************************************************

#include <cstdlib>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <math.h>
#include <fftw3.h>
#include <tinyxml2.h>

#include "Signal.h"
#include "DTMF.h"

//*****************************************************************************

enum SequenceType {
    eSeqType_Sequence,
    eSeqType_Signal,
    eSeqType_DTMF,
    eSeqType_Silence
};


//*****************************************************************************

class Sequence
{
public:

    // Maximum number of elements a sequence can have (signals, silences, and DTMFs)
    static const unsigned int cMaxSequence = 64;

    Sequence();
    Sequence(tinyxml2::XMLElement *seqTree);
    virtual ~Sequence();

    int LoadFile(const char *seqFileName);

    std::string name() { return m_SeqName; };
    std::string description() { return m_SeqDescription; };
    SampleRate sampleRate() { return m_SampleRate; };
    unsigned int numSegments() { return m_numSeqObjs; };

    int GenerateWaveform(std::vector<double> &data);
    int GenerateSilence(std::vector<double> &data, unsigned int ms);

    void printInfo();

private:
    struct SeqDesc {
        SequenceType type; // Specifies how to treat sequenceObj
        void *sequenceObj;
    };

    int LoadSeqDesc(tinyxml2::XMLElement *rootSig);

    int xmlErrorId = 0;

    std::string m_SeqName;
    std::string m_SeqDescription;
    SampleRate m_SampleRate = eSPS_INVALID;

    SeqDesc m_Sequence[cMaxSequence];
    unsigned int m_numSeqObjs = 0;

    tinyxml2::XMLDocument m_doc;
};

//*****************************************************************************
#endif /* SEQUENCE_H_ */
