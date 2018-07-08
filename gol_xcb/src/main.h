#include <xcb/xcb.h>
#include <xcb/xproto.h>

// There is probably an easier/faster way to do this with one-dimensional arrays, but too lazy/stupid to think about it.
#define INDEX(i, j) wrap(i, board->length) * board->width + wrap(j, board->width)

typedef struct {
	float x;
	float y;
} Vector;

typedef struct {
	Vector offset;
	float scale;
} Screen;

typedef struct {
	int height;
	int width;
	xcb_window_t id;
} Drawable;

typedef struct {
	int *current_board;
	int *next_board;
	int length;
	int width;
	int alive;
} Board;

typedef struct {
	Board board;
	Screen screen;
} Game;

void draw(xcb_connection_t *connection, Drawable window , xcb_gcontext_t foreground, Game *game, int size);
void update(Board *board);

Drawable create_window(xcb_connection_t *connection);
Game create_game(int length, int width);
void zoom(Screen *screen, float current_mouse_x, float current_mouse_y, float scale);

Vector ScreenToWorld(Screen *screen, float x, float y);
Vector WorldToScreen(Screen *screen, float x, float y);

void free_board(Board *board);
inline int wrap(int x, int max);
