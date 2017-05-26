/**
 * @file    src/mod_musicbox.cpp
 * @brief
 *
 * @addtogroup
 * @{
 */

#include "mod_musicbox.h"


#if MOD_MUSICBOX

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch_tools.h"
#include "chprintf.h"

#include "qhal.h"
#include "module_init_cpp.h"

#include "ff.h"

#include "board_buttons.h"
#include "mod_rfid.h"
#include "mod_cardreader.h"
#include "mod_player.h"
#include "mod_effects.h"


#define EVENTMASK_RFID EVENT_MASK(0)
#define EVENTMASK_BTN_PLAY EVENT_MASK(1)
#define EVENTMASK_BTN_NEXT EVENT_MASK(2)
#define EVENTMASK_BTN_PREV EVENT_MASK(3)
#define EVENTMASK_BTN_VOLUP EVENT_MASK(4)
#define EVENTMASK_BTN_VOLDOWN EVENT_MASK(5)
#define EVENTMASK_CARDREADER EVENT_MASK(6)
#define EVENTMASK_PLAYER EVENT_MASK(6)

namespace tmb_musicplayer
{

template <>
ModuleMusicbox ModuleMusicboxSingelton::instance = tmb_musicplayer::ModuleMusicbox();

ModuleMusicbox::ModuleMusicbox()
{
    buttons[0].button = &BoardButtons::BtnPlay;
    buttons[Play].handler = &ModuleMusicbox::OnPlayButton;
    buttons[Play].evtMask = EVENTMASK_BTN_PLAY;

    buttons[1].button = &BoardButtons::BtnNext;
    buttons[Next].handler = &ModuleMusicbox::OnNextButton;
    buttons[Next].evtMask = EVENTMASK_BTN_NEXT;

    buttons[Prev].button = &BoardButtons::BtnPrev;
    buttons[Prev].handler = &ModuleMusicbox::OnPrevButton;
    buttons[Prev].evtMask = EVENTMASK_BTN_PREV;

    buttons[VolUp].button = &BoardButtons::BtnVolUp;
    buttons[VolUp].handler = &ModuleMusicbox::OnVolUpButton;
    buttons[VolUp].evtMask = EVENTMASK_BTN_VOLUP;

    buttons[VolDown].button = &BoardButtons::BtnVolDown;
    buttons[VolDown].handler = &ModuleMusicbox::OnVolDownButton;
    buttons[VolDown].evtMask = EVENTMASK_BTN_VOLDOWN;
}

ModuleMusicbox::~ModuleMusicbox()
{

}

void ModuleMusicbox::Init()
{
    m_modRFID = ModuleRFIDSingelton::GetInstance();
    m_modCardreader = ModuleCardreaderSingelton::GetInstance();
    m_modPlayer = ModulePlayerSingelton::GetInstance();
    m_modEffects = ModuleEffectsSingelton::GetInstance();
}

void ModuleMusicbox::Start()
{
    BaseClass::Start();
}

void ModuleMusicbox::Shutdown()
{
    BaseClass::Shutdown();
    m_modRFID = NULL;
   m_modCardreader = NULL;
   m_modPlayer = NULL;
}

void ModuleMusicbox::ThreadMain()
{
    chRegSetThreadName("musicbox");

    chibios_rt::EvtListener rfidEvtListener;
    chibios_rt::EvtListener cardreaderEvtListener;
    chibios_rt::EvtListener playerEvtListener;

    if (m_modRFID != NULL)
    {
        m_modRFID->RegisterListener(&rfidEvtListener, EVENTMASK_RFID);
    }

    if (m_modCardreader != NULL)
    {
        m_modCardreader->RegisterListener(&cardreaderEvtListener, EVENTMASK_CARDREADER);
    }

    if (m_modPlayer != NULL)
    {
        m_modPlayer->RegisterListener(&playerEvtListener, EVENTMASK_PLAYER);
    }

    RegisterButtonEvents();


    while (!chThdShouldTerminateX())
    {
        eventmask_t evt = chEvtWaitAny(ALL_EVENTS);
        if (evt & EVENTMASK_RFID)
        {
            eventflags_t flags = rfidEvtListener.getAndClearFlags();
            OnRFIDEvent(flags);
        }

        if (evt & EVENTMASK_CARDREADER)
        {
            eventflags_t flags = cardreaderEvtListener.getAndClearFlags();
            OnCardReaderEvent(flags);
        }

        if (evt & EVENTMASK_PLAYER)
        {
            eventflags_t flags = playerEvtListener.getAndClearFlags();
            OnPlayerEvent(flags);
        }

        /* process buttons */
        int i;
        for (i = 0; i < 5; i++)
        {
            ButtonData& btnData = buttons[i];
            if (evt & btnData.evtMask)
            {
                eventflags_t flags = btnData.evtListener.getAndClearFlags();
                (this->*btnData.handler)(btnData.button, flags);
            }
        }
    }

    UnregisterButtonEvents();

    if (m_modPlayer != NULL)
    {
        m_modPlayer->UnregisterListener(&playerEvtListener);
    }

    if (m_modCardreader != NULL)
    {
        m_modCardreader->UnregisterListener(&cardreaderEvtListener);
    }

    if (m_modRFID != NULL)
    {
        m_modRFID->UnregisterListener(&rfidEvtListener);
    }
}

void ModuleMusicbox::RegisterButtonEvents()
{
    for (int i = 0; i < ButtonTypeCount; i++)
    {
        ButtonData& btnData = buttons[i];
        btnData.button->RegisterListener(&btnData.evtListener, btnData.evtMask);
    }
}

void ModuleMusicbox::UnregisterButtonEvents()
{
    for (int i = 0; i < ButtonTypeCount; i++)
    {
        ButtonData& btnData = buttons[i];
        btnData.button->UnregisterListener(&btnData.evtListener);
    }
}

void ModuleMusicbox::OnPlayButton(Button* btn, eventflags_t flags)
{
    (void)btn;
    if (flags & Button::Up)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: Play button up event.\r\n");
        m_modPlayer->Toggle();
    }
}
void ModuleMusicbox::OnNextButton(Button* btn, eventflags_t flags)
{
    (void)btn;
    if (flags & Button::Pressed)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: Next button pressed event.\r\n");
        m_modPlayer->Next();
    }
}

