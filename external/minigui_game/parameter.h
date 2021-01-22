/*
 * Copyright (c) 2018 rockchip
 *
 */
#ifndef _PARAMETER_H_
#define _PARAMETER_H_

int parameter_recovery(void);
int get_language(void);
void set_language(int val);
int get_screenoff(void);
void set_screenoff(int val);
int get_screenoff_val(void);
void set_screenoff_val(int val);
int get_eq(void);
void set_eq(int val);
int get_backlight(void);
void set_backlight(int val);
int get_gamedisp(void);
void set_gamedisp(int val);
int get_themestyle(void);
void set_themestyle(int val);
char *get_ui_image_path(void);
int parameter_init(void);
void parameter_deinit(void);
#endif
