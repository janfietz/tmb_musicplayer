/**
 * @file    src/mod_effects.h
 * @brief
 *
 * @addtogroup
 * @{
 */

#ifndef _MOD_EFFECTS_H_
#define _MOD_EFFECTS_H_

#include "target_cfg.h"
#include "threadedmodule.h"
#include "singleton.h"

#include "color.h"
#include "display.h"

#include "mood.h"
#include "mood_default.h"

#if MOD_EFFECTS

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/
#ifndef MOD_EFFECTS_THREADSIZE
#define MOD_EFFECTS_THREADSIZE 512
#endif

#ifndef MOD_EFFECTS_THREADPRIO
#define MOD_EFFECTS_THREADPRIO LOWPRIO
#endif

#ifndef LEDCOUNT
#error "LEDCOUNT driver must be specified for this target"
#endif

#ifndef DISPLAY_WIDTH
#error "DISPLAY_WIDTH driver must be specified for this target"
#endif

#ifndef DISPLAY_HEIGHT
#error "DISPLAY_HEIGHT driver must be specified for this target"
#endif

namespace tmb_musicplayer
{
/**
 * @brief
 */

class ModuleEffects : public qos::ThreadedModule<MOD_EFFECTS_THREADSIZE>
{
public:

    enum PlayModes
    {
        ModePlay = 0,
        ModePause,
        ModeStop,
        ModeEmptyPlaylist,
        ModeStandby,
        ModeDeepStandby,
        ModeSpectrumResult,
        ModeBrightness,
    };

    ModuleEffects();
    ~ModuleEffects();

    virtual void Init();
    virtual void Start();
    virtual void Shutdown();

    void SetMode(PlayModes mode);
    void SetBrightness(float brightness);
    void SetSpectrum(int8_t* current, int8_t* peak, int8_t bands);

protected:
    typedef qos::ThreadedModule<MOD_EFFECTS_THREADSIZE> BaseClass;

    virtual void ThreadMain();
    virtual tprio_t GetThreadPrio() const {return MOD_EFFECTS_THREADPRIO;}

private:
    void DrawCurrentMood();

    float m_brightness = 0.9f; // do not use full brightness
    Color displayPixel[LEDCOUNT];
    DisplayBuffer display =
    {
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT,
        .pixels = displayPixel,
    };

    Mood* currentMood = NULL;

    class Msg
    {
    public:
        PlayModes mode;
        int8_t spectrumCurrent[5];
        int8_t spectrumPeak[5];
        uint8_t spare2;
        uint8_t spare3;
        float brightness;
    };


    chibios_rt::ObjectsPool<Msg, 2> m_MsgObjectPool;
    chibios_rt::Mailbox<Msg*, 2> m_Mailbox;

    static MoodDefault defaultMood;

};
typedef qos::Singleton<ModuleEffects> ModuleEffectsSingelton;

}
#endif /* MOD_EFFECTS */
#endif /* _MOD_EFFECTS_H_ */

/** @} */
