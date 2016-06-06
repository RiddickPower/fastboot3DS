#include <sys/iosupport.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include "types.h"
#include "cache.h"
#include "gfx.h"
#include "console.h"
#include "default_font_bin.h"

#define TAB_SIZE	4

ssize_t con_write(struct _reent *r,int fd,const char *ptr, size_t len);

PrintConsole defaultConsole =
{
	//Font:
	{
		default_font_bin, //font gfx
		0, //first ascii character in the set
		256 //number of characters in the font set
	},
	(u32*)NULL,
	0,0,	//cursorX cursorY
	0,0,	//prevcursorX prevcursorY
	40,		//console width
	30,		//console height
	0,		//window x
	0,		//window y
	40,		//window width
	30,		//window height
	3,		//tab size
	15,		// foreground color
	0,		// background color
	0,		// flags
	false	//console initialized
};

static const devoptab_t dotab_stdout = {
	"con",
	0,
	NULL,
	NULL,
	con_write,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


// Format: RGBA
static const u32 colorTable[] =
{
	// Normal
	0x00000000,	// Black
	0xAA000000,	// Red
	0x00AA0000,	// Green
	0xAAAA0000,	// Yellow
	0x000000AA,	// Blue
	0xAA00AA00,	// Magenta
	0x00AAAA00,	// Cyan
	0xC0C0C000,	// Gray
	
	// Bright
	0xAAAAAA00,	// Darkgray
	0xFF000000,	// Red
	0x00FF0000,	// Green
	0xFFFF0000,	// Yellow
	0x0000FF00,	// Blue
	0xFF00FF00,	// Magenta
	0x00FFFF00,	// Cyan
	0xFFFFFF00	// White
};

PrintConsole currentCopy;

PrintConsole* currentConsole = &currentCopy;

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

void consolePrintChar(int c);
void consoleDrawChar(int c);

//---------------------------------------------------------------------------------
static void consoleCls(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp,rowTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			while(i++ < ((currentConsole->windowHeight * currentConsole->windowWidth) - (rowTemp * currentConsole->consoleWidth + colTemp)))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while (i++ < (rowTemp * currentConsole->windowWidth + colTemp))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '2':
		{
			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowHeight * currentConsole->windowWidth)
				consolePrintChar(' ');

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;
			break;
		}
	default: ;
	}
	//gfxFlushBuffers();
}
//---------------------------------------------------------------------------------
static void consoleClearLine(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;

			while(i++ < (currentConsole->windowWidth - colTemp)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < ((currentConsole->windowWidth - colTemp)-2)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '2':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowWidth) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	default: ;
	}
	//gfxFlushBuffers();
}


