/*
 * Rockchip App
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#define _ID_TIMER_MAIN                  100
#define _ID_TIMER_DESKTOP               101
#define _ID_TIMER_AUDIOPLAY             102
#define _ID_TIMER_BROWSER               103
#define _ID_TIMER_PICPREVIEW            104
#define _ID_TIMER_VIDEOPLAY             105
#define _ID_TIMER_SETTING               106
#define _ID_TIMER_SETTING_VERSION       107
#define _ID_TIMER_SETTING_LANGUAGE      108
#define _ID_TIMER_SETTING_EQ            109
#define _ID_TIMER_SETTING_BACKLIGHT     110
#define _ID_TIMER_SETTING_SCREENOFF     111
#define _ID_TIMER_VIDEOPLAY_HW          112
#define _ID_TIMER_SETTING_GAMEDISP      113
#define _ID_TIMER_SETTING_THEMESTYLE    114
#define _ID_TIMER_LOWPOWER              115

#define MSG_VIDEOPLAY_END         (MSG_USER + 1)
#define MSG_MEDIA_UPDATE          (MSG_USER + 2)

//500ms
#define TIMER_MAIN                  100
#define TIMER_DESKTOP               50
#define TIMER_AUDIOPLAY             50
#define TIMER_BROWSER               50
#define TIMER_PICPREVIEW            5
#define TIMER_VIDEOPLAY             50
#define TIMER_SETTING               50
#define TIMER_SETTING_VERSION       50
#define TIMER_SETTING_LANGUAGE      50
#define TIMER_SETTING_EQ            50
#define TIMER_SETTING_BACKLIGHT     50
#define TIMER_SETTING_SCREENOFF     50
#define TIMER_VIDEOPLAY_HW          50
#define TIMER_SETTING_GAMEDISP      50
#define TIMER_SETTING_THEMESTYLE    50
#define TIMER_LOWPOWER              50

enum RES_STR_ID {
    RES_STR_RES = 0,
    RES_STR_TITLE_GAME,
    RES_STR_TITLE_MUSIC,
    RES_STR_TITLE_PIC,
    RES_STR_TITLE_VIDEO,
    RES_STR_TITLE_BROWSER,
    RES_STR_TITLE_SETTING,
    RES_STR_TITLE_LANGUAGE,
    RES_STR_TITLE_GAMEDISP,
    RES_STR_TITLE_THEMESTYLE,
    RES_STR_TITLE_SCREENOFF,
    RES_STR_TITLE_BACKLIGHT,
    RES_STR_TITLE_RESTORE,
    RES_STR_TITLE_INFO,
    RES_STR_LANGUAGE_CN,
    RES_STR_LANGUAGE_EN,
    RES_STR_LANGUAGE_JP,
    RES_STR_LANGUAGE_KO,
    RES_STR_EQ_1,
    RES_STR_EQ_2,
    RES_STR_EQ_3,
    RES_STR_EQ_4,
    RES_STR_EQ_5,
    RES_STR_SCREENOFF_1,
    RES_STR_SCREENOFF_2,
    RES_STR_SCREENOFF_3,
    RES_STR_SCREENOFF_4,
    RES_STR_SCREENOFF_5,
    RES_STR_SCREENOFF_6,
    RES_STR_BACKLIGHT_1,
    RES_STR_BACKLIGHT_2,
    RES_STR_BACKLIGHT_3,
    RES_STR_BACKLIGHT_4,
    RES_STR_INFO_MODEL,
    RES_STR_INFO_UDISKCAP,
    RES_STR_INFO_UDISKAVACAP,
    RES_STR_INFO_VERSION,
    RES_STR_TITLE_GAMEDISP_FULL,
    RES_STR_TITLE_GAMEDISP_EQUAL,
    RES_STR_WARNING,
    RES_STR_WARNING_RECOVERY,
    RES_STR_YES,
    RES_STR_NO,
    RES_STR_OK,
    RES_STR_CANCEL,
    RES_STR_GAME_1,
    RES_STR_GAME_2,
    RES_STR_GAME_3,
    RES_STR_GAME_4,
    RES_STR_GAME_5,
    RES_STR_GAME_6,
    RES_STR_THEMESTYLE_THEME1,
    RES_STR_THEMESTYLE_THEME2,
    RES_STR_LOWPOWER,
    RES_STR_MAX
};

enum filter_filetype {
    FILTER_FILE_NO = 0,
    FILTER_FILE_MUSIC = 1,
    FILTER_FILE_GAME = 2,
    FILTER_FILE_PIC = 3,
    FILTER_FILE_ZIP = 4,
    FILTER_FILE_VIDEO = 5
};

enum filetype {
    FILE_FOLDER = 0,
    FILE_MUSIC = 1,
    FILE_GAME = 2,
    FILE_PIC = 3,
    FILE_ZIP = 4,
    FILE_VIDEO = 5,
    FILE_OTHER = 6,
    FILE_TYPE_MAX
};

enum languagetype {
    LANGUAGE_CH = 0,
    LANGUAGE_EN,
    LANGUAGE_JA,
    LANGUAGE_KO,
    LANGUAGE_MAX
};

struct file_node
{
    struct file_node *pre_node;
    struct file_node *next_node;
    char *name;
    int type;
};

struct directory_node
{
    struct directory_node *pre_node;
    struct directory_node *next_node;
    struct file_node *file_node_list;
    char *patch;
    int total;
    int file_sel;
};

enum MEDIA_CMD {
    MEDIA_CMD_CUR_TIME = 0,
    MEDIA_CMD_TOTAL_TIME,
    MEDIA_CMD_END,
    MEDIA_CMD_QUIT,
    MEDIA_CMD_READY
};

#if 1
#define BROWSER_PATH_ROOT     "/oem/file"
#define BROWSER_PATH_PIC      "/oem/file/pic"
#define BROWSER_PATH_MUSIC    "/oem/file/music"
#define BROWSER_PATH_GAME     "/oem/file/game"
#define BROWSER_PATH_VIDEO    "/oem/file/video"
#else
#define BROWSER_PATH_ROOT     "/sdcard"
#define BROWSER_PATH_PIC      "/sdcard/pic"
#define BROWSER_PATH_MUSIC    "/sdcard/music"
#define BROWSER_PATH_GAME     "/sdcard/game"
#define BROWSER_PATH_VIDEO    "/sdcard/video"
#endif

#define REC_FILE_CN    "/usr/local/share/minigui/res/string/CN-UTF8.bin"
#define REC_FILE_EN    "/usr/local/share/minigui/res/string/EN-UTF8.bin"
#define REC_FILE_JA    "/usr/local/share/minigui/res/string/JP-UTF8.bin"
#define REC_FILE_KO    "/usr/local/share/minigui/res/string/KO-UTF8.bin"

#define VERSION_FILE   "/etc/version"
#define DEFRETROARCH   "/data/retroarch/retroarch.cfg"
#define DEFRETROARCHNAME   ".cfg"
        
//#include "ui_1024x600.h"
//#include "ui_480x320.h"
#include "ui_480x272.h"
#include "key_map_rk3128.h"

#include "parameter.h"
#include "desktop_dialog.h"
#include "pic_preview_dialog.h"
#include "browser_dialog.h"

#ifdef ENABLE_VIDEO
#include "videoplay_dialog.h"
#endif

#include "audioplay_dialog.h"
#include "setting_dialog.h"
#include "setting_language_dialog.h"
#include "setting_eq_dialog.h"
#include "setting_screenoff_dialog.h"
#include "setting_backlight_dialog.h"
#include "setting_version_dialog.h"
#include "setting_gamedisp_dialog.h"
#include "setting_themestyle_dialog.h"
#include "message_dialog.h"
#include "videoplay_hw_dialog.h"
#include "system.h"
#include "ffplay_ipc.h"
#include "lowpower_dialog.h"

extern int loadstringres(void);
extern int loadversion(char **model, char **version);
extern int main_loadres(void);
extern void main_unloadres(void);
extern void DisableScreenAutoOff(void);
extern void EnableScreenAutoOff(void);

extern BITMAP batt_bmap[6];
extern int battery;
extern BITMAP background_bmap;
extern RECT msg_rcBg;
extern RECT msg_rcBatt;
extern RECT msg_rcTitle;
extern RECT msg_rcDialog;
extern char *res_str[RES_STR_MAX];
extern LOGFONT *logfont_cej;
extern LOGFONT *logfont_k;
extern LOGFONT *logfont;
#endif
