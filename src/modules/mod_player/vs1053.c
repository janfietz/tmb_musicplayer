/**
 * @file    vs1053.c
 * @brief   VS1053 Driver code.
 *
 * @addtogroup VS1053
 * @{
 */

#include "hal.h"
#include "vs1053.h"
#include <string.h>

#if HAL_USE_VS1053 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03

/*SCI Registers*/
#define SCI_MODE        	0x00
#define SCI_STATUS      	0x01
#define SCI_BASS        	0x02
#define SCI_CLOCKF      	0x03
#define SCI_DECODE_TIME 	0x04
#define SCI_AUDATA      	0x05
#define SCI_WRAM        	0x06
#define SCI_WRAMADDR    	0x07
#define SCI_HDAT0       	0x08
#define SCI_HDAT1       	0x09
#define SCI_AIADDR      	0x0a
#define SCI_VOL         	0x0b
#define SCI_AICTRL0     	0x0c
#define SCI_AICTRL1     	0x0d
#define SCI_AICTRL2     	0x0e
#define SCI_AICTRL3     	0x0f

/* SCI_MODE Register bits */
#define SM_DIFF         	0x0001
#define SM_LAYER12			0x0002
#define SM_RESET        	0x0004
#define SM_CANCEL           0x0008
#define SM_PDOWN        	0x0010
#define SM_TESTS        	0x0020
#define SM_STREAM       	0x0040
#define SM_PLUSV        	0x0080
#define SM_DACT         	0x0100
#define SM_SDIORD       	0x0200
#define SM_SDISHARE     	0x0400
#define SM_SDINEW       	0x0800
#define SM_ADPCM        	0x1000
#define SM_ADPCM_HP     	0x2000
#define SM_LINE1            0x4000

/*Common Parameter Addresses*/
#define PARA_endFillByte    0x1E06

