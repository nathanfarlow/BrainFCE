#ifndef BF_H_
#define BF_H_

//Remember to set DEBUGMODE to DEBUG in makefile

//define if you want to print the output and cells to cemu console
//#define _DEBUG

#ifdef _DEBUG
//define if you want to debug the native code in cemu
#define _DEBUG_NATIVE

#define _DEBUG_CELL_AMOUNT 10
#endif

#endif