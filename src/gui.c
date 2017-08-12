/*
	Please don't judge my programming from this gui and file loading code...
	I'm new to the calculator and it's rendering/file functions. Maybe check
	out compiler.c instead? :P
*/

#include "gui.h"

#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>

#include <string.h> //for memset

#include "files.h"
#include "bf/vm.h"

#define TRANSPARENT_COLOR 10

#define BACKGROUND_COLOR 0x8B
#define BORDER_COLOR gfx_black
#define TEXT_COLOR 0xEB

const char *text_info = "Run Info";
const char *text_bytecode = "Run Bytecode";
const char *text_native = "Run Native";

FileList_t list;
bool loaded_files = false;

//if the program list arrow is currently being used
bool list_focused = true;
//the arrow indexes for the file list and options
uint16_t file_index = 0, button_index = 0;

//the current pressed key
char key;

//if the interrupt checkbox is checked
bool key_interrupt = true;
//if the optimize checkbox is checked
bool optimize = true;

//hacky way to supply input to the xmas tree program
int input_index = 0;

//draw an outline
void rect_border(uint24_t x, uint8_t y, uint24_t width, uint8_t height) {
	gfx_HorizLine(x, y, width);
	gfx_HorizLine(x, y + height, width + 1);
	gfx_VertLine(x, y, height);
	gfx_VertLine(x + width, y, height);
}

