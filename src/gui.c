#include "gui.h"

#include <tice.h>
#include <graphx.h>

#include <string.h> //for memset

void pause();

#define TRANSPARENT_COLOR 10

#define BACKGROUND_COLOR gfx_white
#define BORDER_COLOR gfx_black
#define TEXT_COLOR 0xEB

void rect_border(uint24_t x, uint8_t y, uint24_t width, uint8_t height) {
	gfx_HorizLine(x, y, width);
	gfx_HorizLine(x, y + height, width);
	gfx_VertLine(x, y, height);
	gfx_VertLine(x + width, y, height);
}

uint16_t cursor_index;
void gui_list_files() {
	gfx_SetColor(BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(2, 22, 200 - 1, LCD_HEIGHT - 20 * 2 - 4);
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
	rect_border(200, 23, LCD_WIDTH - 200 - 4, LCD_HEIGHT - 20 * 2 - 7);

	gfx_PrintStringXY("BrainfuckCE (Cesium theme)", 6, 7);
	gfx_PrintStringXY("BrainfuckCE Version 1.0", 6, LCD_HEIGHT - 14);

	gui_list_files();

	pause();

	gfx_End();
}