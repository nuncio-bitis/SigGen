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
/*****************************************************************************

 FILE NAME: SeqGen.c

 DESCRIPTION:
   Generate a sequence of signals from an input file containing a sequence description.

 Format:
   SeqGen.exe Sequence_Description_File

 AUTHOR: J. Parziale
 Copyright   : 2001-2022

 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <map>

#include "SigGen.h"
#include "Sequence.h"
#include "Signal.h"
#include "DTMF.h"

#define SAMPLE_TYPE float

/*****************************************************************************/
int main(int argc, char *argv[])
{
    FILE *f_sig, *f_dat;
    Sequence mySequence;
    std::vector<double> waveformData;

    std::string outFileName;
    std::string rawFileName;

    //*************************************************************************

    std::cout << argv[0] << " Version " << seqgen_VERSION_MAJOR << "."
              << seqgen_VERSION_MINOR << std::endl
              << std::endl;

    if (argc < 2)
    {
        (void)fprintf(stderr, "ERROR: Not enough arguments.\n");
        (void)fprintf(stderr, "Format: %s Sequence_Description_File\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::cout << std::endl;

    //*************************************************************************
    // Load sequence information

    if (mySequence.LoadFile(argv[1]) != EXIT_SUCCESS)
    {
        std::cerr << "ERROR: Could not load sequence file " << argv[1] << std::endl;
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Sequence/Signal file '" << argv[1] << "' loaded." << std::endl;

    //*************************************************************************

    mySequence.printInfo();
    std::cout << std::endl;
    std::cout << "Generate sequence consisting of " << mySequence.numSegments() << " segments." << std::endl;
    std::cout << "================================================================================" << std::endl;

    //*************************
    // Add silence at the beginning
    mySequence.GenerateSilence(waveformData, 10);

    //*************************
    // Generate waveform data from input signal description
    std::cout << "=== Generate waveform" << std::endl;

    if (mySequence.GenerateWaveform(waveformData) != EXIT_SUCCESS)
    {
        std::cerr << "ERROR: Could not generate sequence" << std::endl;
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    //*************************
    // Add silence at the end
    mySequence.GenerateSilence(waveformData, 10);

    std::cout << "================================================================================" << std::endl;

    //*************************************************************************
#if 0
    // Normalize overall waveform for max amplitude to be around 0 dB
    double max = 0.0;
    for (unsigned int i = 0; i < waveformData.size(); i++)
    {
        double h = waveformData[i];
        double h_abs = fabs(h);
        if (h_abs > max)
        {
            max = h_abs;
        }
    }
    double maxAmp = max;
    max = 0.0;
    for (unsigned int i = 0; i < waveformData.size(); i++)
    {
        double h = waveformData[i] / maxAmp;
        waveformData[i] = h;
        double h_abs = fabs(h);
        if (h_abs > max)
        {
            max = h_abs;
        }
    }
    printf("*** Normalized waveform amplitude: %.4f (%.4f dB)\n", max, 10 * log(max)/log(10));
#endif

    //*************************************************************************
    // Output waveform from signalData

    outFileName = mySequence.name() + ".out";
    rawFileName = mySequence.name() + ".dat";

    if ((f_sig = fopen(outFileName.c_str(), "w")) == NULL)
    {
        (void)fprintf(stderr, "ERROR: Can't open output file: %s\n", outFileName.c_str());
        return EXIT_FAILURE;
    }

    if ((f_dat = fopen(rawFileName.c_str(), "wb")) == NULL)
    {
        (void)fprintf(stderr, "ERROR: Can't open output file: %s\n", rawFileName.c_str());
        return EXIT_FAILURE;
    }

    // Sample loop
    for (unsigned int i = 0; i < waveformData.size(); i++)
    {
        SAMPLE_TYPE h = (SAMPLE_TYPE)waveformData[i];

        (void)fprintf(f_sig, "%11.8f\n", h);
        fwrite((const void *)&h, sizeof(h), 1, f_dat);
    }
    fclose(f_sig);
    fclose(f_dat);

    //*************************

    std::cout << std::endl;
    std::cout << "Number of Samples:  " << waveformData.size() << " = ";
    std::cout << (1000.0 * (SAMPLE_TYPE)waveformData.size() / (SAMPLE_TYPE)mySequence.sampleRate()) << " mS" << std::endl;
    std::cout << "Sample size:        " << (sizeof(SAMPLE_TYPE) * 8) << " bits, float" << std::endl;

    std::cout << "Output text file:   " << outFileName << std::endl;
    std::cout << "Output binary file: " << rawFileName << std::endl;
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