//---------------------------------------------------------------------------------
ssize_t con_write(struct _reent *r,int fd,const char *ptr, size_t len) {
//---------------------------------------------------------------------------------

	char chr;

	int i, count = 0;
	char *tmp = (char*)ptr;

	if(!tmp || len<=0) return -1;

	i = 0;

	while((size_t)i<len) {

		chr = *(tmp++);
		i++; count++;
		
		if((u32) tmp > 0x8100000) panic();	// ptr poisoned.

		if ( chr == 0x1b && *tmp == '[' ) {
			bool escaping = true;
			char *escapeseq	= tmp++;
			int escapelen = 1;
			i++; count++;

			do {
				chr = *(tmp++);
				i++; count++; escapelen++;
				int parameter, assigned, consumed;
				
				if((u32) tmp > 0x8100000) panic();	// ptr poisoned.
				if((u32) escapeseq > 0x8100000) panic();	// ptr poisoned.

				// make sure parameters are positive values and delimited by semicolon
				if((chr >= '0' && chr <= '9') || chr == ';')
					continue;

				switch (chr) {
					//---------------------------------------
					// Cursor directional movement
					//---------------------------------------
					case 'A':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dA%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorY  =  (currentConsole->cursorY  - parameter) < 0 ? 0 : currentConsole->cursorY  - parameter;
						escaping = false;
						break;
					case 'B':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dB%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorY  =  (currentConsole->cursorY  + parameter) > currentConsole->windowHeight - 1 ? currentConsole->windowHeight - 1 : currentConsole->cursorY  + parameter;
						escaping = false;
						break;
					case 'C':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dC%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorX  =  (currentConsole->cursorX  + parameter) > currentConsole->windowWidth - 1 ? currentConsole->windowWidth - 1 : currentConsole->cursorX  + parameter;
						escaping = false;
						break;
					case 'D':
						consumed = 0;
						assigned = sscanf(escapeseq,"[%dD%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorX  =  (currentConsole->cursorX  - parameter) < 0 ? 0 : currentConsole->cursorX  - parameter;
						escaping = false;
						break;
					//---------------------------------------
					// Cursor position movement
					//---------------------------------------
					case 'H':
					case 'f':
					{
						int  x, y;
						char c;
						if(sscanf(escapeseq,"[%d;%d%c", &y, &x, &c) == 3 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(sscanf(escapeseq,"[%d;%c", &y, &c) == 2 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(sscanf(escapeseq,"[;%d%c", &x, &c) == 2 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(sscanf(escapeseq,"[;%c", &c) == 1 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						// invalid format
						escaping = false;
						break;
					}
					//---------------------------------------
					// Screen clear
					//---------------------------------------
					case 'J':
						if(escapelen <= 3)
							consoleCls(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Line clear
					//---------------------------------------
					case 'K':
						if(escapelen <= 3)
							consoleClearLine(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Save cursor position
					//---------------------------------------
					case 's':
						if(escapelen == 2) {
							currentConsole->prevCursorX  = currentConsole->cursorX ;
							currentConsole->prevCursorY  = currentConsole->cursorY ;
						}
						escaping = false;
						break;
					//---------------------------------------
					// Load cursor position
					//---------------------------------------
					case 'u':
						if(escapelen == 2) {
							currentConsole->cursorX  = currentConsole->prevCursorX ;
							currentConsole->cursorY  = currentConsole->prevCursorY ;
						}
						escaping = false;
						break;
					//---------------------------------------
					// Color scan codes
					//---------------------------------------
					case 'm':
						escapeseq++;
						escapelen--;

						do {
							parameter = 0;
							if (escapelen == 1) {
								consumed = 1;
							} else {
								if((u32) escapeseq > 0x8100000) panic();	// ptr poisoned.
								if (strchr(escapeseq,';')) {
									sscanf(escapeseq,"%d;%n", &parameter, &consumed);
								}
							 else 
								sscanf(escapeseq,"%dm%n", &parameter, &consumed);
							}

							escapeseq += consumed;
							escapelen -= consumed;

							switch(parameter) {
							case 0: // reset
								currentConsole->flags = 0;
								currentConsole->bg    = 0;
								currentConsole->fg    = 7;
								break;

							case 1: // bold
								currentConsole->flags &= ~CONSOLE_COLOR_FAINT;
								currentConsole->flags |= CONSOLE_COLOR_BOLD;
								break;

							case 2: // faint
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								currentConsole->flags |= CONSOLE_COLOR_FAINT;
								break;

							case 3: // italic
								currentConsole->flags |= CONSOLE_ITALIC;
								break;

							case 4: // underline
								currentConsole->flags |= CONSOLE_UNDERLINE;
								break;

							case 5: // blink slow
								currentConsole->flags &= ~CONSOLE_BLINK_FAST;
								currentConsole->flags |= CONSOLE_BLINK_SLOW;
								break;

							case 6: // blink fast
								currentConsole->flags &= ~CONSOLE_BLINK_SLOW;
								currentConsole->flags |= CONSOLE_BLINK_FAST;
								break;

							case 7: // reverse video
								currentConsole->flags |= CONSOLE_COLOR_REVERSE;
								break;

							case 8: // conceal
								currentConsole->flags |= CONSOLE_CONCEAL;
								break;

							case 9: // crossed-out
								currentConsole->flags |= CONSOLE_CROSSED_OUT;
								break;

							case 21: // bold off
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								break;

							case 22: // normal color
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								currentConsole->flags &= ~CONSOLE_COLOR_FAINT;
								break;

							case 23: // italic off
								currentConsole->flags &= ~CONSOLE_ITALIC;
								break;

							case 24: // underline off
								currentConsole->flags &= ~CONSOLE_UNDERLINE;
								break;

							case 25: // blink off
								currentConsole->flags &= ~CONSOLE_BLINK_SLOW;
								currentConsole->flags &= ~CONSOLE_BLINK_FAST;
								break;

							case 27: // reverse off
								currentConsole->flags &= ~CONSOLE_COLOR_REVERSE;
								break;

							case 29: // crossed-out off
								currentConsole->flags &= ~CONSOLE_CROSSED_OUT;
								break;

							case 30 ... 37: // writing color
								currentConsole->fg = (u32)parameter - 30;
								break;

							case 39: // reset foreground color
								currentConsole->fg = 7;
								break;

							case 40 ... 47: // screen color
								currentConsole->bg = (u32)parameter - 40;
								break;

							case 49: // reset background color
								currentConsole->fg = 0;
								break;
							default: ;
							}
						} while (escapelen > 0);

						escaping = false;
						break;

					default:
						// some sort of unsupported escape; just gloss over it
						escaping = false;
						break;
				}
			} while (escaping);
			continue;
		}

		consolePrintChar(chr);
	}

	return count;
}


static const devoptab_t dotab_null = {
	"null",
	0,
	NULL,
	NULL,
	con_write,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

//---------------------------------------------------------------------------------
PrintConsole* consoleInit(int screen, PrintConsole* console) {
//---------------------------------------------------------------------------------

	static bool firstConsoleInit = true;
	
	if(firstConsoleInit) {
		devoptab_list[STD_OUT] = &dotab_stdout;
		devoptab_list[STD_ERR] = &dotab_stdout;
		
		setvbuf(stdout, NULL , _IONBF, 0);
		setvbuf(stderr, NULL , _IONBF, 0);

		firstConsoleInit = false;
	}

	if(console) {
		currentConsole = console;
	} else {
		console = currentConsole;
	}

	*currentConsole = defaultConsole;

	console->consoleInitialised = 1;

	if(screen==1) {
		console->frameBuffer = (u32*)FRAMEBUF_TOP_A_1;
		console->consoleWidth = 50;
		console->windowWidth = 50;
	}
	else
		console->frameBuffer = (u32*)FRAMEBUF_SUB_A_1;

	consoleCls('2');

	return currentConsole;

}

//---------------------------------------------------------------------------------
PrintConsole *consoleSelect(PrintConsole* console){
//---------------------------------------------------------------------------------
	PrintConsole *tmp = currentConsole;
	currentConsole = console;
	return tmp;
}

//---------------------------------------------------------------------------------
void consoleSetFont(PrintConsole* console, ConsoleFont* font){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->font = *font;

}

//---------------------------------------------------------------------------------
static void newRow() {
//---------------------------------------------------------------------------------


	currentConsole->cursorY ++;


	if(currentConsole->cursorY  >= currentConsole->windowHeight)  {
		currentConsole->cursorY --;
		u32 *dst = &currentConsole->frameBuffer[(currentConsole->windowX * 8 * 240) + (239 - (currentConsole->windowY * 8))];
		u32 *src = dst - 8;

		int i,j;

		for (i=0; i<currentConsole->windowWidth*8; i++) {
			u32 *from = src;
			u32 *to = dst;
			for (j=0;j<((currentConsole->windowHeight-1)*8);j++) *(to--) = *(from--);
			dst += 240;
			src += 240;
		}

		consoleClearLine('2');
		//gfxFlushBuffers();
	}
}
//---------------------------------------------------------------------------------
void consoleDrawChar(int c) {
//---------------------------------------------------------------------------------
	c -= currentConsole->font.asciiOffset;
	if ( c < 0 || c > currentConsole->font.numChars ) return;

	const u8 *fontdata = currentConsole->font.gfx + (8 * c);

	u32 writingColor = currentConsole->fg;
	u32 screenColor = currentConsole->bg;

	if (currentConsole->flags & CONSOLE_COLOR_BOLD) {
		writingColor += 8;
	} else if (currentConsole->flags & CONSOLE_COLOR_FAINT) {
		writingColor += 16;
	}

	if (currentConsole->flags & CONSOLE_COLOR_REVERSE) {
		u32 tmp = writingColor;
		writingColor = screenColor;
		screenColor = tmp;
	}

	u32 bg = colorTable[screenColor];
	u32 fg = colorTable[writingColor];

	u8 b1 = *(fontdata++);
	u8 b2 = *(fontdata++);
	u8 b3 = *(fontdata++);
	u8 b4 = *(fontdata++);
	u8 b5 = *(fontdata++);
	u8 b6 = *(fontdata++);
	u8 b7 = *(fontdata++);
	u8 b8 = *(fontdata++);

	if (currentConsole->flags & CONSOLE_UNDERLINE) b8 = 0xff;

	if (currentConsole->flags & CONSOLE_CROSSED_OUT) b4 = 0xff;

	u8 mask = 0x80;


	int i;

	int x = (currentConsole->cursorX + currentConsole->windowX) * 8;
	int y = ((currentConsole->cursorY + currentConsole->windowY) *8 );

	u32 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 7))];

	for (i=0;i<8;i++) {
		if (b8 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b7 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b6 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b5 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b4 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b3 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b2 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b1 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		mask >>= 1;
		screen += 240 - 8;
	}

}

//---------------------------------------------------------------------------------
void consolePrintChar(int c) {
//---------------------------------------------------------------------------------
	if (c==0) return;

	if(currentConsole->cursorX  >= currentConsole->windowWidth) {
		currentConsole->cursorX  = 0;

		newRow();
	}

	switch(c) {
		/*
		The only special characters we will handle are tab (\t), carriage return (\r), line feed (\n)
		and backspace (\b).
		Carriage return & line feed will function the same: go to next line and put cursor at the beginning.
		For everything else, use VT sequences.

		Reason: VT sequences are more specific to the task of cursor placement.
		The special escape sequences \b \f & \v are archaic and non-portable.
		*/
		case 8:
			currentConsole->cursorX--;

			if(currentConsole->cursorX < 0) {
				if(currentConsole->cursorY > 0) {
					currentConsole->cursorX = currentConsole->windowX - 1;
					currentConsole->cursorY--;
				} else {
					currentConsole->cursorX = 0;
				}
			}

			consoleDrawChar(' ');
			break;

		case 9:
			currentConsole->cursorX  += currentConsole->tabSize - ((currentConsole->cursorX)%(currentConsole->tabSize));
			break;
		case 10:
			newRow();
		case 13:
			currentConsole->cursorX  = 0;
			//gfxFlushBuffers();
			break;
		default:
			consoleDrawChar(c);
			++currentConsole->cursorX ;
			break;
	}
	
	//flushDCacheRange((void*)FRAMEBUF_TOP_A_1, 0xA8C00);
	flushDCache();
}

//---------------------------------------------------------------------------------
void consoleClear(void) {
//---------------------------------------------------------------------------------
	iprintf("\x1b[2J");
}

//---------------------------------------------------------------------------------
void consoleSetWindow(PrintConsole* console, int x, int y, int width, int height){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->windowWidth = width;
	console->windowHeight = height;
	console->windowX = x;
	console->windowY = y;

	console->cursorX = 0;
	console->cursorY = 0;

}

void drawConsoleWindow(PrintConsole* console, int thickness, u8 colorIndex) {

	if(colorIndex >= 16) return;

	if(!console) console = currentConsole;
	
	int startx = console->windowX * 8 - thickness;
	int endx = (console->windowX + console->windowWidth) * 8 + thickness;
	
	int starty = (console->windowY - 1) * 8 - thickness;
	int endy = console->windowHeight * 8 + thickness;
	
	/*
	printf("startx: %i\n", startx);
	printf("endx: %i\n", endx);
	printf("endy: %i\n", endy);
	printf("starty: %i\n", starty);
	*/

	u32 color = colorTable[colorIndex];
	
	// upper line
	for(int y = starty; y < starty + thickness; y++)
		for(int x = startx; x < endx; x++)
		{
			u32 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 7))];
			*screen = color;
		}
	
	// lower line
	for(int y = endy; y > endy - thickness; y--)
		for(int x = startx; x < endx; x++)
		{
			u32 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 7))];
			*screen = color;
		}
		
	// left and right
	for(int y = starty; y < endy; y++)
	{
		for(int i = 0; i < thickness; i++)
		{
			u32 *screen = &currentConsole->frameBuffer[((startx + i) * 240) + (239 - (y + 7))];
			*screen = color;
			screen = &currentConsole->frameBuffer[((endx - thickness + i) * 240) + (239 - (y + 7))];
			*screen = color;
		}
	}
}
