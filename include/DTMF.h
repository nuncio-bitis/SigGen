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
#ifndef DTMF_H_
#define DTMF_H_
//*****************************************************************************

#include <stdint.h>
#include <string>
#include <vector>

//*****************************************************************************

class DTMF
{
public:
    static const uint32_t cDefaultSampleRate = 9600;
    static const uint32_t cDefaultDuration = 50; // milliseconds

    DTMF(char digit, uint32_t on = cDefaultDuration, uint32_t off = cDefaultDuration);
    ~DTMF();

    char name() { return m_dtmfDigit; };

    void GenerateData(
        std::vector<double> &data,
        uint32_t wSampleRate = cDefaultSampleRate);

private:
    char     m_dtmfDigit = '\0';
    uint32_t m_onDuration = 0;
    uint32_t m_offDuration = 0;
    uint32_t m_lowTone = 0;
    uint32_t m_highTone = 0;
};

//*****************************************************************************
#endif // DTMF_H_
