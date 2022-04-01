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
 * Sequence.cpp
 *
 *  Created on: Mar 5, 2019
 *      Author: jparziale
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>

#include "Sequence.h"

//*****************************************************************************

Sequence::Sequence() : m_numSeqObjs(0)
{
    memset(m_Sequence, 0, sizeof(m_Sequence));
}

//*****************************************************************************

Sequence::Sequence(tinyxml2::XMLElement *seqTree) : m_numSeqObjs(0)
{
    memset(m_Sequence, 0, sizeof(m_Sequence));

    // The element consists of a single sequence descriptor element.
    (void)LoadSeqDesc(seqTree);
}

//*****************************************************************************

Sequence::~Sequence()
{
    for (unsigned int i = 0; i < m_numSeqObjs; ++i)
    {
        switch (m_Sequence[i].type)
        {
        case eSeqType_Sequence:
        {
            Sequence *pSeq = static_cast<Sequence *>(m_Sequence[i].sequenceObj);
            delete pSeq;
            break;
        }

        case eSeqType_Signal:
        {
            Signal *pSig = static_cast<Signal *>(m_Sequence[i].sequenceObj);
            delete pSig;
            break;
        }

        case eSeqType_DTMF:
        {
            DTMF *pDTMF = static_cast<DTMF *>(m_Sequence[i].sequenceObj);
            delete pDTMF;
            break;
        }

        case eSeqType_Silence:
        {
            int *pDur = static_cast<int *>(m_Sequence[i].sequenceObj);
            delete pDur;
            break;
        }

        default:
            break;
        }
    }
}

//*****************************************************************************

int Sequence::LoadFile(const char *seqFileName)
{
    memset(m_Sequence, 0, sizeof(m_Sequence));

    // Check if file is good. tinyxml2 crashes if the file doesn't exist.
    std::ifstream f(seqFileName);
    if (!f.good())
    {
        return EXIT_FAILURE;
    }

    //*********************************
    // Get document information

    m_doc.LoadFile(seqFileName);
    xmlErrorId = m_doc.ErrorID();
    if (xmlErrorId != tinyxml2::XML_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    //std::cout << "================================================================================" << std::endl;
    //m_doc.Print();
    //std::cout << "================================================================================" << std::endl;
    //std::cout << std::endl;

    tinyxml2::XMLElement *root = m_doc.RootElement();
    std::string rootNodeName = root->Name();

    // Check root node to make sure it's a sequence definition file
    if (rootNodeName.compare("Sequence") == 0)
    {
        // File consists of a sequence of signals, spaces, and/or DTMF tones
        // Element is a sequence description. Load it.
        if (LoadSeqDesc(root) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }

        // Check loaded parameters
        if ((m_SampleRate < eSPS_FIRST) || (m_SampleRate > eSPS_LAST))
        {
            std::cerr << "ERROR: [" << __FUNCTION__ << "] The sequence description file did not specify a valid sample rate." << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] The Sequence class will only handle a sequence description file" << std::endl;
        return EXIT_FAILURE;
    }

    //*********************************

    return EXIT_SUCCESS;
}

//*****************************************************************************

int Sequence::LoadSeqDesc(tinyxml2::XMLElement *rootSeq)
{
    // Get sequence name and description
    const tinyxml2::XMLElement *elem = rootSeq->FirstChildElement("Name");
    if (elem != NULL)
    {
        m_SeqName = elem->GetText();
    }
    else
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] The specified sequence is missing the 'Name' element." << std::endl;
        return EXIT_FAILURE;
    }

    elem = rootSeq->FirstChildElement("Description");
    if (elem != NULL)
    {
        m_SeqDescription = elem->GetText();
    }
    else
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] The specified sequence is missing the 'Description' element." << std::endl;
        return EXIT_FAILURE;
    }

    //*********************************
    // Build sequence description from input XML file data

    // Parse child elements
    tinyxml2::XMLElement *child = rootSeq->FirstChildElement();
    while (child != NULL)
    {
        std::string childName = child->Name();

        if (childName.compare("SampleRate") == 0)
        {
            (void)sscanf(child->GetText(), "%d", (unsigned int *)&m_SampleRate);
        }

        else if (childName.compare("Signal") == 0)
        {
            Signal *pSig = new Signal(child);
            // Override child's possible sample rate specification to match the overall sequence
            if ((m_SampleRate >= eSPS_FIRST) && (m_SampleRate <= eSPS_LAST))
            {
                pSig->SetSampleRate(m_SampleRate);
            }
            else
            {
                // Sequence doesn't have a valid sample rate. Take signal's
                m_SampleRate = pSig->sampleRate();
                std::cerr << "WARNING: [" << __FUNCTION__ << "] The specified sequence is missing a sample rate. Using signal's." << std::endl;
            }
            m_Sequence[m_numSeqObjs].type = eSeqType_Signal;
            m_Sequence[m_numSeqObjs].sequenceObj = (void *)pSig;
            m_numSeqObjs++;
        }

        else if (childName.compare("Silence") == 0)
        {
            int *pDur = new int;
            *pDur = std::atoi(child->GetText());
            m_Sequence[m_numSeqObjs].type = eSeqType_Silence;
            m_Sequence[m_numSeqObjs].sequenceObj = (void *)pDur;
            m_numSeqObjs++;
        }

        else if (childName.compare("DTMF") == 0)
        {
            uint32_t on = DTMF::cDefaultDuration;
            uint32_t off = DTMF::cDefaultDuration;

            // NOTE: If the digits text doesn't appear before OnTime and OffTime, digits will be set to NULL.
            const char *digits = child->GetText();
            if (digits == NULL)
            {
                std::cerr << "ERROR: [" << __FUNCTION__ << "] Couldn't get DTMF text" << std::endl;
                break;
            }

            // Get on and off times
            elem = child->FirstChildElement("OnTime");
            if (elem != NULL)
            {
                on = std::atoi(elem->GetText());
            }

            elem = child->FirstChildElement("OffTime");
            if (elem != NULL)
            {
                off = std::atoi(elem->GetText());
            }

            // Parse DTMF text string and add DTMF digits
            for (unsigned int i = 0; i < strlen(digits); ++i)
            {
                // Silently ignore invalid digits
                if (((digits[i] >= '0') && (digits[i] <= '9')) || (digits[i] == '*') || (digits[i] == '#'))
                {
                    DTMF *pDTMF = new DTMF(digits[i], on, off);
                    m_Sequence[m_numSeqObjs].type = eSeqType_DTMF;
                    m_Sequence[m_numSeqObjs].sequenceObj = (void *)pDTMF;
                    m_numSeqObjs++;
                }
            }
        }

        // Intentionally skip Name and Description child elements (obtained previously)

        child = child->NextSiblingElement();
    } // end while

    return EXIT_SUCCESS;
}