//draw the right inner rectangle with the run options
void gui_file_info() {
	uint16_t info_width, bytecode_width, native_width;
	uint16_t arrow_x = 0; uint8_t arrow_y = 0;

	info_width = gfx_GetStringWidth(text_info);
	bytecode_width = gfx_GetStringWidth(text_bytecode);
	native_width = gfx_GetStringWidth(text_native);

	//Trust me, these absolute coordinates are just as cringey and confusing to me as they are to you :(

	gfx_SetColor(BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(170 + 1, 23 + 1, LCD_WIDTH - 170 - 4 - 2, LCD_HEIGHT - 20 * 2 - 7 - 2);

	gfx_SetTextFGColor(TEXT_COLOR);

	gfx_PrintStringXY(text_info, 170 + (LCD_WIDTH - 170) / 2 - info_width / 2, 26);

	gfx_SetColor(BORDER_COLOR);
	gfx_HorizLine(170 + (LCD_WIDTH - 170) / 2 - info_width / 2 - 4, 26 + 10, info_width + 8);

//optimize checkbox
	gfx_PrintStringXY("Optimize", 170 + 8 + 4 + 4, 50);
	rect_border(LCD_WIDTH - 4 - 4 - 8, 50, 8, 8);
	gfx_SetColor(optimize ? BORDER_COLOR : BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(LCD_WIDTH - 4 - 4 - 6, 52, 5, 5);
	gfx_SetColor(BORDER_COLOR);
//key interrupt checkbox
	gfx_PrintStringXY("Key Interrupt", 170 + 8 + 4 + 4, 50 + 12);
	rect_border(LCD_WIDTH - 4 - 4 - 8, 50 + 12, 8, 8);
	gfx_SetColor(key_interrupt ? BORDER_COLOR : BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(LCD_WIDTH - 4 - 4 - 6, 52 + 12, 5, 5);
	gfx_SetColor(BORDER_COLOR);
	gfx_PrintStringXY("(Bytecode only)", 170 + 8 + 4 + 4, 50 + 12 + 12);

	gfx_PrintStringXY(text_bytecode, 170 + (LCD_WIDTH - 170) / 2 - bytecode_width / 2, 170);
	rect_border(170 + (LCD_WIDTH - 170) / 2 - bytecode_width / 2 - 4, 170 - 4, bytecode_width + 6, 8 + 6);

	gfx_PrintStringXY(text_native, 170 + (LCD_WIDTH - 170) / 2 - native_width / 2, 170 + 8 + 6 + 4);
	rect_border(170 + (LCD_WIDTH - 170) / 2 - native_width / 2 - 4, 170 + 8 + 6 + 4 - 4, native_width + 6, 8 + 6);

	switch(button_index) {
		case 0:
			arrow_x = 170 + 6;
			arrow_y = 50;
			break;
		case 1:
			arrow_x = 170 + 6;
			arrow_y = 50 + 12;
			break;
		case 2:
			arrow_x = 170 + (LCD_WIDTH - 170) / 2 - bytecode_width / 2 - 4 - 8 -2;
			arrow_y = 170;
			break;
		case 3:
			arrow_x = 170 + (LCD_WIDTH - 170) / 2 - native_width / 2 - 4 - 8 - 2;
			arrow_y = 170 + 8 + 6 + 4;
			break;
	}

	gfx_SetTextFGColor(list_focused ? TEXT_COLOR : BORDER_COLOR);
	gfx_PrintStringXY(">", arrow_x, arrow_y);
}

//draw the left inner rectangle with the list of programs
void gui_list_files() {
	uint16_t i;

	if(!loaded_files) {
		list_Create(&list);
		loaded_files = true;
	}

	gfx_SetColor(BACKGROUND_COLOR);
	gfx_SetTextFGColor(TEXT_COLOR);

	if(list.amount == 0) {
		gfx_PrintStringXY("Create a TI-BASIC", 5, 30);
		gfx_PrintStringXY("program and put", 5, 30 + 10);
		gfx_PrintStringXY("brainfuck in it!", 5, 30 + 10 * 2);
		return;
	}

	gfx_FillRectangle_NoClip(2, 22, 170 - 2, LCD_HEIGHT - 20 * 2 - 4);

	for(i = 0; i < list.amount; i++) {
		if(i == file_index) {
			gfx_SetTextFGColor(list_focused ? BORDER_COLOR : TEXT_COLOR);
			gfx_PrintStringXY(">", 20 - 6 - 2, 30 + 10 * i);
			gfx_SetTextFGColor(TEXT_COLOR);
		}
		gfx_PrintStringXY(list.files[i], 20, 30 + 10 * i);
	}
	
}

//the starting indexes to render
int cursor_x = 0, cursor_y = 0;
//the current index of the console to write a string to
int console_x = 0, console_y = 0;

#define CONSOLE_LINES 50
#define CONSOLE_LINE_LENGTH 150

char console[CONSOLE_LINES][CONSOLE_LINE_LENGTH];

//Just render the console text, don't do the background rect
void gui_draw_console_text() {
	unsigned int i = 0, y = 53;

	for(i = 0; i < 14; i++) {
		//27 characters at a time
		char buffer[28];

		if(cursor_y + i >= CONSOLE_LINES || cursor_y < 0
			|| cursor_x >= CONSOLE_LINE_LENGTH || cursor_x < 0)
			continue;

		strncpy(buffer, &console[cursor_y + i][cursor_x], sizeof(buffer));
		buffer[27] = 0;
		gfx_PrintStringXY(buffer, 53, y);
		y += 10;
	}
}

//Render the border and inside background rect
void gui_draw_console() {
	gfx_SetMonospaceFont(8);

	gfx_SetColor(BORDER_COLOR);
	rect_border(50, 50, LCD_WIDTH - 100, LCD_HEIGHT - 100 + 2);
	gfx_SetColor(BACKGROUND_COLOR);
	gfx_FillRectangle_NoClip(50 + 1, 50 + 1, LCD_WIDTH - 100 - 1, LCD_HEIGHT - 100 + 1);

	gui_draw_console_text();
}


void gui_console_print_char(char c, bool draw) {
	if(c == '\n') {
		console_y++;
		console_x = 0;
		return;
	}

	if(console_x < CONSOLE_LINE_LENGTH) {
		console[console_y][console_x++] = c;
	}

	//only redraw if text is changed within the bounds of the current window
	if(draw && (console_x - cursor_x < 28 && console_x - cursor_x >= 0)
		&& (console_y - cursor_y < 14 && console_y - cursor_y >= 0)) {
		gui_draw_console_text();
	}
}


//Add text to the console
void gui_console_print(const char *str) {
	unsigned int i = 0, newlines = 0, len = strlen(str);

	for(i = 0; i < len; i++) {
		if(str[i] == '\n')
			newlines++;
		gui_console_print_char(str[i], false);
	}

	//only redraw if text is changed within the bounds of the current window
	if(( (console_x - len - cursor_x < 28 && console_x - len - cursor_x >= 0)
			|| (console_x - cursor_x < 28 && console_x - cursor_x >= 0))

		&& ((console_y - newlines - cursor_y < 14 && console_y - newlines >= 0)
			|| (console_y - cursor_y < 14 && console_y - cursor_y >= 0))) {

		gui_draw_console_text();
	}
}

//a quick routine to take control of the console after program finishes
void gui_navigate_console() {
	char key = 0;

	while(key != sk_Clear) {
		key = os_GetCSC();
		if(key == sk_Right) {
			cursor_x++;
			if(cursor_x > CONSOLE_LINE_LENGTH - 28)
				cursor_x = CONSOLE_LINE_LENGTH - 28;
			gui_draw_console();
		}
		if(key == sk_Left) {
			cursor_x--;
			if(cursor_x < 0)
				cursor_x = 0;
			gui_draw_console();
		}
		if(key == sk_Up) {
			cursor_y--;
			if(cursor_y < 0)
				cursor_y = 0;
			gui_draw_console();
		}
		if(key == sk_Down) {
			cursor_y++;
			if(cursor_y > CONSOLE_LINES - 14)
				cursor_y = CONSOLE_LINES - 14;
			gui_draw_console();
		}
	}

}

//clear the console contents and redraw other windows
void gui_reset_console() {
	console_x = 0;
	console_y = 0;
	cursor_x = 0;
	cursor_y = 0;

	memset(console, 0, sizeof(console));
	input_index = 0;
}

//Draw the entire gui without the console
void gui_draw() {
	gfx_SetMonospaceFont(0);

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
	gui_file_info();
}

//again, becaues malloc() is sketchy af here
#define MAX_PROGRAM_SIZE 12000
char code[MAX_PROGRAM_SIZE];

struct VM vm;

void run_bytecode() {
	int error;

	ti_var_t program;
	size_t size;

	ti_CloseAll();
	program = ti_OpenVar(list.files[file_index], "r", TI_PRGM_TYPE);
	size = ti_GetSize(program);

	if(size > MAX_PROGRAM_SIZE) {
		gui_draw_console();
		gui_console_print("Program too large. Sorry :(");
	} else {
		ti_Read(code, size, 1, program);
		ti_Close(program);

		gui_draw_console();

		gui_console_print("Compiling bytecode... ");
		vm_Create(&vm);

		if((error = vm_Compile(&vm, code, size, optimize)) != E_SUCCESS) {
			char buffer[50];
		       sprintf(buffer, "\nBytecode compile error %i (%s)", error, error_strings[error]);
		       gui_console_print(buffer);
		       //our alg doesn't work here for some reason so I'm just going to call manually lol
		       gui_draw_console_text();
		} else {
			gui_console_print("Done.\nRunning...\n");

			while(!vm_IsDone(&vm)) {
				int error;
		        error = vm_Step(&vm);
		        if(error != E_SUCCESS) {
		            char buffer[25];
		            sprintf(buffer, "Runtime error %i", error);
		            gui_console_print(buffer);
		            break;
		        }

		        if(key_interrupt) {
		        	kb_Scan();
		        	if(kb_Data[6] == kb_Clear)
		        		break;
		        }
		    }

		    strcpy(&console[1][10], " Done. Use arrows to navigate.");
		    gui_draw_console_text();

		}
	}
	
	vm_Cleanup(&vm);
	gui_navigate_console();
	gui_reset_console();
	gui_draw();
}

void run_native() {
	Compiler_t c;
	ti_var_t program;
	size_t size;

	uint8_t i;

	ti_CloseAll();
	program = ti_OpenVar(list.files[file_index], "r", TI_PRGM_TYPE);
	size = ti_GetSize(program);

	if(size > MAX_PROGRAM_SIZE) {
		gui_draw_console();
		gui_console_print("Program too large. Sorry :(");
	} else {
		ti_Read(code, size, 1, program);
		ti_Close(program);

		gui_draw_console();

		gui_console_print("Compiling native... ");

		comp_Create(&c, code, size);

		vm_Create(&vm);

		comp_CompileNative(&c, &vm.mem, optimize);

	    if (c.error != E_SUCCESS) {
		    char buffer[50];
		    sprintf(buffer, "\nNative compile error %i (%s)", c.error, error_strings[c.error]);
		    gui_console_print(buffer);
		    //our alg doesn't work here for some reason so I'm just going to call manually lol
		    gui_draw_console_text();
		} else {
		    gui_console_print("Done.\nRunning...\n");

		    (* ((void(*)()) c.code.native)) ();

		    strcpy(&console[1][10], " Done. Use arrows to navigate.");
		    gui_draw_console_text();

		}
	}

	//print the first 10 cells for debugging purposes
	dbg_sprintf(dbgout, "{");
	for(i = 0; i < 10; i++) {
		dbg_sprintf(dbgout, "0x%02X", vm.mem.cells[i]);
		if(i != 5)
			dbg_sprintf(dbgout, ", ");
	}
	dbg_sprintf(dbgout, "}\n");
	
	vm_Cleanup(&vm);
	gui_navigate_console();
	gui_reset_console();
	gui_draw();
}

void gui_run() {

    os_ClrHome();

	gfx_Begin();

	gui_draw();

	while(true) {
		key = os_GetCSC();

		if(key == sk_Down) {
			if(list_focused) {
				if(file_index < list.amount - 1) file_index++;
				else file_index = 0;

				gui_list_files();
			} else {
				if(button_index < 3) button_index++;
				else button_index = 0;

				gui_file_info();
			}
		} else if(key == sk_Up) {
			if(list_focused) {
				if(file_index > 0) file_index--;
				else file_index = list.amount - 1;
			
				gui_list_files();
			} else {
				if(button_index > 0) button_index--;
				else button_index = 3;

				gui_file_info();
			}
		} else if(key == sk_Enter) {
			if(list_focused) {
				if(list.amount > 0) {
					list_focused = false;
					gui_list_files();
					gui_file_info();
				}
			} else if(button_index == 0) {
				optimize = !optimize;
				gui_file_info();
			} else if(button_index == 1) {
				key_interrupt = !key_interrupt;
				gui_file_info();
			}else if(button_index == 2) {
				
				run_bytecode();

			} else if(button_index == 3) {
				
				run_native();
			    
			}
		} else if(key == sk_Right) {
			if(list.amount > 0 && list_focused) {
				list_focused = false;
				gui_list_files();
				gui_file_info();
			}
		} else if(key == sk_Left) {
			if(!list_focused) {
				list_focused = true;
				gui_list_files();
				gui_file_info();
			}
		} else if(key == sk_Clear) {
			break;
		}
	}

	gfx_End();
}

void gui_cleanup() {
	list_Cleanup(&list);
}

//TODO: Support input
CELL_TYPE bf_get_input() {
    int ret = 'A';
    if(input_index == 0) ret = '2';
    else if(input_index == 1) ret = '0';
    else if(input_index >= 2) ret = 0;
    input_index++;
    return ret;
}

void bf_print_cell(CELL_TYPE cell) {
	dbg_sprintf(dbgout, "%c", cell);
    gui_console_print_char(cell, true);
}