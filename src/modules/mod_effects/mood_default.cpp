/**
 * @file    src/mood_default.cpp
 * @brief
 *
 * @addtogroup
 * @{
 */

#include "mood_default.h"
#include <string.h>

namespace tmb_musicplayer {

MoodDefault::MoodDefault() {
}

MoodDefault::~MoodDefault() {
}

void MoodDefault::SwitchMode(uint8_t mode) {
    m_newMode = mode;
}

void MoodDefault::SetSpectrum(int8_t* current, int8_t* peak, int8_t bands)
{
    memcpy(m_spectrumCurrent, current, sizeof(m_spectrumCurrent));
    memcpy(m_spectrumPeak, peak, sizeof(m_spectrumPeak));
}

void MoodDefault::Draw(systime_t sysTime, DisplayBuffer* display) {

    if (m_newMode != m_currentMode) {
        m_currentMode = m_newMode;
        if (m_currentMode <= EFFECT_BUTTON_MODE_EMPTYPLAYLIST) {
            effButtons_cfg.playMode = m_currentMode;
        }
        m_modeChangedTime = sysTime;
        m_showButtons = true;

        if (m_newMode == 4)
        {
            const uint32_t pixelCount = display->height * display->width;
            for (uint32_t i = 0; i < pixelCount; i++)
            {
                ColorCopy(&display->pixels[i], &m_RandomPixelColors[i]);
                m_fadeStates[i].fadesequence = 0;
            }
            EffectReset(&effFadingPixel, 0, 0, sysTime);
        }
    }

    const int16_t pixelCount = display->height * display->width;
    memset(display->pixels, 0, sizeof(struct Color) * pixelCount);

    if (m_currentMode == 4)
    {
        EffectUpdate(&effFadingPixel, 0, 0, sysTime, display);

    }
    else if (m_currentMode == 5)
    {

    }
    else
    {
        if (m_showButtons) {
            EffectUpdate(&effButtons, 0, 0, sysTime, display);

            if ((sysTime - m_modeChangedTime) >= MS2ST(10000)) {
                m_showButtons = false;
            }
        } else {
            if (m_currentMode == EFFECT_BUTTON_MODE_PLAY) {
                DrawSpectrum(sysTime, display);
            } else {
                EffectUpdate(&effButtons, 0, 0, sysTime, display);
            }
        }
    }
}

void MoodDefault::DrawSpectrum(systime_t sysTime, DisplayBuffer* display) {

    static Color m_spectrumColors[] = {
                {0,0,0},
                {6,5,8},
                {13,15,16},
                {19,16,24},
                {26,21,32},
                {32,26,40},
                {0x27,0x20,0x30},
                {0x49,0x31,0x34},
                {0x70,0x45,0x37},
                {0x91,0x55,0x3b},
                {0xb0,0x65,0x3e},
                {0xce,0x74,0x41},
                {0xE4,0x82,0x44},
                {0xd7,0x89,0x3},
                {0xc5,0x90,0x3b},
                {0xaf,0x99,0x36},
                {0x9e,0x9f,0x32},
                {0x8c,0xa6,0x2d},
                {0x7a,0xad,0x29},
                {0x64,0xb6,0x24},
                {0x51,0xBD,0x1F},
                {0x52,0xb8,0x60},
                {0x53,0xb2,0x9e},
                {0x53,0xaf,0xc4},
                {0x7b,0xaa,0xcb},
                {0x88,0xaa,0xb9},
                {0xea,0xa9,0xcf},
                {0xac,0xaa,0x89},
                {0xbb,0xaa,0x75},
                {0xcf,0xaa,0x5a},
                {0xE4,0xAA,0x38},
        };

    for (uint32_t i = 0; i < sizeof(m_spectrumCurrent); i++)
    {
        DisplayDraw(i, 0, &m_spectrumColors[m_spectrumCurrent[i]],display);
    }
}

}  // namespace tmb_musicplayer

/** @} */