//*****************************************************************************

/*
 * Generate specified number of milliseconds of silence.
 */
int Sequence::GenerateWaveform(std::vector<double> &data)
{
    for (unsigned int i = 0; i < m_numSeqObjs; ++i)
    {
        switch (m_Sequence[i].type)
        {
        case eSeqType_Sequence:
        {
            Sequence *pSeq = static_cast<Sequence *>(m_Sequence[i].sequenceObj);
            std::cout << std::endl;
            std::cout << "[" << std::setw(2) << (i + 1) << "] Generate Sequence:" << std::endl;
            pSeq->printInfo();
            pSeq->GenerateWaveform(data);
            break;
        }

        case eSeqType_Signal:
        {
            Signal *pSig = static_cast<Signal *>(m_Sequence[i].sequenceObj);
            std::cout << std::endl;
            std::cout << "[" << std::setw(2) << (i + 1) << "] Generate Signal:" << std::endl;
            pSig->printInfo();
            pSig->GenerateSignal(data);
            break;
        }

        case eSeqType_DTMF:
        {
            DTMF *pDTMF = static_cast<DTMF *>(m_Sequence[i].sequenceObj);
            std::cout << "[" << std::setw(2) << (i + 1) << "] Generate DTMF '" << pDTMF->name() << "'" << std::endl;
            pDTMF->GenerateData(data, m_SampleRate);
            break;
        }

        case eSeqType_Silence:
        {
            int *pDur = static_cast<int *>(m_Sequence[i].sequenceObj);
            int dur = *pDur;
            std::cout << std::endl;
            std::cout << "[" << std::setw(2) << (i + 1) << "] Generate Silence: " << dur << " mS" << std::endl;
            GenerateSilence(data, dur);
            break;
        }

        default:
            break;
        }
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}

//*****************************************************************************

int Sequence::GenerateSilence(std::vector<double> &data, unsigned int ms)
{
    // First check if the description info is valid
    bool isValid = true;

    if ((m_SampleRate < eSPS_FIRST) || (m_SampleRate > eSPS_LAST))
    {
        std::cerr << "ERROR: Bad sample rate; " << m_SampleRate << std::endl;
        isValid = false;
    }
    if (!isValid)
    {
        return EXIT_FAILURE;
    }

    // Generate silence data
    unsigned int numSamples = (ms * m_SampleRate) / 1000;

    // Sample loop
    for (unsigned int i = 0; i < numSamples; i++)
    {
        data.push_back(0.0);
    }

    return EXIT_SUCCESS;
}

//*****************************************************************************

void Sequence::printInfo()
{
    std::cout << "Sequence: " << m_SeqName << std::endl;
    std::cout << "  Description: " << m_SeqDescription << std::endl;
    std::cout << "  Sample Rate: " << m_SampleRate << std::endl;
    std::cout << "  Segments   : " << m_numSeqObjs << std::endl;
}

//*****************************************************************************
