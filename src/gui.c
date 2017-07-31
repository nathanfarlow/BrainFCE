#include "gui.h"



#include <tice.h>
#include <graphx.h>

#include <math.h>
#include <string.h> //for memset

#include "files.h"

void pause();

#define TRANSPARENT_COLOR 10

#define BACKGROUND_COLOR 0x8B
#define BORDER_COLOR gfx_black
#define TEXT_COLOR 0xEB

void rect_border(uint24_t x, uint8_t y, uint24_t width, uint8_t height) {
	gfx_HorizLine(x, y, width);
	gfx_HorizLine(x, y + height, width);
	gfx_VertLine(x, y, height);
	gfx_VertLine(x + width, y, height);
}

FileList_t list;
bool loaded_files = false;

uint16_t cursor_index = 0;

char key;

void gui_list_files() {
	uint16_t i;

	if(!loaded_files) {
		list_Create(&list);
		loaded_files = true;
	}

	gfx_SetColor(BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(2, 22, 170 - 2, LCD_HEIGHT - 20 * 2 - 4);

	for(i = 0; i < list.amount; i++) {
		if(i == cursor_index) {
			gfx_PrintStringXY(">", 20 - 6, 30 + 10 * i);
		}
		gfx_PrintStringXY(list.files[i], 20, 30 + 10 * i);
	}
}

void gui_run() {

    os_ClrHome();

	gfx_Begin();

	gfx_FillScreen(BACKGROUND_COLOR);

	gfx_SetTextTransparentColor(TRANSPARENT_COLOR);
	gfx_SetTextBGColor(TRANSPARENT_COLOR);
	gfx_SetTextFGColor(TEXT_COLOR);

	gfx_SetColor(BORDER_COLOR);
	//top solid rect
	gfx_FillRectangle_NoClip(1, 1, LCD_WIDTH - 2, 19);
	//bottom solid rect
	gfx_FillRectangle_NoClip(1, LCD_HEIGHT - 20, LCD_WIDTH - 2, 19);
	//large border
	rect_border(1, 21, LCD_WIDTH - 3, LCD_HEIGHT - 20 * 2 - 3);
	//inside border
	rect_border(170, 23, LCD_WIDTH - 170 - 4, LCD_HEIGHT - 20 * 2 - 7);

	gfx_PrintStringXY("BrainfuckCE (Cesium theme)", 6, 7);
	gfx_PrintStringXY("BrainfuckCE Version 1.0", 6, LCD_HEIGHT - 14);

	gui_list_files();

	while(true) {
		key = os_GetCSC();

		if(key == sk_Down) {
			if(cursor_index < list.amount - 1) cursor_index++;
			else cursor_index = 0;

			gui_list_files();
		} else if(key == sk_Up) {
			if(cursor_index > 0) cursor_index--;
			else cursor_index = list.amount - 1;
			
			gui_list_files();
		}else if(key != 0) {
			break;
		}
	}

	gfx_End();
}

void gui_cleanup() {
	list_Cleanup(&list);
}