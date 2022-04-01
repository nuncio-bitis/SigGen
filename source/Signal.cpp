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
 * Signal.cpp
 *
 *  Created on: Mar 2, 2019
 *      Author: jparziale
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sstream>
#include <iostream>

#include "Signal.h"

//*****************************************************************************

Signal::Signal()
    : m_numSamples(0), m_duration(0),
      m_RiseFactor(cRiseFallFactorDefault),
      m_FallFactor(cRiseFallFactorDefault)
{
    memset(m_Signal, 0, sizeof(m_Signal));
}

//*****************************************************************************

Signal::Signal(tinyxml2::XMLElement *sigTree)
    : m_numSamples(0), m_duration(0),
      m_RiseFactor(cRiseFallFactorDefault),
      m_FallFactor(cRiseFallFactorDefault)
{
    memset(m_Signal, 0, sizeof(m_Signal));

    // The element consists of a single signal descriptor element.
    // Element is a signal description. Load it.
    (void)LoadSigDesc(sigTree);
}

//*****************************************************************************

Signal::~Signal()
{
}

//*****************************************************************************

int Signal::LoadFile(const char *sigFileName)
{
    memset(m_Signal, 0, sizeof(m_Signal));

    // Check if file is good. tinyxml2 crashes if the file doesn't exist.
    std::ifstream f(sigFileName);
    if (!f.good())
    {
        return EXIT_FAILURE;
    }

    //*********************************
    // Get document information

    tinyxml2::XMLDocument doc;
    m_doc.LoadFile(sigFileName);
    xmlErrorId = doc.ErrorID();
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

    // Check root node to make sure it's a signal definition file
    if (rootNodeName.compare("Signal") == 0)
    {
        // The file consists of a single signal descriptor element.
        // Element is a signal description. Load it.
        if (LoadSigDesc(root) != EXIT_SUCCESS)
        {
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] The Signal class will only handle a signal description file" << std::endl;
        return EXIT_FAILURE;
    }

    //*********************************

    return EXIT_SUCCESS;
}

//*****************************************************************************

