/*
 * This is a every simple sample for MiniGUI.
 * It will create a main window and display a string of "Hello, world!" in it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> 
#include <math.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

static int batt = 0;
static int autopoweroff_cnt = 0;
static int runlowpower = 0;

static int loadres(void)
{
    return 0;
}

static void unloadres(void)
{
}

static void poweroff(void)
{
    printf("%s\n", __func__);
    system("halt");
}

static LRESULT dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
    switch (message) {
    case MSG_INITDIALOG: {
    	  DWORD bkcolor;
        HWND hFocus = GetDlgDefPushButton(hWnd);
        loadres();
        bkcolor = GetWindowElementPixel(hWnd, WE_BGC_WINDOW);
        SetWindowBkColor(hWnd, bkcolor);
        if (hFocus)
            SetFocus(hFocus);
        batt = battery;
        autopoweroff_cnt = 0;
        SetTimer(hWnd, _ID_TIMER_LOWPOWER, TIMER_LOWPOWER);
        return 0;
    }
    case MSG_TIMER: {
        if (wParam == _ID_TIMER_LOWPOWER) {
            if (batt != battery) {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcBatt, TRUE);
            }
            if (autopoweroff_cnt < 4) {
                autopoweroff_cnt ++;
                printf("autopoweroff_cnt = %d\n", autopoweroff_cnt);
                if (autopoweroff_cnt >= 4)
                    poweroff();
            }
        }
        break;
    }
    case MSG_PAINT:
    {
        int i;
        int page;
        struct file_node *file_node_temp;
        gal_pixel old_brush;
        RECT msg_rcInfo;
        gal_pixel pixle = 0xffffffff;

        hdc = BeginPaint(hWnd);
        old_brush = SetBrushColor(hdc, pixle);
        FillBoxWithBitmap(hdc, BG_PINT_X,
                               BG_PINT_Y, BG_PINT_W,
                               BG_PINT_H, &background_bmap);
        FillBoxWithBitmap(hdc, BATT_PINT_X, BATT_PINT_Y,
                               BATT_PINT_W, BATT_PINT_H,
                               &batt_bmap[batt]);

        SetBkColor(hdc, COLOR_transparent);
        SetBkMode(hdc,BM_TRANSPARENT);
        SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0, 0));
        SelectFont(hdc, logfont);
        msg_rcInfo.left = 0;
        msg_rcInfo.top = LCD_H / 2;
        msg_rcInfo.right = LCD_W;
        msg_rcInfo.bottom = msg_rcInfo.top + 24;
        DrawText(hdc, res_str[RES_STR_LOWPOWER], -1, &msg_rcInfo, DT_TOP | DT_CENTER);

        SetBrushColor(hdc, old_brush);
        EndPaint(hWnd, hdc);
        break;
    }
    case MSG_KEYDOWN:
        //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
        switch (wParam) {
            case KEY_EXIT_FUNC:
                EndDialog(hWnd, wParam);
                break;
            case KEY_UP_FUNC:
                break;
            case KEY_DOWN_FUNC:
                break;
            case KEY_ENTER_FUNC:
                break;
        }
        break;
    case MSG_COMMAND: {
        break;
    }
    case MSG_DESTROY:
        KillTimer(hWnd, _ID_TIMER_LOWPOWER);
        unloadres();
        break;
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_lowpower_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
    	                        0, 0,
    	                        LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0};
    //DesktopDlg.controls = DesktopCtrl;
    if (runlowpower)
        return;
    runlowpower = 1;
    DialogBoxIndirectParam(&DesktopDlg, hWnd, dialog_proc, 0L);
}
