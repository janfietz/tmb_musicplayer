/**
 * @file    src/mood_default.h
 * @brief
 *
 * @addtogroup
 * @{
 */

#ifndef _MOOD_DEFAULT_H_
#define _MOOD_DEFAULT_H_

#include "effect_buttons.h"
#include "effect_randompixels.h"
#include "effect_fadingpixels.h"
#include "mood.h"

namespace tmb_musicplayer
{
/**
 * @brief
 */
class MoodDefault : public Mood
{
public:
    MoodDefault();
    ~MoodDefault();

    virtual void Draw(systime_t sysTime, DisplayBuffer* display);
    virtual void SwitchMode(uint8_t mode);
    virtual void SetSpectrum(int8_t* current, int8_t* peak, int8_t bands);

private:
    void DrawSpectrum(systime_t sysTime, DisplayBuffer* display);

    uint8_t m_newMode = EFFECT_BUTTON_MODE_EMPTYPLAYLIST;
    uint8_t m_currentMode = EFFECT_BUTTON_MODE_EMPTYPLAYLIST;
    uint8_t m_newButtonMode = EFFECT_BUTTON_MODE_EMPTYPLAYLIST;
    uint8_t m_currentButtonMode = EFFECT_BUTTON_MODE_EMPTYPLAYLIST;
    systime_t m_modeChangedTime;
    float brightness = 1.0f;

    int8_t m_spectrumCurrent[5];
    int8_t m_spectrumPeak[5];

    bool m_showButtons = true;

    EffectButtonsCfg effButtons_cfg =
    {
        .play = {
            .x = 2,
            .y = 0,
            .color = {0x51, 0xBD, 0x1F},
        },
        .vol_up = {
            .x = 4,
            .y = 0,
            .color = {0x46, 0x08, 0x4E},
        },
        .vol_down = {
            .x = 0,
            .y = 0,
            .color = {0x46, 0x08, 0x4E},
        },
        .next = {
            .x = 3,
            .y = 0,
            .color = {0xFF, 0xD6, 0x00},
        },
        .prev = {
            .x = 1,
            .y = 0,
            .color = {0xFF, 0xD6, 0x00},
        },
        .special = {
            .x = 5,
            .y = 0,
            .color = {0xFF, 0x5F, 0x00},
        },

        .playMode = EFFECT_BUTTON_MODE_EMPTYPLAYLIST,
        .colorModeEmptyPlayList = {0x29, 0x00, 0x02},
        .colorModePause = {0xFF, 0x6D, 0x00},
        .colorModeStop = {0xE4, 0x24, 0x2E},

        .blendperiod = MS2ST(500),
    };

    EffectButtonsData effButtons_data =
    {
        .lastPlayMode = EFFECT_BUTTON_MODE_EMPTYPLAYLIST,
        .lastBlendStep = 1.0f,
        .lastPlayModeColor = {0x29, 0x00, 0x02},
        .lastUpdate = 0,
    };

    Effect effButtons =
    {
        .effectcfg = &effButtons_cfg,
        .effectdata = &effButtons_data,
        .update = &EffectButtonsUpdate,
        .reset = &EffectButtonsReset,
        .p_next = NULL,
    };

    EffectRandomPixelsCfg effRandomCfg = {
        .spawninterval = MS2ST(1000),
        .color = {0xFF, 0xFF, 0xFF},
        .randomRed = true,
        .randomGreen = true,
        .randomBlue = true,
    };

    Color m_RandomPixelColors[5];
    EffectRandomPixelsData effRandomData = {
        .lastspawn = 0,
        .pixelColors = m_RandomPixelColors,
    };

    Effect effRandomPixel =
    {
        .effectcfg = &effRandomCfg,
        .effectdata = &effRandomData,
        .update = &EffectRandomPixelsUpdate,
        .reset = &EffectRandomPixelsReset,
        .p_next = NULL,
    };

    EffectFadingPixelsCfg effFadingButtons_cfg = {
        .color = {0xFF, 0xFF, 0xFF},
        .randomColor = true,
        .number = 1,
        .spawninterval = MS2ST(2000),
        .fadeperiod = MS2ST(2000)
    };

    EffectFadeState m_fadeStates[5];
    EffectFadingPixelsData effFadingButtons_data =
    {
        .lastspawn = 0,
        .lastupdate = 0,
        .fadeStates = m_fadeStates,
        .pixelColors = m_RandomPixelColors,
    };

    Effect effFadingPixel =
    {
        .effectcfg = &effFadingButtons_cfg,
        .effectdata = &effFadingButtons_data,
        .update = &EffectFadingPixelsUpdate,
        .reset = &EffectFadingPixelsReset,
        .p_next = NULL,
    };
};

}
#endif /* _MOOD_DEFAULT_H_ */

/** @} */
