#include <stdint.h>

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64

typedef struct {
	uint8_t memory[0xFFF];
	uint8_t registers[0x10];
	uint8_t keyboard[0x10];
	uint8_t screen[SCREEN_HEIGHT][SCREEN_WIDTH];
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint8_t stack_pointer;
	uint16_t stack[0x10];
	uint16_t program_counter;
	uint16_t I;
} Processor;

void fill_sprites(uint8_t *memory);
void unknown_instruction(int first, int second);
void not_implemented(int first, int second);
void tick(uint8_t, uint8_t);
int switch_key(int);

void clear_display(void);
void draw(void);
void draw_sprite(int, int, int);