#define SPECTRUM_ANALYZERPLUGIN_BASEADRESS 0x1800

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/
#define PLUGIN_SIZE 1000
static const unsigned short plugin[1000] = { /* Compressed plugin */
  0x0007, 0x0001, 0x8d00, 0x0006, 0x0004, 0x2803, 0x5b40, 0x0000, /*    0 */
  0x0024, 0x0007, 0x0001, 0x8d02, 0x0006, 0x00d6, 0x3e12, 0xb817, /*    8 */
  0x3e12, 0x3815, 0x3e05, 0xb814, 0x3615, 0x0024, 0x0000, 0x800a, /*   10 */
  0x3e10, 0x3801, 0x0006, 0x0800, 0x3e10, 0xb803, 0x0000, 0x0303, /*   18 */
  0x3e11, 0x3805, 0x3e11, 0xb807, 0x3e14, 0x3812, 0xb884, 0x130c, /*   20 */
  0x3410, 0x4024, 0x4112, 0x10d0, 0x4010, 0x008c, 0x4010, 0x0024, /*   28 */
  0xf400, 0x4012, 0x3000, 0x3840, 0x3009, 0x3801, 0x0000, 0x0041, /*   30 */
  0xfe02, 0x0024, 0x2903, 0xb480, 0x48b2, 0x0024, 0x36f3, 0x0844, /*   38 */
  0x6306, 0x8845, 0xae3a, 0x8840, 0xbf8e, 0x8b41, 0xac32, 0xa846, /*   40 */
  0xffc8, 0xabc7, 0x3e01, 0x7800, 0xf400, 0x4480, 0x6090, 0x0024, /*   48 */
  0x6090, 0x0024, 0xf400, 0x4015, 0x3009, 0x3446, 0x3009, 0x37c7, /*   50 */
  0x3009, 0x1800, 0x3009, 0x3844, 0x48b3, 0xe1e0, 0x4882, 0x4040, /*   58 */
  0xfeca, 0x0024, 0x5ac2, 0x0024, 0x5a52, 0x0024, 0x4cc2, 0x0024, /*   60 */
  0x48ba, 0x4040, 0x4eea, 0x4801, 0x4eca, 0x9800, 0xff80, 0x1bc1, /*   68 */
  0xf1eb, 0xe3e2, 0xf1ea, 0x184c, 0x4c8b, 0xe5e4, 0x48be, 0x9804, /*   70 */
  0x488e, 0x41c6, 0xfe82, 0x0024, 0x5a8e, 0x0024, 0x525e, 0x1b85, /*   78 */
  0x4ffe, 0x0024, 0x48b6, 0x41c6, 0x4dd6, 0x48c7, 0x4df6, 0x0024, /*   80 */
  0xf1d6, 0x0024, 0xf1d6, 0x0024, 0x4eda, 0x0024, 0x0000, 0x0fc3, /*   88 */
  0x2903, 0xb480, 0x4e82, 0x0024, 0x4084, 0x130c, 0x0006, 0x0500, /*   90 */
  0x3440, 0x4024, 0x4010, 0x0024, 0xf400, 0x4012, 0x3200, 0x4024, /*   98 */
  0xb132, 0x0024, 0x4214, 0x0024, 0xf224, 0x0024, 0x6230, 0x0024, /*   a0 */
  0x0001, 0x0001, 0x2803, 0x54c9, 0x0000, 0x0024, 0xf400, 0x40c2, /*   a8 */
  0x3200, 0x0024, 0xff82, 0x0024, 0x48b2, 0x0024, 0xb130, 0x0024, /*   b0 */
  0x6202, 0x0024, 0x003f, 0xf001, 0x2803, 0x57d1, 0x0000, 0x1046, /*   b8 */
  0xfe64, 0x0024, 0x48be, 0x0024, 0x2803, 0x58c0, 0x3a01, 0x8024, /*   c0 */
  0x3200, 0x0024, 0xb010, 0x0024, 0xc020, 0x0024, 0x3a00, 0x0024, /*   c8 */
  0x36f4, 0x1812, 0x36f1, 0x9807, 0x36f1, 0x1805, 0x36f0, 0x9803, /*   d0 */
  0x36f0, 0x1801, 0x3405, 0x9014, 0x36f3, 0x0024, 0x36f2, 0x1815, /*   d8 */
  0x2000, 0x0000, 0x36f2, 0x9817, 0x0007, 0x0001, 0x8d6d, 0x0006, /*   e0 */
  0x01f6, 0x3613, 0x0024, 0x3e12, 0xb817, 0x3e12, 0x3815, 0x3e05, /*   e8 */
  0xb814, 0x3645, 0x0024, 0x0000, 0x800a, 0x3e10, 0xb803, 0x3e11, /*   f0 */
  0x3805, 0x3e11, 0xb811, 0x3e14, 0xb813, 0x3e13, 0xf80e, 0x4182, /*   f8 */
  0x384d, 0x0006, 0x0912, 0x2803, 0x6105, 0x0006, 0x0451, 0x0006, /*  100 */
  0xc352, 0x3100, 0x8803, 0x6238, 0x1bcc, 0x0000, 0x0024, 0x2803, /*  108 */
  0x7705, 0x4194, 0x0024, 0x0006, 0x0912, 0x3613, 0x0024, 0x0006, /*  110 */
  0x0411, 0x0000, 0x0302, 0x3009, 0x3850, 0x0006, 0x0410, 0x3009, /*  118 */
  0x3840, 0x0000, 0x1100, 0x2914, 0xbec0, 0xb882, 0xb801, 0x0000, /*  120 */
  0x1000, 0x0006, 0x0810, 0x2915, 0x7ac0, 0xb882, 0x0024, 0x3900, /*  128 */
  0x9bc1, 0x0006, 0xc351, 0x3009, 0x1bc0, 0x3009, 0x1bd0, 0x3009, /*  130 */
  0x0404, 0x0006, 0x0451, 0x2803, 0x66c0, 0x3901, 0x0024, 0x4448, /*  138 */
  0x0402, 0x4294, 0x0024, 0x6498, 0x2402, 0x001f, 0x4002, 0x6424, /*  140 */
  0x0024, 0x0006, 0x0411, 0x2803, 0x6611, 0x0000, 0x03ce, 0x2403, /*  148 */
  0x764e, 0x0000, 0x0013, 0x0006, 0x1a04, 0x0006, 0x0451, 0x3100, /*  150 */
  0x8024, 0xf224, 0x44c5, 0x4458, 0x0024, 0xf400, 0x4115, 0x3500, /*  158 */
  0xc024, 0x623c, 0x0024, 0x0000, 0x0024, 0x2803, 0x7691, 0x0000, /*  160 */
  0x0024, 0x4384, 0x184c, 0x3100, 0x3800, 0x2915, 0x7dc0, 0xf200, /*  168 */
  0x0024, 0x003f, 0xfec3, 0x4084, 0x4491, 0x3113, 0x1bc0, 0xa234, /*  170 */
  0x0024, 0x0000, 0x2003, 0x6236, 0x2402, 0x0000, 0x1003, 0x2803, /*  178 */
  0x6fc8, 0x0000, 0x0024, 0x003f, 0xf803, 0x3100, 0x8024, 0xb236, /*  180 */
  0x0024, 0x2803, 0x75c0, 0x3900, 0xc024, 0x6236, 0x0024, 0x0000, /*  188 */
  0x0803, 0x2803, 0x7208, 0x0000, 0x0024, 0x003f, 0xfe03, 0x3100, /*  190 */
  0x8024, 0xb236, 0x0024, 0x2803, 0x75c0, 0x3900, 0xc024, 0x6236, /*  198 */
  0x0024, 0x0000, 0x0403, 0x2803, 0x7448, 0x0000, 0x0024, 0x003f, /*  1a0 */
  0xff03, 0x3100, 0x8024, 0xb236, 0x0024, 0x2803, 0x75c0, 0x3900, /*  1a8 */
  0xc024, 0x6236, 0x0402, 0x003f, 0xff83, 0x2803, 0x75c8, 0x0000, /*  1b0 */
  0x0024, 0xb236, 0x0024, 0x3900, 0xc024, 0xb884, 0x07cc, 0x3900, /*  1b8 */
  0x88cc, 0x3313, 0x0024, 0x0006, 0x0491, 0x4194, 0x2413, 0x0006, /*  1c0 */
  0x04d1, 0x2803, 0x9755, 0x0006, 0x0902, 0x3423, 0x0024, 0x3c10, /*  1c8 */
  0x8024, 0x3100, 0xc024, 0x4304, 0x0024, 0x39f0, 0x8024, 0x3100, /*  1d0 */
  0x8024, 0x3cf0, 0x8024, 0x0006, 0x0902, 0xb884, 0x33c2, 0x3c20, /*  1d8 */
  0x8024, 0x34d0, 0xc024, 0x6238, 0x0024, 0x0000, 0x0024, 0x2803, /*  1e0 */
  0x8dd8, 0x4396, 0x0024, 0x2403, 0x8d83, 0x0000, 0x0024, 0x3423, /*  1e8 */
  0x0024, 0x34e4, 0x4024, 0x3123, 0x0024, 0x3100, 0xc024, 0x4304, /*  1f0 */
  0x0024, 0x4284, 0x2402, 0x0000, 0x2003, 0x2803, 0x8b89, 0x0000, /*  1f8 */
  0x0024, 0x3423, 0x184c, 0x34f4, 0x4024, 0x3004, 0x844c, 0x3100, /*  200 */
  0xb850, 0x6236, 0x0024, 0x0006, 0x0802, 0x2803, 0x81c8, 0x4088, /*  208 */
  0x1043, 0x4336, 0x1390, 0x4234, 0x0024, 0x4234, 0x0024, 0xf400, /*  210 */
  0x4091, 0x2903, 0xa480, 0x0003, 0x8308, 0x4336, 0x1390, 0x4234, /*  218 */
  0x0024, 0x4234, 0x0024, 0x2903, 0x9a00, 0xf400, 0x4091, 0x0004, /*  220 */
  0x0003, 0x3423, 0x1bd0, 0x3404, 0x4024, 0x3123, 0x0024, 0x3100, /*  228 */
  0x8024, 0x6236, 0x0024, 0x0000, 0x4003, 0x2803, 0x85c8, 0x0000, /*  230 */
  0x0024, 0xb884, 0x878c, 0x3900, 0x8024, 0x34e4, 0x4024, 0x3123, /*  238 */
  0x0024, 0x31e0, 0x8024, 0x6236, 0x0402, 0x0000, 0x0024, 0x2803, /*  240 */
  0x8b88, 0x4284, 0x0024, 0x0000, 0x0024, 0x2803, 0x8b95, 0x0000, /*  248 */
  0x0024, 0x3413, 0x184c, 0x3410, 0x8024, 0x3e10, 0x8024, 0x34e0, /*  250 */
  0xc024, 0x2903, 0x4080, 0x3e10, 0xc024, 0xf400, 0x40d1, 0x003f, /*  258 */
  0xff44, 0x36e3, 0x048c, 0x3100, 0x8024, 0xfe44, 0x0024, 0x48ba, /*  260 */
  0x0024, 0x3901, 0x0024, 0x0000, 0x00c3, 0x3423, 0x0024, 0xf400, /*  268 */
  0x4511, 0x34e0, 0x8024, 0x4234, 0x0024, 0x39f0, 0x8024, 0x3100, /*  270 */
  0x8024, 0x6294, 0x0024, 0x3900, 0x8024, 0x0006, 0x0411, 0x6894, /*  278 */
  0x04c3, 0xa234, 0x0403, 0x6238, 0x0024, 0x0000, 0x0024, 0x2803, /*  280 */
  0x9741, 0x0000, 0x0024, 0xb884, 0x90cc, 0x39f0, 0x8024, 0x3100, /*  288 */
  0x8024, 0xb884, 0x3382, 0x3c20, 0x8024, 0x34d0, 0xc024, 0x6238, /*  290 */
  0x0024, 0x0006, 0x0512, 0x2803, 0x9758, 0x4396, 0x0024, 0x2403, /*  298 */
  0x9703, 0x0000, 0x0024, 0x0003, 0xf002, 0x3201, 0x0024, 0xb424, /*  2a0 */
  0x0024, 0x0028, 0x0002, 0x2803, 0x9605, 0x6246, 0x0024, 0x0004, /*  2a8 */
  0x0003, 0x2803, 0x95c1, 0x4434, 0x0024, 0x0000, 0x1003, 0x6434, /*  2b0 */
  0x0024, 0x2803, 0x9600, 0x3a00, 0x8024, 0x3a00, 0x8024, 0x3213, /*  2b8 */
  0x104c, 0xf400, 0x4511, 0x34f0, 0x8024, 0x6294, 0x0024, 0x3900, /*  2c0 */
  0x8024, 0x36f3, 0x4024, 0x36f3, 0xd80e, 0x36f4, 0x9813, 0x36f1, /*  2c8 */
  0x9811, 0x36f1, 0x1805, 0x36f0, 0x9803, 0x3405, 0x9014, 0x36f3, /*  2d0 */
  0x0024, 0x36f2, 0x1815, 0x2000, 0x0000, 0x36f2, 0x9817, 0x0007, /*  2d8 */
  0x0001, 0x1868, 0x0006, 0x0010, 0x0032, 0x004f, 0x007e, 0x00c8, /*  2e0 */
  0x013d, 0x01f8, 0x0320, 0x04f6, 0x07e0, 0x0c80, 0x13d8, 0x1f7f, /*  2e8 */
  0x3200, 0x4f5f, 0x61a8, 0x0000, 0x0007, 0x0001, 0x8e68, 0x0006, /*  2f0 */
  0x0054, 0x3e12, 0xb814, 0x0000, 0x800a, 0x3e10, 0x3801, 0x3e10, /*  2f8 */
  0xb803, 0x3e11, 0x7806, 0x3e11, 0xf813, 0x3e13, 0xf80e, 0x3e13, /*  300 */
  0x4024, 0x3e04, 0x7810, 0x449a, 0x0040, 0x0001, 0x0003, 0x2803, /*  308 */
  0xa344, 0x4036, 0x03c1, 0x0003, 0xffc2, 0xb326, 0x0024, 0x0018, /*  310 */
  0x0042, 0x4326, 0x4495, 0x4024, 0x40d2, 0x0000, 0x0180, 0xa100, /*  318 */
  0x4090, 0x0010, 0x0fc2, 0x4204, 0x0024, 0xbc82, 0x4091, 0x459a, /*  320 */
  0x0024, 0x0000, 0x0054, 0x2803, 0xa244, 0xbd86, 0x4093, 0x2403, /*  328 */
  0xa205, 0xfe01, 0x5e0c, 0x5c43, 0x5f2d, 0x5e46, 0x020c, 0x5c56, /*  330 */
  0x8a0c, 0x5e53, 0x5e0c, 0x5c43, 0x5f2d, 0x5e46, 0x020c, 0x5c56, /*  338 */
  0x8a0c, 0x5e52, 0x0024, 0x4cb2, 0x4405, 0x0018, 0x0044, 0x654a, /*  340 */
  0x0024, 0x2803, 0xb040, 0x36f4, 0x5810, 0x0007, 0x0001, 0x8e92, /*  348 */
  0x0006, 0x0080, 0x3e12, 0xb814, 0x0000, 0x800a, 0x3e10, 0x3801, /*  350 */
  0x3e10, 0xb803, 0x3e11, 0x7806, 0x3e11, 0xf813, 0x3e13, 0xf80e, /*  358 */
  0x3e13, 0x4024, 0x3e04, 0x7810, 0x449a, 0x0040, 0x0000, 0x0803, /*  360 */
  0x2803, 0xaf04, 0x30f0, 0x4024, 0x0fff, 0xfec2, 0xa020, 0x0024, /*  368 */
  0x0fff, 0xff02, 0xa122, 0x0024, 0x4036, 0x0024, 0x0000, 0x1fc2, /*  370 */
  0xb326, 0x0024, 0x0010, 0x4002, 0x4326, 0x4495, 0x4024, 0x40d2, /*  378 */
  0x0000, 0x0180, 0xa100, 0x4090, 0x0010, 0x0042, 0x4204, 0x0024, /*  380 */
  0xbc82, 0x4091, 0x459a, 0x0024, 0x0000, 0x0054, 0x2803, 0xae04, /*  388 */
  0xbd86, 0x4093, 0x2403, 0xadc5, 0xfe01, 0x5e0c, 0x5c43, 0x5f2d, /*  390 */
  0x5e46, 0x0024, 0x5c56, 0x0024, 0x5e53, 0x5e0c, 0x5c43, 0x5f2d, /*  398 */
  0x5e46, 0x0024, 0x5c56, 0x0024, 0x5e52, 0x0024, 0x4cb2, 0x4405, /*  3a0 */
  0x0010, 0x4004, 0x654a, 0x9810, 0x0000, 0x0144, 0xa54a, 0x1bd1, /*  3a8 */
  0x0006, 0x0413, 0x3301, 0xc444, 0x687e, 0x2005, 0xad76, 0x8445, /*  3b0 */
  0x4ed6, 0x8784, 0x36f3, 0x64c2, 0xac72, 0x8785, 0x4ec2, 0xa443, /*  3b8 */
  0x3009, 0x2440, 0x3009, 0x2741, 0x36f3, 0xd80e, 0x36f1, 0xd813, /*  3c0 */
  0x36f1, 0x5806, 0x36f0, 0x9803, 0x36f0, 0x1801, 0x2000, 0x0000, /*  3c8 */
  0x36f2, 0x9814, 0x0007, 0x0001, 0x8ed2, 0x0006, 0x000e, 0x4c82, /*  3d0 */
  0x0024, 0x0000, 0x0024, 0x2000, 0x0005, 0xf5c2, 0x0024, 0x0000, /*  3d8 */
  0x0980, 0x2000, 0x0000, 0x6010, 0x0024, 0x000a, 0x0001, 0x0d00,
};

