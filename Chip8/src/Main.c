#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "term.h"
#include "Main.h"

// Passing the processor instance is a pain in the ass but,
// TODO(ym): globals are bad.
static Processor pc = { {0}, {0}, {0}, {{0}}, 0, 0, 0, {0}, 0, 0 };

int main(int argc, char **argv) {

	if(argc == 1) {
		fputs("No file specified!\n", stderr);
		return 1;
	}

	FILE *bin = fopen(*++argv, "rb");

	if(bin == NULL) {
		fprintf(stderr, "File not found: %s\n", *argv);
		return 2;
	}

	if(init_termios()) {
		fputs("Standard output is not a tty!\n", stderr);
		return 3;
	}

	fread(pc.memory + 0x200, 1, sizeof(pc.memory), bin);
	fclose(bin);

	fill_sprites(pc.memory);

	for(pc.program_counter = 0x200; pc.program_counter < 0xFFF; pc.program_counter += 2){
		tick(pc.memory[pc.program_counter], pc.memory[pc.program_counter + 1]);
		pc.delay_timer -= pc.delay_timer ? 1 : 0;
		pc.sound_timer -= pc.sound_timer ? 1 : 0;
		usleep(2000);
	}
}

int switch_key(int character) {
	for(int i = 0; i < 0xF; i++) {
		pc.keyboard[i] = 0;
	}

	if(character == -1) return -1;

	switch(character) {
		case 'i':
			exit(0);
		case '1':
			pc.keyboard[1] = 1;
			return 1;
		case '2':
			pc.keyboard[2] = 1;
			return 2;
		case '3':
			pc.keyboard[3] = 1;
			return 3;
		case '4':
			pc.keyboard[0xC] = 1;
			return 0xC;
		case '\'':
			pc.keyboard[4] = 1;
			return 4;
		case ',':
			pc.keyboard[5] = 1;
			return 5;
		case '.':
			pc.keyboard[6] = 1;
			return 6;
		case 'p':
			pc.keyboard[0xD] = 1;
			return 0xD;
		case 'a':
			pc.keyboard[7] = 1;
			return 7;
		case 'o':
			pc.keyboard[8] = 1;
			return 8;
		case 'e':
			pc.keyboard[9] = 1;
			return 9;
		case 'u':
			pc.keyboard[0xE] = 1;
			return 0xE;
		case ';':
			pc.keyboard[0xA] = 1;
			return 0xA;
		case 'q':
			pc.keyboard[0] = 1;
			return 0;
		case 'j':
			pc.keyboard[0xB] = 1;
			return 0xB;
		case 'k':
			pc.keyboard[0xF] = 1;
			return 0xF;
		default:
			break;
	}
	return -1;
}

void fill_sprites(uint8_t *memory) {
	uint8_t sprites[][5] = {
		{ 0xF0, 0x90, 0x90, 0x90, 0xF0 },
		{ 0x20, 0x60, 0x20, 0x20, 0x70 },
		{ 0xF0, 0x10, 0xF0, 0x80, 0xF0 },
		{ 0xF0, 0x10, 0xF0, 0x10, 0xF0 },
		{ 0x90, 0x90, 0xF0, 0x10, 0x10 },
		{ 0xF0, 0x80, 0xF0, 0x10, 0xF0 },
		{ 0xF0, 0x80, 0xF0, 0x90, 0xF0 },
		{ 0xF0, 0x10, 0x20, 0x40, 0x40 },
		{ 0xF0, 0x90, 0xF0, 0x90, 0xF0 },
		{ 0xF0, 0x90, 0xF0, 0x10, 0xF0 },
		{ 0xF0, 0x90, 0xF0, 0x90, 0x90 },
		{ 0xE0, 0x90, 0xE0, 0x90, 0xE0 },
		{ 0xF0, 0x80, 0x80, 0x80, 0xF0 },
		{ 0xE0, 0x90, 0x90, 0x90, 0xE0 },
		{ 0xF0, 0x80, 0xF0, 0x80, 0xF0 },
		{ 0xF0, 0x80, 0xF0, 0x80, 0x80 }
	};

	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 5; j++) {
			memory[i * 5 + j] = sprites[i][j];
		}
	}
}

void unknown_instruction(int first, int second) {
	printf("Unknown instruction: 0x%x, at: 0x%x.\r\n", (((uint16_t) (first << 8)) | second), pc.program_counter);
	exit(4);
}

