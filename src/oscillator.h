#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "config.h"
#include "i2sSetup.h"
#include <Arduino.h>

class Oscillator
{

    double m_phs;
    float m_freq;
    double m_amp;
    double m_incr;
    static inline double m_table[WAVETABLE_SIZE] = {0.0};
    double m_out[AUDIO_BUFFER_SIZE];
    bool m_isSine;

public:
    Oscillator(double amp, float frequency) : m_amp(amp), m_freq(frequency), m_phs(0), m_isSine(1), m_incr(0)
    {
        std::fill_n(m_out, AUDIO_BUFFER_SIZE, 0.0);
    };

    void initWaveTable()
    {
        for (int i = 0; i < WAVETABLE_SIZE; ++i)
        {
            m_table[i] = sin(2.0 * M_PI * i / WAVETABLE_SIZE);
        }
    }

    const double *process(const double *a, float level, float f, const double *phsMod)
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

    const double *process(const double *a, float level, float f)
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
};

#endif