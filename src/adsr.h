#ifndef ADSR_H
#define ADSR_H

#include "config.h"

class ADSR
{

public:
    enum class State
    {
        IDLE,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE
    };
    float m_attack;
    float m_decay;
    float m_release;
    float m_sustainLevel;

private:
    double m_attackIncr;
    double m_decayIncr;
    double m_releaseIncr;
    double m_currentVal;
    double m_out[AUDIO_BUFFER_SIZE];
    State m_state;
    bool m_isLinear;

    void calcIncr(float attack, float decay, float release)
    {
        m_attackIncr = 1.0 / (AUDIO_SR * attack);
        m_decayIncr = 1.0 / (AUDIO_SR * decay);
        m_releaseIncr = 1.0 / (AUDIO_SR * release);
    }

public:
    ADSR(bool linear) : m_attackIncr(0.1), m_decayIncr(0.1), m_sustainLevel(0.7), m_releaseIncr(0.1), m_state(State::IDLE), m_currentVal(0), m_isLinear(linear)
    {
        std::fill_n(m_out, AUDIO_BUFFER_SIZE, 0.0);
    };

    void setParams(float atk, float dec, float sus, float rel)
    {
        m_attack = atk;
        m_decay = dec;
        m_release = rel;
        calcIncr(atk, dec, rel);
        m_sustainLevel = sus;
    }

    void getParams(float adsrParams[4])
    {
        adsrParams[0] = m_attack;
        adsrParams[1] = m_decay;
        adsrParams[2] = m_sustainLevel;
        adsrParams[3] = m_release;
    }

    double *process(bool gate, float vel)
    {
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        {

            if (gate)
            {
                if (m_state != State::ATTACK && m_state != State::SUSTAIN && m_state != State::DECAY)
                {
                    m_state = State::ATTACK;
                }
            }
            else
            {
                if (m_state != State::RELEASE || m_state != State::IDLE)
                {
                    m_state = State::RELEASE;
                }
            }

            switch (m_state)
            {
            case State::IDLE:
                m_currentVal = 0.0;
                break;

            case State::ATTACK:
                m_currentVal += m_attackIncr;
                if (m_currentVal >= vel)
                {
                    m_currentVal = vel;
                    m_state = State::DECAY;
                }
                break;

            case State::DECAY:
                m_currentVal -= m_decayIncr;
                if (m_currentVal <= m_sustainLevel * vel)
                {
                    m_currentVal = m_sustainLevel * vel;
                    m_state = State::SUSTAIN;
                }
                break;

            case State::SUSTAIN:
                break;

            case State::RELEASE:
                m_currentVal -= m_releaseIncr;
                if (m_currentVal <= 0.0)
                {
                    m_currentVal = 0.0;
                    m_state = State::IDLE;
                }

                break;
            }
            m_out[i] = m_currentVal;
            // Serial.println(m_currentVal);
        }
        return m_out;
    }
};
#endif