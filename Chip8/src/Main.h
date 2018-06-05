#include <stdint.h>

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64

struct everything {
	uint8_t memory[0xFFF];
	uint8_t registers[0xF];
	uint8_t screen[SCREEN_HEIGHT][SCREEN_WIDTH];
	uint16_t stack[0xF];
	uint16_t program_counter;
	uint8_t stack_pointer;
	uint16_t I;
	uint8_t delay_timer;
	uint8_t sound_timer;
} pc = { {0}, {0}, {{0}}, {0}, 0, 0, 0, 0, 0 };

void fill_sprites(uint8_t *memory);
void unknown_instruction(int first, int second);
void not_implemented(int first, int second);
void tick(uint8_t, uint8_t);

void clear_display(void);
void draw(void);
void drawSprite(int x, int y, int length);
