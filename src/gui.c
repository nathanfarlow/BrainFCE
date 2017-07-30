#include "gui.h"

#include <tice.h>
#include <graphx.h>

void pause();

void gui_run() {

	gfx_Begin();

	gfx_FillRectangle_NoClip(1, 1, LCD_WIDTH - 2, 19);

	pause();

	gfx_End();
}