int Signal::LoadSigDesc(tinyxml2::XMLElement *rootSig)
{
    // Get signal name and description
    const tinyxml2::XMLElement *elem = rootSig->FirstChildElement("Name");
    if (elem != NULL)
    {
        m_SigName = elem->GetText();
    }
    else
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] The specified signal is missing the 'Name' element." << std::endl;
        return EXIT_FAILURE;
    }

    elem = rootSig->FirstChildElement("Description");
    if (elem != NULL)
    {
        m_Description = elem->GetText();
    }
    else
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] The specified signal is missing the 'Description' element." << std::endl;
        return EXIT_FAILURE;
    }

    //*********************************
    // Build signal description from input XML file data

    m_numTones = 0;

    tinyxml2::XMLElement *child = rootSig->FirstChildElement();
    while (child != NULL)
    {
        std::string childName = child->Name();

        if (childName.compare("SampleRate") == 0)
        {
            m_SampleRate = (SampleRate)std::stoi(child->GetText());
        }

        else if (childName.compare("Duration") == 0)
        {
            m_duration = std::stoi(child->GetText());
            // Only allow one to be specified: number of samples or milliseconds
            m_numSamples = 0;
        }

        else if (childName.compare("Samples") == 0)
        {
            (void)sscanf(child->GetText(), "%d", &m_numSamples);
            // Only allow one to be specified: number of samples or milliseconds
            m_duration = 0;
        }

        else if (childName.compare("RiseFactor") == 0)
        {
            // Expressed in percent (e.g. 20.5 for 20.5% => factor of 0.205)
            m_RiseFactor = std::stod(child->GetText()) / 100.0;
            if (((m_RiseFactor - cRiseFallFactorMin) < 0) ||
                ((cRiseFallFactorMax - m_RiseFactor) < 0))
            {
                std::cerr << "ERROR: [" << __FUNCTION__ << "] Bad rise/fall factor specified: "
                          << (m_RiseFactor * 100.0) << "%" << std::endl;
                m_RiseFactor = cRiseFallFactorDefault;
            }
        }

        else if (childName.compare("FallFactor") == 0)
        {
            // Expressed in percent (e.g. 20.5 for 20.5% => factor of 0.205)
            m_FallFactor = std::stod(child->GetText()) / 100.0;
            if (((m_FallFactor - cRiseFallFactorMin) < 0) ||
                ((cRiseFallFactorMax - m_FallFactor) < 0))
            {
                std::cerr << "ERROR: [" << __FUNCTION__ << "] Bad rise/fall factor specified: "
                          << (m_FallFactor * 100.0) << "%" << std::endl;
                m_FallFactor = cRiseFallFactorDefault;
            }
        }

        else if (childName.compare("Tone") == 0)
        {
            const tinyxml2::XMLElement *attrAmp = child->FirstChildElement("amp");
            const tinyxml2::XMLElement *attrFreq = child->FirstChildElement("freq");
            const tinyxml2::XMLElement *attrPhase = child->FirstChildElement("phase");
            const tinyxml2::XMLElement *attrHarmonics = child->FirstChildElement("harmonics");
            const tinyxml2::XMLElement *attrHarmonicAmps = child->FirstChildElement("harmonicAmps");

            m_Signal[m_numTones].freq = std::stod(attrFreq->GetText());
            m_Signal[m_numTones].phase = std::stod(attrPhase->GetText());
            m_Signal[m_numTones].amp = std::stod(attrAmp->GetText());

            // Extract list of harmonics (if any)
            if (attrHarmonics != NULL)
            {
                std::stringstream ss(attrHarmonics->GetText());
                while (ss.good())
                {
                    std::string substr;
                    std::getline(ss, substr, ',');
                    m_Signal[m_numTones].harmonics.push_back(std::stoi(substr));
                }
            }
            // Extract list of harmonics' amplitudes (if any)
            if (attrHarmonicAmps != NULL)
            {
                std::stringstream ss(attrHarmonicAmps->GetText());
                while (ss.good())
                {
                    std::string substr;
                    std::getline(ss, substr, ',');
                    m_Signal[m_numTones].harmonicAmps.push_back(std::stod(substr));
                }
            }
            // Error if different number of each
            if (m_Signal[m_numTones].harmonics.size() != m_Signal[m_numTones].harmonicAmps.size())
            {
                std::cerr << "[" << __FUNCTION__ << "] ERROR: "
                          << "Number of harmonics(" << m_Signal[m_numTones].harmonics.size()
                          << ") doesn't match number of amplitudes(" << m_Signal[m_numTones].harmonicAmps.size()
                          << ")" << std::endl;
                std::cerr << "\t    Signal: " << m_SigName << ", Freq: " << m_Signal[m_numTones].freq << std::endl;
                std::cerr << std::endl;
                m_Signal[m_numTones].harmonics.clear();
                m_Signal[m_numTones].harmonicAmps.clear();
            }

            m_numTones++;
        }
        // Silently ignore unrecognized XML elements.

        child = child->NextSiblingElement();
    }

    // Get max amplitude of combined signals, to keep output within [-1.0..1.0]
    for (unsigned int j = 0; j < m_numTones; j++)
    {
        m_maxAmp += m_Signal[j].amp;
        for (unsigned int k = 0; k < m_Signal[j].harmonicAmps.size(); k++)
        {
            m_maxAmp += m_Signal[j].harmonicAmps[k];
        }
    }

    return EXIT_SUCCESS;
}

//*****************************************************************************

/*
 * Purposely override the sample rate read from the descriptor file.
 */
int Signal::SetSampleRate(SampleRate Sr)
{
    m_SampleRate = Sr;

    if ((m_SampleRate < eSPS_FIRST) || (m_SampleRate > eSPS_LAST))
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] Bad sample rate; " << m_SampleRate << std::endl;
        return EXIT_FAILURE;
    }

    if (m_numSamples == 0)
    {
        // If duration was specified, calculate number of samples needed.
        m_numSamples = m_duration * m_SampleRate / 1000;
    }
    else if (m_duration == 0)
    {
        // If number of samples was specified, calculate duration needed.
        m_duration = 1000 * m_numSamples / m_SampleRate;
    }

    return EXIT_SUCCESS;
}

//*****************************************************************************

/*
 * Generate signal data based on the loaded signal information.
 */
