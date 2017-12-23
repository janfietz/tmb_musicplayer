/**
 * @file    src/mod_effects.c
 * @brief
 *
 * @addtogroup
 * @{
 */

#include "mod_effects.h"

#if MOD_EFFECTS

#include "ch_tools.h"
#include "watchdog.h"
#include "module_init_cpp.h"

#include "qhal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace tmb_musicplayer
{
template <>
ModuleEffects ModuleEffectsSingelton::instance = tmb_musicplayer::ModuleEffects();

MoodDefault ModuleEffects::defaultMood = MoodDefault();

/**
 * @brief
 */

ModuleEffects::ModuleEffects()
{

}

ModuleEffects::~ModuleEffects()
{

}

void ModuleEffects::Init()
{
    watchdog_register(WATCHDOG_MOD_EFFECTS);

    if (currentMood == NULL)
    {
        currentMood = &ModuleEffects::defaultMood;
    }
}

void ModuleEffects::Start()
{
    BaseClass::Start();
}

void ModuleEffects::Shutdown()
{
    BaseClass::Shutdown();
}

void ModuleEffects::SetMode(PlayModes mode)
{
    Msg* msg = (Msg*)m_MsgObjectPool.alloc();
    if (msg != NULL)
    {
        msg->mode = mode;
        if (m_Mailbox.post(msg, MS2ST(1)) != MSG_OK)
        {
            m_MsgObjectPool.free(msg);
        }
    }
}

void ModuleEffects::SetSpectrum(int8_t* current, int8_t* peak, int8_t bands)
{
    Msg* msg = (Msg*)m_MsgObjectPool.alloc();
    if (msg != NULL)
    {
        msg->mode = ModeSpectrumResult;

        memcpy(msg->spectrumCurrent, current, sizeof(msg->spectrumCurrent));
        memcpy(msg->spectrumPeak, peak, sizeof(msg->spectrumPeak));

        if (m_Mailbox.post(msg, MS2ST(1)) != MSG_OK)
        {
            m_MsgObjectPool.free(msg);
        }
    }
}

void ModuleEffects::SetBrightness(float brightness)
{
    Msg* msg = (Msg*)m_MsgObjectPool.alloc();
    if (msg != NULL)
    {
        msg->mode = ModeBrightness;
        msg->brightness = brightness;

        if (m_Mailbox.post(msg, MS2ST(1)) != MSG_OK)
        {
            m_MsgObjectPool.free(msg);
        }
    }
}

void ModuleEffects::ThreadMain()
{
    chRegSetThreadName("effects");

    PlayModes currentMode = ModeEmptyPlaylist;
    currentMood->SwitchMode((uint8_t)currentMode);

    while (!chThdShouldTerminateX())
    {
        watchdog_reload(WATCHDOG_MOD_EFFECTS);

       /* Processing the event.*/
       Msg* msg = NULL;
       while (m_Mailbox.fetch(&msg, TIME_IMMEDIATE) == MSG_OK)
       {
           if (msg->mode == ModeSpectrumResult)
           {
               currentMood->SetSpectrum(msg->spectrumCurrent, msg->spectrumPeak, sizeof(msg->spectrumPeak));
           }
           else if (msg->mode == ModeBrightness)
           {
               m_brightness = msg->brightness;
           }
           else {
               if (currentMode != msg->mode)
               {
                   currentMood->SwitchMode(msg->mode);
               }
               currentMode = msg->mode;
           }
           m_MsgObjectPool.free(msg);
       }

        DrawCurrentMood();
        chibios_rt::BaseThread::sleep(MS2ST(10));
    }
}

void ModuleEffects::DrawCurrentMood()
{
    int i;
    systime_t current = chVTGetSystemTime();

    currentMood->Draw(current, &display);

#if HAL_USE_WS281X
    for (i = 0; i < LEDCOUNT; i++)
    {
        Color* color = &display.pixels[i];
        /*
         * Scale brightness using a factor from global settings.
         */
        ColorScale(color, m_brightness);
        ws281xSetColor(&ws281x, i, color->R, color->G, color->B);
    }

    ws281xUpdate(&ws281x);
#endif /* HAL_USE_WS281X */
}

}

MODULE_INITCALL(6, qos::ModuleInit<tmb_musicplayer::ModuleEffectsSingelton>::Init,
        qos::ModuleInit<tmb_musicplayer::ModuleEffectsSingelton>::Start,
        qos::ModuleInit<tmb_musicplayer::ModuleEffectsSingelton>::Shutdown)

#endif

/** @} */