void ModuleMusicbox::OnPrevButton(Button* btn, eventflags_t flags)
{
    (void)btn;
    if (flags & Button::Pressed)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: Prev button pressed event.\r\n");
        m_modPlayer->Prev();
    }
}

void ModuleMusicbox::OnVolUpButton(Button* btn, eventflags_t flags)
{
    (void)btn;
    if (flags & Button::Pressed)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: VolUp button pressed event.\r\n");
        ChangeVolume(-10);
    }
}

void ModuleMusicbox::OnVolDownButton(Button* btn, eventflags_t flags)
{
    (void)btn;
    if (flags & Button::Pressed)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: VolDown button pressed event.\r\n");
        ChangeVolume(10);
    }
}

void ModuleMusicbox::OnRFIDEvent(eventflags_t flags)
{
    if (flags & ModuleRFID::CardDetected)
    {
        if (m_modRFID->GetCurrentCardId(uid) == true)
        {
            hasRFIDCard = true;
            char pszUID[32];
            if (MifareUIDToString(uid, pszUID) > 0)
            {
                chprintf(DEBUG_CANNEL, "ModuleMusicbox: RFID detected: %s.\r\n", pszUID);
                ProcessMifareUID(pszUID);
            }
        }
    }

    if (flags & ModuleRFID::CardLost)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: RFID lost.\r\n");
        hasRFIDCard = false;
        m_modPlayer->Stop();

        m_modEffects->SetMode(ModuleEffects::ModeEmptyPlaylist);
    }
}