void tick(uint8_t first, uint8_t second) {

	switch_key(getchar());

	uint8_t x = first & 0xF;
	uint8_t y = (second & 0xF0) >> 4;
	uint16_t nnn = ((first & 0xF) << 8) | second;

	switch((first & 0xF0) >> 4) {
		case 0x0:
			switch(second & 0xF) {
				case 0x0:
					clear_display();
					break;
				case 0xE:
					pc.program_counter = pc.stack[--pc.stack_pointer];
					break;
				default:
					unknown_instruction(first, second);
					break;
			}
			break;
		case 0x1:
			pc.program_counter = nnn - 2;
			break;
		case 0x2:
			pc.stack[pc.stack_pointer++] = pc.program_counter;
			pc.program_counter = nnn - 2;
			break;
		case 0x3:
			if(pc.registers[x] == second) pc.program_counter += 2;
			break;
		case 0x4:
			if(pc.registers[x] != second) pc.program_counter += 2;
			break;
		case 0x5:
			if(pc.registers[x] == pc.registers[y]) pc.program_counter += 2;
			break;
		case 0x6:
			pc.registers[x] = second;
			break;
		case 0x7:
			pc.registers[x] += second;
			break;
		case 0x8:
			switch(second & 0xF) {
				case 0x0:
					pc.registers[x] = pc.registers[y];
					break;
				case 0x1:
					pc.registers[x] |= pc.registers[y];
					break;
				case 0x2:
					pc.registers[x] &=  pc.registers[y];
					break;
				case 0x3:
					pc.registers[x] ^= pc.registers[y];
					break;
				case 0x4: {
					uint16_t result = pc.registers[x] + pc.registers[y];
					pc.registers[0xF] = result > 0xFF;
					pc.registers[x] = (result & 0xFF);
				} break;
				case 0x5:
					pc.registers[0xF] = pc.registers[x] > pc.registers[y];
					pc.registers[x] -= pc.registers[y];
					break;
				case 0x6:
					pc.registers[0xF] = pc.registers[x] & 0x1;
					pc.registers[x] >>= 1;
					break;
				case 0x7:
					pc.registers[0xF] = pc.registers[y] > pc.registers[x];
					pc.registers[x] = (uint8_t) (pc.registers[y] - pc.registers[x]);
					break;
				case 0xE:
					pc.registers[0xF] = pc.registers[x] & 0x80;
					pc.registers[x] <<= 1;
					break;
				default:
					unknown_instruction(first, second);
					break;
			}
			break;
		case 0x9:
			if(pc.registers[x] != pc.registers[y]) {
				pc.program_counter += 2;
			}
			break;
		case 0xA:
			pc.I = nnn;
			break;
		case 0xB:
			pc.program_counter = (pc.registers[0] + nnn) & 0xFFF;
			break;
		case 0xC:
			pc.registers[x] = ((uint8_t) (rand() % 0xFF)) & second;
			break;
		case 0xD:
			draw_sprite(pc.registers[x], pc.registers[y], second & 0xF);
			break;
		case 0xE:
			if(second == 0x9E) {
				if(pc.keyboard[pc.registers[x]] == 1) {
					pc.program_counter += 2;
				}
			} else if(second == 0xA1) {
				if(pc.keyboard[pc.registers[x]] == 0) {
					pc.program_counter += 2;
				}
			} else {
				unknown_instruction(first, second);
			}
			break;
		case 0xF:
			switch((second & 0xF0) >> 4) {
				case 0x0:
					switch(second & 0xF) {
						case 0x7:
							pc.registers[x] = pc.delay_timer;
							break;
						case 0xA: {
							int character_num = -1;
							while((character_num = switch_key(getchar())) == -1);
							pc.registers[x] = character_num;
						} break;
						default:
							unknown_instruction(first, second);
							break;
					}
					break;
				case 0x1:
					switch(second & 0xF) {
						case 0x5:
							pc.delay_timer = pc.registers[x];
							break;
						case 0x8:
							pc.sound_timer = pc.registers[x];
							break;
						case 0xE:
							pc.I += pc.registers[x];
							pc.registers[0xF] = pc.I > 0xFFF;
							pc.I &= 0xFFF;
							break;
						default:
							unknown_instruction(first, second);
							break;
					}
					break;
				default:
					switch(second) {
						case 0x29:
							pc.I = pc.registers[x] * 5;
							break;
						case 0x33: {
							int value = pc.registers[x];
							for(int i = 3; i > 0; i--) {
								pc.memory[pc.I + i] = value % 10;
								value /= 10;
							}
						  } break;
						case 0x55:
							for(int i = 0; i <= x; i++) {
								pc.memory[pc.I + i] = pc.registers[i];
							}
							/* pc.I += x; */
							break;
						case 0x65:
							for(int i = 0; i <= x; i++) {
								pc.registers[i] = pc.memory[pc.I + i];
							}
							/* pc.I += x; */
							break;
						default:
							unknown_instruction(first, second);
							break;
					}
					break;
			}
			break;
		default:
			unknown_instruction(first, second);
			break;
	}
}

void clear_display(void) {
	for(int i = 0; i < SCREEN_HEIGHT; i++) {
		for(int j = 0; j < SCREEN_WIDTH; j++) {
			pc.screen[i][j] = 0;
		}
	}
	draw();
}

void draw(void) {
	printf("\033[2J");
	printf("\033[0;0H");
	for(int i = 0; i < SCREEN_HEIGHT; i++) {
		for(int j = 0; j < SCREEN_WIDTH; j++) {
			printf(!pc.screen[i][j] ? " " : "█");
		}
		printf("\r\n");
	}
}

void draw_sprite(int x, int y, int length) {
	pc.registers[0xF] = 0;
	for(int i = 0; i < length; i++) {
		for(int k = 0; k < 8; k++) {
			uint8_t pixel = pc.screen[(y + i) % SCREEN_HEIGHT][(x + k) % SCREEN_WIDTH];
			uint8_t new_pixel = (pixel ^ (pc.memory[pc.I + i] & (0x80 >> k)));
			if(pixel == 1 && new_pixel == 0) {
				pc.registers[0xF] |= 1;
			}
			pc.screen[(y + i) % SCREEN_HEIGHT][(x + k) % SCREEN_WIDTH] = new_pixel;
		}
	}
	draw();
}

void not_implemented(int first, int second) {
	printf("Unimplemented instruction: 0x%x, at: 0x%x.\r\n", (((uint16_t) (first << 8)) | second), pc.program_counter);
}