//static const int16_t spectrumCenterFrequencies[] = {
//        105,
//        340,
//        1000,
//        3400,
//        10500,
//};

static const int16_t spectrumCenterFrequencies[] = {
        100,
        250,
        440,
        1000,
        10000,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static bool ReadDREQ(VS1053Driver* VS1053p)
{
    return palReadPad(VS1053p->config->xDREQPort, VS1053p->config->xDREQPad);
}

static bool ResetChip(VS1053Driver* VS1053p)
{
    palClearPad(VS1053p->config->xResetPort, VS1053p->config->xResetPad);

    chThdSleep(MS2ST(10));

    palSetPad(VS1053p->config->xResetPort, VS1053p->config->xResetPad);

    chThdSleep(MS2ST(5));

    if (ReadDREQ(VS1053p) == false)
    {
        return false;
    }
    return true;
}

static void ActivateSCI(VS1053Driver* VS1053p)
{
    palSetPad(VS1053p->config->xDCSPort, VS1053p->config->xDCSPad);
    palClearPad(VS1053p->config->xCSPort, VS1053p->config->xCSPad);
}

static void DeactivateSCI(VS1053Driver* VS1053p)
{
    palSetPad(VS1053p->config->xCSPort, VS1053p->config->xCSPad);
    palClearPad(VS1053p->config->xDCSPort, VS1053p->config->xDCSPad);
}

static void WriteRegister(VS1053Driver* VS1053p, uint8_t addressbyte, uint8_t highbyte, uint8_t lowbyte)
{
    VS1053p->state = VS1053_SCI_TRANSFER;
    ActivateSCI(VS1053p);
    spiSelect(VS1053p->config->spid);
    VS1053p->txBuffer[0] = VS_WRITE_COMMAND;
    VS1053p->txBuffer[1] = addressbyte;
    VS1053p->txBuffer[2] = highbyte;
    VS1053p->txBuffer[3] = lowbyte;
    spiSend(VS1053p->config->spid, 4, VS1053p->txBuffer);
    DeactivateSCI(VS1053p);

    while (ReadDREQ(VS1053p) == false)
    {
    }
    VS1053p->state = VS1053_ACTIVE;
}

static void WriteWordRegister(VS1053Driver* VS1053p, uint8_t address, uint16_t data)
{
    WriteRegister(VS1053p, address,  0x00ff & (data >> 8), 0x00ff & data);
}

static uint16_t ReadRegister(VS1053Driver* VS1053p, uint8_t addressbyte)
{
    VS1053p->state = VS1053_SCI_TRANSFER;
    ActivateSCI(VS1053p);

    spiSelect(VS1053p->config->spid);
    VS1053p->txBuffer[0] = VS_READ_COMMAND;
    VS1053p->txBuffer[1] = addressbyte;
    VS1053p->txBuffer[2] = 0;
    VS1053p->txBuffer[3] = 0;
    spiExchange(VS1053p->config->spid, 4, VS1053p->txBuffer, VS1053p->rxBuffer);
    spiUnselect(VS1053p->config->spid);

    DeactivateSCI(VS1053p);
    VS1053p->state = VS1053_ACTIVE;

    uint16_t result = VS1053p->rxBuffer[2] << 8;
    result |= VS1053p->rxBuffer[3];

    return result;
}

static void SoftReset(VS1053Driver* VS1053p)
{
    WriteRegister(VS1053p, SCI_MODE, SM_SDINEW>>8, SM_RESET);
}

static void SetVolume(VS1053Driver* VS1053p, uint8_t left, uint8_t right)
{
    WriteRegister(VS1053p, SCI_VOL, left,  right);
}

static uint8_t ReadEndFillByte(VS1053Driver* VS1053p)
{
    WriteRegister(VS1053p, SCI_WRAMADDR,(PARA_endFillByte & 0xFF00) >> 8, (PARA_endFillByte & 0x00FF));
    return (uint8_t)ReadRegister(VS1053p, SCI_WRAM) & 0xFF;
}

static void LoadUserCode(VS1053Driver* VS1053p) {
    int i = 0;
    int pluginWords = sizeof(plugin)/sizeof(plugin[0]);
    while (i < pluginWords) {
        uint16_t addr = plugin[i++];
        uint16_t n = plugin[i++];
        if (n & 0x8000U) { /* RLE run, replicate n samples */
            n &= 0x7FFF;
            uint16_t val = plugin[i++];
            while (n--) {
                WriteWordRegister(VS1053p, addr, val);
            }
        } else {           /* Copy run, copy n samples */
            while (n--) {
                uint16_t val = plugin[i++];
                WriteWordRegister(VS1053p, addr, val);
            }
        }
    }
}

static void InitSpectrumAnalyzerPlugin(VS1053Driver* VS1053p)
{
    /* Setting Bands
     *  You can also change the frequency band center frequencies and the number of bands
     *  to best suit your needs. The bands are in XRAM 0x1868..0x187e in ascending order. If
     *  not all 15 bands are used, end the list with 25000. To activate the new band selections,
     *  write 0 to the sample rate field (0x1811).
     */


    WriteWordRegister(VS1053p, SCI_WRAMADDR, 0x1868);
    /* write center frequencies */
    int32_t i;
    for (i = 0; i < VS1053_SPECTRUM_BANDS; i++) {
        WriteWordRegister(VS1053p, SCI_WRAM, spectrumCenterFrequencies[i]);
    }

    while (i < 15)
    {
        WriteWordRegister(VS1053p, SCI_WRAM, 25000);
        i++;
    }

    /* Reset sample rate field to activate new frequencies */
    WriteWordRegister(VS1053p, SCI_WRAMADDR, 0x1811);
    WriteWordRegister(VS1053p, SCI_WRAM, 0);
}

static bool InitChip(VS1053Driver* VS1053p)
{
    /*Set clock frequency*/
    WriteRegister(VS1053p, SCI_CLOCKF,0x60 | 0x08, 0x00);

    /*Set mode*/
    SoftReset(VS1053p);

    /* Load and configure Plugins */
    LoadUserCode(VS1053p);
    InitSpectrumAnalyzerPlugin(VS1053p);

    SetVolume(VS1053p, 50, 50);

    return true;
}
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   VS1053 Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void VS1053Init(void) {

}

/**
 * @brief   Initializes the standard part of a @p VS1053Driver structure.
 *
 * @param[out] VS1053p     pointer to the @p VS1053Driver object
 *
 * @init
 */
void VS1053ObjectInit(VS1053Driver* VS1053p) {

	VS1053p->state = VS1053_STOP;
	VS1053p->config = NULL;
}

/**
 * @brief   Configures and activates the VS1053 peripheral.
 *
 * @param[in] VS1053p      pointer to the @p VS1053Driver object
 * @param[in] config    pointer to the @p VS1053Config object
 *
 * @api
 */

void VS1053Start(VS1053Driver* VS1053p, const VS1053Config* config) {

	osalDbgCheck((VS1053p != NULL) && (config != NULL));

	osalSysLock();
	osalDbgAssert((VS1053p->state == VS1053_STOP) || (VS1053p->state == VS1053_READY),
			"invalid state");
	VS1053p->config = config;

	osalSysUnlock();

	if (ResetChip(VS1053p) == false)
	{
	    return;
	}

	DeactivateSCI(VS1053p);

	spiStart(config->spid, config->spiCfg);
	spiUnselect(VS1053p->config->spid);

	InitChip( VS1053p);

	osalSysLock();
	VS1053p->state = VS1053_ACTIVE;
	osalSysUnlock();
}

/**
 * @brief   Deactivates the VS1053 peripheral.
 *
 * @param[in] VS1053p      pointer to the @p VS1053Driver object
 *
 * @api
 */
void VS1053Stop(VS1053Driver* VS1053p) {

	osalDbgCheck(VS1053p != NULL);

	osalSysLock();
	osalDbgAssert(VS1053p->state == VS1053_ACTIVE, "invalid state");
	osalSysUnlock();

	palClearPad(VS1053p->config->xResetPort, VS1053p->config->xResetPad);

	spiStop(VS1053p->config->spid);

	osalSysLock();
	VS1053p->state = VS1053_STOP;
	osalSysUnlock();
}

void VS1053SineTest(VS1053Driver* VS1053p, uint16_t freq, uint8_t leftVol, uint8_t rightVol)
{
    osalDbgCheck(VS1053p != NULL);

    osalSysLock();
    osalDbgAssert(VS1053p->state == VS1053_ACTIVE, "invalid state");
    osalSysUnlock();

    /*Set sample rate*/
    WriteRegister(VS1053p, SCI_AUDATA, 0x45, 0xAC);

    /*Set volume*/
    WriteRegister(VS1053p, SCI_VOL, leftVol, rightVol);

    /*Set frequency*/
    WriteRegister(VS1053p, SCI_AICTRL0, 0x00ff & (freq >> 8), 0x00ff & freq);
    WriteRegister(VS1053p, SCI_AICTRL1, 0x00ff & (freq >> 8), 0x00ff & freq);

    /*Start test*/
    WriteRegister(VS1053p, SCI_AIADDR, 0x40, 0x20);
}

/*
 * Volume Control. 0 = Max, 254 is Silence, 255 Analog Power Down
 */
void VS1053SetVolume(VS1053Driver* VS1053p, uint8_t leftVol, uint8_t rightVol)
{
    osalDbgCheck(VS1053p != NULL);

    osalSysLock();
    osalDbgAssert(VS1053p->state == VS1053_ACTIVE, "invalid state");
    osalSysUnlock();

    /*Set volume*/
    SetVolume(VS1053p, leftVol, rightVol);
}

uint8_t VS1053SendData(VS1053Driver* VS1053p, const char* data, uint8_t bytes)
{
    osalSysLock();
    VS1053p->state = VS1053_SDI_TRANSFER;
    osalSysUnlock();

    while (ReadDREQ(VS1053p) == false)
    {
        chThdSleep(MS2ST(1));
    }

    DeactivateSCI(VS1053p);

    spiSelect(VS1053p->config->spid);
    spiSend(VS1053p->config->spid, bytes, data);
    spiUnselect(VS1053p->config->spid);

    ActivateSCI(VS1053p);

    osalSysLock();
    VS1053p->state = VS1053_ACTIVE;
    osalSysUnlock();

    return bytes;
}

void VS1053StopPlaying(VS1053Driver* VS1053p)
{
    uint8_t endFillByte = ReadEndFillByte(VS1053p);
    char buf[32];
    memset(buf, endFillByte, sizeof(buf));

    int8_t y;
    for (y = 0; y < 66; y++)
        VS1053SendData(VS1053p, buf, sizeof(buf)); // 66*32 > 2052

    WriteRegister(VS1053p, SCI_MODE, SM_SDINEW>>8, SM_CANCEL);

    uint16_t byteCount = 0;
    while (true)
    {
        VS1053SendData(VS1053p, buf, sizeof(buf));
        byteCount = byteCount+ sizeof(buf);
        uint16_t mode = ReadRegister(VS1053p, SCI_MODE);
        if ((mode & SM_CANCEL) == 0)
        {
            WriteRegister(VS1053p, SCI_MODE, SM_SDINEW>>8, SM_LAYER12);
            return;
        }
        if(byteCount > 2048)
        {
            break;
        }
    }

    SoftReset(VS1053p);
}

void VS1053ReadHeaderData(VS1053Driver* VS1053p, uint16_t* headerData0, uint16_t* headerData1)
{
    while (ReadDREQ(VS1053p) == false)
    {
        chThdSleep(MS2ST(1));
    }
    *headerData0 = ReadRegister(VS1053p, SCI_HDAT0);
    *headerData1 = ReadRegister(VS1053p, SCI_HDAT1);
}

void VS1053ReadSpectrumAnalyzerResult(VS1053Driver* VS1053p, struct VS1053SpectrumAnalyzerResult* result)
{
    WriteWordRegister(VS1053p, SCI_WRAMADDR, 0x1814);
    size_t i;
    for (i = 0; i < VS1053_SPECTRUM_BANDS; i++)
    {
        uint16_t val = ReadRegister(VS1053p, SCI_WRAM);
        result->current[i] = 0x3f & val;
        result->peak[i] = 0x3f & (val >> 6);
    }
}

uint16_t VS1053ReadStatus(VS1053Driver* VS1053p)
{
    while (ReadDREQ(VS1053p) == false)
    {
        chThdSleep(MS2ST(1));
    }
    return ReadRegister(VS1053p, SCI_STATUS);
}

uint16_t VS1053ReadSampleRate(VS1053Driver* VS1053p)
{
    while (ReadDREQ(VS1053p) == false)
    {
        chThdSleep(MS2ST(1));
    }
    return ReadRegister(VS1053p, SCI_AUDATA);
}


#endif /* HAL_USE_VS1053 */

/** @} */
