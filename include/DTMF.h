#ifndef DTMF_H_
#define DTMF_H_
//*****************************************************************************

#include  <stdint.h>
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
