/*
 * Copyright (c) 2018 rockchip
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parameter.h"

#define PARAMETER_FILE    "/data/parameter"
#define VERSION          1002

#define LANGUAGE_DEF     0
#define SCREENOFF_DEF    3     //list value
#define SCREENOFF_VAL_DEF    30     //wait time
#define EQ_DEF           0
#define BACKLIGHT_DEF    0
#define GAMEDISP_DEF     0
#define THEMESTYLE_DEF   0

struct parameter_data
{
    int version;
    int language;
    int screenoff;
    int screenoff_val;
    int eq_val;
    int backlight_val;
    int gamedisp_val;
    int themestyle_val;
};

static struct parameter_data para_data;

#define UI_IMAGE_PATH_0         "/usr/local/share/minigui/res/images/"
#define UI_IMAGE_PATH_1         "/usr/local/share/minigui/res/images1/"

static void set_version(int val)
{
    para_data.version = val;
}

static int parameter_save(void)
{
    FILE * fpFile = 0;

    fpFile = fopen(PARAMETER_FILE, "wb+");
    if (fpFile <= 0) {
        printf("create parameter file fail\n");
        return -1;
    }
    fwrite(&para_data, 1, sizeof(struct parameter_data), fpFile);
    fflush(fpFile);
    fsync(fpFile);
    fclose(fpFile);
    system("sync &");

    return 0;
}

int parameter_init(void)
{
    FILE * fpFile = 0;

    memset(&para_data, 0, sizeof(struct parameter_data));
    fpFile = fopen(PARAMETER_FILE, "r");
    if (fpFile <= 0) {
        parameter_recovery();
        return 0;
    }
    if (fpFile > 0)
        fclose(fpFile);

    fpFile = fopen(PARAMETER_FILE, "rb+");
    if (fpFile <= 0) {
        printf("open parameter file fail\n");
        return -1;
    }
    fread(&para_data, 1, sizeof(struct parameter_data), fpFile);
    fclose(fpFile);

    if (para_data.version != VERSION) {
        parameter_recovery();
    }

    return 0;
}

void parameter_deinit(void)
{

}

int parameter_recovery(void)
{
    FILE * fpFile = 0;

    set_version(VERSION);
    set_language(LANGUAGE_DEF);
    set_screenoff(SCREENOFF_DEF);
    set_screenoff_val(SCREENOFF_VAL_DEF);
    set_eq(EQ_DEF);
    set_backlight(BACKLIGHT_DEF);
    set_gamedisp(GAMEDISP_DEF);
    set_themestyle(THEMESTYLE_DEF);

    fpFile = fopen(PARAMETER_FILE, "wb+");
    if (fpFile <= 0) {
        printf("create parameter file fail\n");
        return -1;
    }
    fwrite(&para_data, 1, sizeof(struct parameter_data), fpFile);
    fclose(fpFile);

    return 0;
}

int get_language(void)
{
    return para_data.language;
}

void set_language(int val)
{
    para_data.language = val;
    parameter_save();
}

int get_screenoff(void)
{
    return para_data.screenoff;
}

void set_screenoff(int val)
{
    para_data.screenoff = val;
    parameter_save();
}

int get_screenoff_val(void)
{
    return para_data.screenoff_val;
}

void set_screenoff_val(int val)
{
    para_data.screenoff_val = val;
    parameter_save();
}

int get_eq(void)
{
    return para_data.eq_val;
}

void set_eq(int val)
{
    para_data.eq_val = val;
    parameter_save();
}

int get_backlight(void)
{
    return para_data.backlight_val;
}

void set_backlight(int val)
{
    para_data.backlight_val = val;
    parameter_save();
}

int get_gamedisp(void)
{
    return para_data.gamedisp_val;
}

void set_gamedisp(int val)
{
    para_data.gamedisp_val = val;
    parameter_save();
}

int get_themestyle(void)
{
    return para_data.themestyle_val;
}

void set_themestyle(int val)
{
    para_data.themestyle_val = val;
    parameter_save();
}

char *get_ui_image_path(void)
{
    if (get_themestyle() == 0)
        return UI_IMAGE_PATH_0;
    else if (get_themestyle() == 1)
        return UI_IMAGE_PATH_1;
    else
        return UI_IMAGE_PATH_0;
}