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
static char *model = 0;
static char *verstion = 0;
static char *model_disp = 0;
static char *verstion_disp = 0;

static int loadres(void)
{
    int len;

    loadversion(&model, &verstion);
    if (model) {
        len = strlen(model) + strlen(res_str[RES_STR_INFO_MODEL]) + 2;

        model_disp = malloc(len);
        snprintf(model_disp, len, "%s%s", res_str[RES_STR_INFO_MODEL], model);
        free(model);
        model = 0;
    }
    if (verstion) {
        len = strlen(verstion) + strlen(res_str[RES_STR_INFO_VERSION]) + 2;
        verstion_disp = malloc(len);
        snprintf(verstion_disp, len, "%s%s", res_str[RES_STR_INFO_VERSION], verstion);
        free(verstion);
        verstion = 0;
    }
    return 0;
}

static void unloadres(void)
{
    if (model_disp)
        free(model_disp);
    model_disp = 0;

    if (verstion_disp)
        free(verstion_disp);
    verstion_disp = 0;
}

static LRESULT setting_version_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        SetTimer(hWnd, _ID_TIMER_SETTING_VERSION, TIMER_SETTING_VERSION);
        return 0;
    }
    case MSG_TIMER: {
        if (wParam == _ID_TIMER_SETTING_VERSION) {
            if (batt != battery) {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcBatt, TRUE);
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
        SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
        SelectFont(hdc, logfont);
        DrawText(hdc, res_str[RES_STR_TITLE_INFO], -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, TITLE_LINE_PINT_X, TITLE_LINE_PINT_Y, TITLE_LINE_PINT_W, TITLE_LINE_PINT_H);

        msg_rcInfo.left = SETTING_INFO_PINT_X;
        msg_rcInfo.top = SETTING_INFO_PINT_Y;
        msg_rcInfo.right = LCD_W - msg_rcInfo.left;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        if (model_disp)
            DrawText(hdc, model_disp, -1, &msg_rcInfo, DT_TOP);

        msg_rcInfo.top += SETTING_INFO_PINT_SPAC;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        if (verstion_disp)
            DrawText(hdc, verstion_disp, -1, &msg_rcInfo, DT_TOP);
/*
        msg_rcInfo.top += SETTING_INFO_PINT_SPAC;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        DrawText(hdc, res_str[RES_STR_INFO_UDISKCAP], -1, &msg_rcInfo, DT_TOP);

        msg_rcInfo.top += SETTING_INFO_PINT_SPAC;
        msg_rcInfo.bottom = msg_rcInfo.top + SETTING_INFO_PINT_H;
        DrawText(hdc, res_str[RES_STR_INFO_UDISKAVACAP], -1, &msg_rcInfo, DT_TOP);
*/
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
        KillTimer(hWnd, _ID_TIMER_SETTING_VERSION);
        unloadres();
        break;
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_setting_version_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
    	                        0, 0,
    	                        LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0};
    //DesktopDlg.controls = DesktopCtrl;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, setting_version_dialog_proc, 0L);
}
