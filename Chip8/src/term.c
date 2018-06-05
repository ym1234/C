#include <termios.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "term.h"

#define TERMINFO_DIRECTORY "/usr/share/terminfo/"

typedef struct termios termios;
static termios old_termios;

int init_termios(void) {
	if(!isatty(STDOUT_FILENO)) {
		return -1;
	}
	termios new_termios;
	tcgetattr(STDOUT_FILENO, &old_termios);
	memcpy(&new_termios, &old_termios, sizeof(termios));

	// Raw mode
	cfmakeraw(&new_termios);

	// Make read() async
	new_termios.c_cc[VMIN] = 0;
	new_termios.c_cc[VTIME] = 0;

	tcsetattr(STDOUT_FILENO, TCSANOW, &new_termios);
	atexit(reset_termios);
	return 0;
}

void reset_termios(void) {
	tcsetattr(0, TCSANOW, &old_termios);
}

// Resources:
// 		https://github.com/mauke/unibilium/blob/master/secret/terminfo.pod
/* int read_terminfo(terminfo *info) { */
/* 	FILE *terminfo_file = fopen(get_terminfo(), "rb"); */
/* 	int file_size = get_file_size(terminfo_file); */

/* 	uint8_t buffer[file_size]; */
/* 	fread(buffer, 1, file_size, terminfo_file); */

/* 	// Magic */
/* 	if(to_int(buffer[0], buffer[1]) != 0x1A01) { */
/* 			return -1; */
/* 	} */

/* 	uint16_t NAME_SIZE = to_int(buffer[2], buffer[3]); */
/* 	uint16_t BOOL_COUNT = to_int(buffer[4], buffer[5]); */
/* 	uint16_t NUM_COUNT = to_int(buffer[6], buffer[7]); */
/* 	uint16_t STRING_COUNT = to_int(buffer[8], buffer[9]); */
/* 	uint16_t TABLE_SIZE = to_int(buffer[10], buffer[11]); */

/* 	// malloc(0) is implementation dependant */
/* 	char *term_name = NAME_SIZE == 0 ? NULL : malloc(sizeof(char) * NAME_SIZE); */
/* 	for(int i = 0; i < NAME_SIZE; j++) { */
/* 		term_name[i] = (char) buffer[12 + i]; */
/* 	} */

/* } */


uint16_t to_int(uint8_t first, uint8_t second) {
	return ((first << 8) & 0xFF00) | second;
}

// TODO(ym): Error checking (return off_t instead of int, check the result of fstat, etc)
// you can also use glib functions but eh
int get_file_size(FILE *file) {
	struct stat buffer;
	fstat(fileno(file), &buffer);
	return buffer.st_size;
}

char *get_terminfo(void) {
	const char *term_name = getenv("TERM");
	char *terminfo = calloc(sizeof(char), strlen(term_name) + strlen(TERMINFO_DIRECTORY) + 3);
	char character[3] = { *term_name, '/', '\0' };
	return strcat(strcat(strcat(terminfo, TERMINFO_DIRECTORY), character), term_name);
}
