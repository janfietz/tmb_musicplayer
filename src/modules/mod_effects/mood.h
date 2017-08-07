/**
 * @file    src/mood.h
 * @brief
 *
 * @addtogroup
 * @{
 */

#ifndef _MOOD_H_
#define _MOOD_H_

namespace tmb_musicplayer
{
/**
 * @brief
 */
class Mood
{
public:
    virtual void Draw(systime_t sysTime, DisplayBuffer* display) = 0;
    virtual void SwitchMode(uint8_t mode) = 0;
    virtual void SetSpectrum(int8_t* current, int8_t* peak, int8_t bands) = 0;
};

}
#endif /* _MOOD_H_ */

/** @} */
