/*
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "target_cfg.h"
#include "chprintf.h"
#if HAL_USE_SDC
#include "ff.h"

static void halt_error(uint8_t code)
{
    ledOff(LED_READ);
    while (1)
    {
        ledBlink(LED_STATUS, OSAL_MS2ST(200), OSAL_MS2ST(200), code);

        chThdSleepMilliseconds(code * 400 + 1000);
    }
}
#endif /* HAL_USE_SDC */

/*
 * Application entry point.
 */
int main(void)
{
    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    boardStart();

    ledOn(LED_STATUS);

    #define BLOCK_SIZE 8

    if (sdcConnect(&SDCD1) == HAL_SUCCESS) {
        /* Mount filesystem. */
        FATFS fs;
        FRESULT result = f_mount(&fs, "/mnt/", 1);
        if (result == FR_OK)
        {
            /* Check for update file. */
            {
                FIL fwufile;
                result = f_open(&fwufile, "firmware.bin", FA_READ | FA_OPEN_EXISTING);

                if (result == FR_OK)
                {
                    ledOn(LED_READ);
                    /* Erase flash partition. */

                    wdgReset(&WDGD1);
                    nvmMassErase(PARTITION_FW);

                    /* Write new firmware to flash. */
                    const DWORD update_size = f_size(&fwufile);
                    const size_t n_blocks = update_size / BLOCK_SIZE +
                            ((update_size % BLOCK_SIZE) ? 1 : 0);

                    for (size_t i = 0; i < n_blocks; ++i)
                    {
                        wdgReset(&WDGD1);

                        UINT this_block_size = 0;
                        uint8_t buffer[BLOCK_SIZE] =
                                { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

                        result = f_read(&fwufile, buffer, BLOCK_SIZE, &this_block_size);
                        if (result != FR_OK)
                            halt_error(1);

                        if (nvmWrite(PARTITION_FW, i * BLOCK_SIZE, BLOCK_SIZE, buffer) != HAL_SUCCESS)
                            halt_error(2);

                    }

                    result = f_close(&fwufile);
                    if (result != FR_OK)
                        halt_error(3);

                    result = f_unlink("firmware.bin");
                    if (result != FR_OK)
                        halt_error(4);

                    ledOff(LED_READ);
                }
            }

            result = f_mount(NULL, "", 1);
            if (result != FR_OK)
                halt_error(5);
        }
        sdcDisconnect(&SDCD1);
    }

    ledOff(LED_STATUS);

    boardStop();
    chSysDisable();

    boardJumpToApplication(FW_ORIGIN);
}