void ModuleMusicbox::OnCardReaderEvent(eventflags_t flags)
{
    if (flags & ModuleCardreader::FilesystemMounted)
    {
        if (hasRFIDCard == true)
        {
            if (m_modRFID->GetCurrentCardId(uid) == true)
            {
                hasRFIDCard = true;
                char pszUID[32];
                if (MifareUIDToString(uid, pszUID) > 0)
                {
                    ProcessMifareUID(pszUID);
                }
            }
        }
    }

    if (flags & ModuleCardreader::FilesystemUnmounted)
    {
        m_modPlayer->Stop();
        m_modEffects->SetMode(ModuleEffects::ModeEmptyPlaylist);
    }
}

void ModuleMusicbox::OnPlayerEvent(eventflags_t flags)
{
    if (flags & ModulePlayer::EventNext)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: player Next.\r\n");
    }
    else if (flags & ModulePlayer::EventPrev)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: player Prev.\r\n");
    }
    else if (flags & ModulePlayer::EventPlay)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: player Play.\r\n");
        m_modEffects->SetMode(ModuleEffects::ModePlay);
    }
    else if (flags & ModulePlayer::EventStop)
    {
        chprintf(DEBUG_CANNEL, "ModuleMusicbox: player Stop.\r\n");
        if (hasRFIDCard)
        {
            m_modEffects->SetMode(ModuleEffects::ModeStop);
        }
        else
        {
            m_modEffects->SetMode(ModuleEffects::ModeEmptyPlaylist);
        }
    }

}

void ModuleMusicbox::ChangeVolume(int16_t diff)
{
    volume = volume + diff;
    if (volume < 0)
    {
        volume = 0;
    }
    else if (volume > 254)
    {
        volume = 254;
    }

    //mod_musicplayer_cmdVolume((uint8_t)volume);
}

void ModuleMusicbox::ProcessMifareUID(const char* pszUID)
{
    DIR directory;

    FILINFO fileInfo;
    fileInfo.lfname = fileNameBuffer;
    fileInfo.lfsize = sizeof(fileNameBuffer);

    /*search for folder*/
    if (m_modCardreader != NULL)
    {
        if (m_modCardreader->CommandFind(&directory, &fileInfo, "/music", pszUID) == true)
        {
            chprintf(DEBUG_CANNEL, "ModuleMusicbox: Found directory: %s.\r\n", pszUID);

            m_modEffects->SetMode(ModuleEffects::ModeStop);

            memset(absoluteFileNameBuffer, 0, sizeof(absoluteFileNameBuffer));
            strcat(absoluteFileNameBuffer, "/music/");
            strcat(absoluteFileNameBuffer, fileInfo.lfname);

            m_modPlayer->Play(absoluteFileNameBuffer);
        }
    }

}

size_t ModuleMusicbox::MifareUIDToString(const MifareUID& uid, char* psz)
{
    int i;
    size_t charCount = 0;
    char* pszTarget = psz;
    for (i = 0; i < uid.size; i++)
    {
        char c = 0x0f & (uid.bytes[i] >> 4);
        if (c < 10)
        {
            *pszTarget = c + '0';
        }
        else
        {
            *pszTarget = c + ('A' - 10);
        }

        pszTarget++;
        charCount++;

        c = 0x0f & uid.bytes[i];
        if (c < 10)
        {
            *pszTarget = c + '0';
        }
        else
        {
            *pszTarget = c + ('A' - 10);
        }

        pszTarget++;
        charCount ++;
    }

    *pszTarget = 0;
    return charCount;
}

}

MODULE_INITCALL(2, qos::ModuleInit<tmb_musicplayer::ModuleMusicboxSingelton>::Init,
        qos::ModuleInit<tmb_musicplayer::ModuleMusicboxSingelton>::Start,
        qos::ModuleInit<tmb_musicplayer::ModuleMusicboxSingelton>::Shutdown)

#endif /* MOD_MUSICBOX */
/** @} */
