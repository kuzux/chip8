#ifndef _MAIN_H
#define _MAIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <SDL2/SDL.h>

#define VERSION "0.01"

#define SCR_WIDTH 64
#define SCR_HEIGHT 32
#define SCR_SIZE 64 * 32

#define WINDOW_TITLE "Chip8 Emulator"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320

#define PIXEL_WIDTH 10
#define PIXEL_HEIGHT 10

#define CFG_FILE "chip8.cfg"

#endif
