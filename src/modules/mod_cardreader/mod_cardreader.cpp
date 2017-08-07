/**
 * @file    src/mod_cardreader.cpp
 * @brief
 *
 * @addtogroup
 * @{
 */

#include "mod_cardreader.h"

#if MOD_CARDREADER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch_tools.h"
#include "chprintf.h"

#include "qhal.h"
#include "module_init_cpp.h"

#include "watchdog.h"
#include "board_buttons.h"


#if HAL_USE_SDC
#else
#error "SDC driver must be specified"
#endif

template <>
tmb_musicplayer::ModuleCardreader tmb_musicplayer::ModuleCardreaderSingelton::instance = tmb_musicplayer::ModuleCardreader();

namespace tmb_musicplayer
{

ModuleCardreader::ModuleCardreader()
{

}

ModuleCardreader::~ModuleCardreader()
{

}

void ModuleCardreader::Init()
{

}

void ModuleCardreader::Start()
{
    BaseClass::Start();
}

void ModuleCardreader::Shutdown()
{
    BaseClass::Shutdown();
}

void ModuleCardreader::RegisterListener(chibios_rt::EvtListener* listener, eventmask_t mask)
{
    m_evtSource.registerMask(listener, mask);
}

void ModuleCardreader::UnregisterListener(chibios_rt::EvtListener* listener)
{
    m_evtSource.unregister(listener);
}


void ModuleCardreader::ThreadMain()
{
    chRegSetThreadName("cardReader");

    chibios_rt::EvtListener carddetectEvtListener;

#if HAL_USE_BUTTONS
    BoardButtons::BtnCardDetect.RegisterListener(&carddetectEvtListener, EVENT_MASK(0));
#endif

    if (BoardButtons::BtnCardDetect.GetState() == false)
    {
        OnCardInserted();
    }

    while (!chThdShouldTerminateX())
    {
        eventmask_t evt = chEvtWaitAny(ALL_EVENTS);
        if (evt & EVENT_MASK(0))
        {
            eventflags_t flags = carddetectEvtListener.getAndClearFlags();
            if (flags & Button::Up)
            {
                OnCardRemoved();
            }
            else if (flags & Button::Down)
            {
                OnCardInserted();
            }
        }
    }

    OnCardRemoved();

#if HAL_USE_BUTTONS
    BoardButtons::BtnCardDetect.UnregisterListener(&carddetectEvtListener);
#endif
}

void ModuleCardreader::OnCardRemoved()
{
    chprintf(DEBUG_CANNEL, "ModuleCardreader: Memory card removed.\r\n");

    UnmountFilesystem();

    m_evtSource.broadcastFlags(FilesystemUnmounted);

    SetCardDetectLed(false);
}

void ModuleCardreader::OnCardInserted()
{
    chprintf(DEBUG_CANNEL, "ModuleCardreader: Memory card inserted.\r\n");

    if (MountFilesystem() == true)
    {
        m_evtSource.broadcastFlags(FilesystemMounted);
        SetCardDetectLed(true);
    }
}

bool ModuleCardreader::MountFilesystem()
{
    if (sdcConnect(&SDCD1) == HAL_SUCCESS)
    {
        FRESULT err;
        err = f_mount(&m_filesystem, "/mount/", 1);
        if (err != FR_OK) {
            chprintf(DEBUG_CANNEL, "ModuleCardreader: FS: f_mount() failed. Is the SD card inserted?\r\n");
            PrintFilesystemError(DEBUG_CANNEL, err);
            return false;
        }
        chprintf(DEBUG_CANNEL, "ModuleCardreader: FS: f_mount() succeeded\r\n");

        return true;
    }
    chprintf(DEBUG_CANNEL, "ModuleCardreader: Failed to connect sdc card.\r\n");

    return false;
}

bool ModuleCardreader::UnmountFilesystem()
{
    FRESULT err;

    err = f_mount(NULL, "/mount/", 0);

    sdcDisconnect(&SDCD1);
    if (err != FR_OK) {
        chprintf(DEBUG_CANNEL, "ModuleCardreader: FS: f_mount() unmount failed\r\n");
        PrintFilesystemError(DEBUG_CANNEL, err);
        return false;
    }

    chprintf(DEBUG_CANNEL, "ModuleCardreader: FS: f_mount() unmount succeeded\r\n");
    return true;
}

void ModuleCardreader::SetCardDetectLed(bool on)
{
#if HAL_USE_LED
    if (on == true)
    {
        ledOn(LED_CARDDETECT);
    }
    else
    {
        ledOff(LED_CARDDETECT);
    }
#endif /* HAL_USE_LED */

}


bool ModuleCardreader::CommandCD(const char* path)
{
    DIR dir;
    FRESULT res = f_opendir(&dir, path);
    if (res == FR_OK)
    {
        return true;
    }
    chprintf(DEBUG_CANNEL, "ModuleCardreader: FS: f_opendir \"%s\" failed\r\n", path);
    PrintFilesystemError(DEBUG_CANNEL, res);
    return false;
}

bool ModuleCardreader::CommandFind(DIR* dp, FILINFO* fno, const char* path, const char* pattern)
{
    FRESULT res = f_findfirst(dp, fno, path, pattern);
    if (res == FR_OK)
    {
        return true;
    }

    chprintf(DEBUG_CANNEL, "ModuleCardreader: FS: f_findfirst \"%s\" in path \"%s\" failed\r\n", pattern, path);
    PrintFilesystemError(DEBUG_CANNEL, res);
    return false;
}

void ModuleCardreader::PrintFilesystemError(BaseSequentialStream* chp, FRESULT err)
{
    chprintf(chp, "ModuleCardreader: \t%s.\r\n", FilesystemResultToString(err));
}

const char* ModuleCardreader::FilesystemResultToString(FRESULT stat)
{
    static const char* ErrorStrings[] = {
        "Succeeded",
        "A hard error occurred in the low level disk I/O layer",
        "Assertion failed",
        "The physical drive cannot work",
        "Could not find the file",
        "Could not find the path",
        "The path name format is invalid",
        "Access denied due to prohibited access or directory full",
        "Access denied due to prohibited access",
        "The file/directory object is invalid",
        "The physical drive is write protected",
        "The logical drive number is invalid",
        "The volume has no work area",
        "There is no valid FAT volume",
        "The f_mkfs() aborted due to any parameter error",
        "Could not get a grant to access the volume within defined period",
        "The operation is rejected according to the file sharing policy",
        "LFN working buffer could not be allocated",
        "Number of open files > _FS_SHARE",
        "Given parameter is invalid",
        "Unknown"
    };

    if (stat > FR_INVALID_PARAMETER)
    {
        return ErrorStrings[20];
    }

    return ErrorStrings[stat];
}

}

MODULE_INITCALL(3, qos::ModuleInit<tmb_musicplayer::ModuleCardreaderSingelton>::Init,
        qos::ModuleInit<tmb_musicplayer::ModuleCardreaderSingelton>::Start,
        qos::ModuleInit<tmb_musicplayer::ModuleCardreaderSingelton>::Shutdown)

#endif /* MOD_CARDREADER */
/** @} */
