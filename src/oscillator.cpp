#include "oscillator.h"
#include <cmath>

double Oscillator::m_table[WAVETABLE_SIZE] = {0};

void Oscillator::initWaveTable()
{
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        m_table[i] = sin(2.0 * M_PI * i / WAVETABLE_SIZE);
    }
}

const double *Oscillator::process(const double *a, float level, float f, const double *phsMod)
{
    m_incr = f * WAVETABLE_SIZE / AUDIO_SR;
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        int index = int(m_phs);
        float frac = m_phs - index;
        double val1 = m_table[index];
        double val2 = m_table[(index + 1) % WAVETABLE_SIZE];
        double val = (val1 + (frac * (val2 - val1))) * level;
        m_out[i] = val * (a[i]);
        m_phs += phsMod[i];
        m_phs += m_incr;
        while (m_phs >= WAVETABLE_SIZE)
            m_phs -= WAVETABLE_SIZE;
        while (m_phs < 0)
            m_phs += WAVETABLE_SIZE;
    }
    return m_out;
};

const double *Oscillator::process(const double *a, float level, float f)
{
    m_incr = f * WAVETABLE_SIZE / AUDIO_SR;
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        int index = int(m_phs);
        float frac = m_phs - index;
        double val1 = m_table[index];
        double val2 = m_table[(index + 1) % WAVETABLE_SIZE];
        double val = (val1 + (frac * (val2 - val1))) * level;
        m_out[i] = val * (a[i]);
        m_phs += m_incr;
        while (m_phs >= WAVETABLE_SIZE)
            m_phs -= WAVETABLE_SIZE;
        while (m_phs < 0)
            m_phs += WAVETABLE_SIZE;
    }
    return m_out;
}