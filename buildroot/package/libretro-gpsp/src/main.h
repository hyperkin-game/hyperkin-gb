/* gameplaySP
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

typedef enum
{
  TIMER_INACTIVE,
  TIMER_PRESCALE,
  TIMER_CASCADE
} timer_status_type;

typedef enum
{
  TIMER_NO_IRQ,
  TIMER_TRIGGER_IRQ
} timer_irq_type;


typedef enum
{
  TIMER_DS_CHANNEL_NONE,
  TIMER_DS_CHANNEL_A,
  TIMER_DS_CHANNEL_B,
  TIMER_DS_CHANNEL_BOTH
} timer_ds_channel_type;

typedef struct
{
  s32 count;
  u32 reload;
  u32 prescale;
  u32 stop_cpu_ticks;
  fixed8_24 frequency_step;
  timer_ds_channel_type direct_sound_channels;
  timer_irq_type irq;
  timer_status_type status;
} timer_type;

typedef enum
{
  no_frameskip = 0,
  auto_frameskip,
  auto_threshold_frameskip,
  fixed_interval_frameskip
} frameskip_type;

typedef enum
{
  auto_detect = 0,
  builtin_bios,
  official_bios
} bios_type;

typedef enum
{
  boot_game = 0,
  boot_bios
} boot_mode;

extern u32 cpu_ticks;
extern u32 execute_cycles;
extern u32 global_cycles_per_instruction;
extern u32 skip_next_frame;

extern u32 cycle_memory_access;
extern u32 cycle_pc_relative_access;
extern u32 cycle_sp_relative_access;
extern u32 cycle_block_memory_access;
extern u32 cycle_block_memory_sp_access;
extern u32 cycle_block_memory_words;
extern u32 cycle_dma16_words;
extern u32 cycle_dma32_words;
extern u32 flush_ram_count;

extern u64 base_timestamp;

extern char main_path[512];
extern char save_path[512];

u32 update_gba(void);
void reset_gba(void);

void init_main(void);

void game_name_ext(char *src, char *buffer, char *extension);
void main_write_savestate(void);
void main_read_savestate(void);


u32 file_length(FILE *fp);

extern u32 num_skipped_frames;
extern int dynarec_enable;
extern boot_mode selected_boot_mode;

void change_ext(const char *src, char *buffer, const char *extension);

#endif


