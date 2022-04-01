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
#include <cstdlib>
#include <iostream>
#include <math.h>

#include "DTMF.h"

#ifndef M_PI
#define M_PI (2.0 * asin(1.0))
#endif

//*****************************************************************************

DTMF::DTMF(char digit, uint32_t on, uint32_t off)
    : m_dtmfDigit(digit), m_onDuration(on), m_offDuration(on), m_lowTone(0), m_highTone(0)
{
    switch (digit)
    {
    case '1':
    case '2':
    case '3':
    case 'A':
        m_lowTone = 697;
        break;
    case '4':
    case '5':
    case '6':
    case 'B':
        m_lowTone = 770;
        break;
    case '7':
    case '8':
    case '9':
    case 'C':
        m_lowTone = 852;
        break;
    case '*':
    case '0':
    case '#':
    case 'D':
        m_lowTone = 941;
        break;
    default:
        std::cerr << "ERROR: Invalid DTMF tone specified; " << digit << std::endl;
    }

    switch (digit)
    {
    case '1':
    case '4':
    case '7':
    case '*':
        m_highTone = 1209;
        break;
    case '2':
    case '5':
    case '8':
    case '0':
        m_highTone = 1336;
        break;
    case '3':
    case '6':
    case '9':
    case '#':
        m_highTone = 1477;
        break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
        m_highTone = 1633;
        break;
    default:
        std::cerr << "ERROR: Invalid DTMF tone specified; " << digit << std::endl;
    }
}

//*****************************************************************************

DTMF::~DTMF()
{
}

//*****************************************************************************

/*
 * Generate signal data based on the loaded signal information.
 */
void DTMF::GenerateData(std::vector<double> &data, uint32_t wSampleRate)
{
    uint32_t onSamples = m_onDuration * wSampleRate / 1000;

    double pi_prod_1 = (2.0 * M_PI * m_lowTone) / wSampleRate;
    double pi_prod_2 = (2.0 * M_PI * m_highTone) / wSampleRate;

    // DTMF tone
    for (uint32_t i = 0; i < onSamples; i++)
    {
        double h = (sin(i * pi_prod_1) + sin(i * pi_prod_2)) / 2.0;
        data.push_back(h);
    }

    // Following silence
    uint32_t offSamples = m_offDuration * wSampleRate / 1000;
    for (uint32_t i = 0; i < offSamples; i++)
    {
        data.push_back(0.0);
    }
}

//*****************************************************************************