int Signal::GenerateSignal(std::vector<double> &data)
{
    //*********************************
    // First check if the description info is valid
    bool isValid = true;

    if (xmlErrorId != tinyxml2::XML_SUCCESS)
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] XML file was not loaded successfully." << std::endl;
        isValid = false;
    }
    if ((m_SampleRate < eSPS_FIRST) || (m_SampleRate > eSPS_LAST))
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] Bad sample rate; " << m_SampleRate << std::endl;
        isValid = false;
    }
    if ((m_numTones < 1) || (m_numTones > MAX_SIG))
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] Invalid number of tones; " << m_numTones << std::endl;
        isValid = false;
    }
    if (!isValid)
    {
        return EXIT_FAILURE;
    }

    //*********************************
    // Generate waveform

    if (m_numSamples == 0)
    {
        // If duration was specified, calculate number of samples needed.
        m_numSamples = m_duration * m_SampleRate / 1000;
    }
    else if (m_duration == 0)
    {
        // If number of samples was specified, calculate duration needed.
        m_duration = 1000 * m_numSamples / m_SampleRate;
    }

    unsigned int riseSamples = (unsigned int)(m_RiseFactor * m_numSamples);
    unsigned int fallSamples = (unsigned int)(m_FallFactor * m_numSamples);
    unsigned int fallSamplesStart = m_numSamples - fallSamples;
    double riseFallAmp = 0.0;
    double riseStep = 1.0 / riseSamples;
    double fallStep = 1.0 / fallSamples;

    // Sample loop
    double max = 0.0;
    for (unsigned int i = 0; i < m_numSamples; i++)
    {
        double x = 2.0 * M_PI * i / m_SampleRate;
        double h = 0.0;
        double h_abs = 0.0;

        // Tone loop
        for (unsigned int j = 0; j < m_numTones; j++)
        {
            // Convert phase from degrees to radians
            double phase = M_PI * m_Signal[j].phase / 180.0;

            h += riseFallAmp * m_Signal[j].amp * sin(m_Signal[j].freq * x + phase);

            // Account for harmonics
            for (unsigned int k = 0; k < m_Signal[j].harmonics.size(); k++)
            {
                h += m_Signal[j].harmonicAmps[k] * riseFallAmp *
                     sin(m_Signal[j].harmonics[k] * m_Signal[j].freq * x + phase);
            }
        }

        // Recalculate envelope amplitude
        if (i < riseSamples)
        {
            riseFallAmp += riseStep;
        }
        else if (i > fallSamplesStart)
        {
            riseFallAmp -= fallStep;
        }
        else
        {
            riseFallAmp = 1.0;
        }

        h /= m_maxAmp;
        h_abs = fabs(h);

        // XXX
        if (h_abs > 100.0)
        {
            std::cout << "h: " << h << "h_abs: " << h_abs << std::endl;
        }
        // XXX

        if (h_abs > max)
        {
            max = h_abs;
        }

        data.push_back(h);
    }
    printf("  Max Amplitude = %.4f (%.4f dB)\n", max, 10 * log(max) / log(10));

    return EXIT_SUCCESS;
}

//*****************************************************************************

/*
 * Generate specified number of milliseconds of silence.
 */
int Signal::GenerateSilence(std::vector<double> &data, unsigned int ms)
{
    // First check if the description info is valid
    bool isValid = true;

    if ((m_SampleRate < eSPS_FIRST) || (m_SampleRate > eSPS_LAST))
    {
        std::cerr << "ERROR: [" << __FUNCTION__ << "] Bad sample rate; " << m_SampleRate << std::endl;
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

void Signal::printInfo()
{
    std::cout << "Signal: " << m_SigName << std::endl;
    std::cout << "  Description: " << m_Description << std::endl;
    std::cout << "  Sample Rate: " << m_SampleRate << std::endl;
    std::cout << "  Number of tones: " << m_numTones << std::endl;
    std::cout << "  Number of Samples: " << m_numSamples << " = " << m_duration << " mS" << std::endl;
    std::cout << "  Rise time: " << (m_RiseFactor * 100.0) << "% of duration = "
              << (m_RiseFactor * m_duration) << " mS " << std::endl;
    std::cout << "  Fall time: " << (m_FallFactor * 100.0) << "% of duration = "
              << (m_FallFactor * m_duration) << " mS " << std::endl;
    printf("  Maximum amplitude: %.2f\n", m_maxAmp);

    std::cout << "  Tones:" << std::endl;
    for (unsigned int i = 0; i < m_numTones; ++i)
    {
        (void)printf("    %3d | %4.2f %7.2f %.2f, Harmonics: ", i + 1,
                     m_Signal[i].amp, m_Signal[i].freq, m_Signal[i].phase);
        for (unsigned int j = 0; j < m_Signal[i].harmonics.size(); j++)
        {
            (void)printf("%d@%4.2f  ", m_Signal[i].harmonics[j], m_Signal[i].harmonicAmps[j]);
        }
        std::cout << std::endl;
    }
}

//*****************************************************************************
