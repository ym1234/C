#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcb/xproto.h>
#include <xcb/xcb.h>

// There is probably an easier/faster way to do this with one-dimensional arrays, but too lazy to think about it.
#define INDEX(i, j) wrap(i, game.length) * game.width + wrap(j, game.width)

// Couple of structs to make my life easier
// Should all tuples(height width, length width) be vectors?
typedef struct {
	float x;
	float y;
} Vector;

typedef struct {
	int height;
	int width;
	xcb_window_t id;
} Window;

typedef struct {
	Vector offset;
	float scale;
} Screen;

typedef struct {
	int *current_board;
	int *next_board;
	int length;
	int width;
} Game;

void draw(xcb_connection_t *connection, xcb_window_t window, xcb_gcontext_t foreground, xcb_gcontext_t background, int size, Game game, Screen *screen);
int clear(xcb_gcontext_t gc, xcb_connection_t *connection, Window window);
Window init_window(xcb_connection_t *connection);
int wrap(int x, int max);

Vector ScreenToWorld(Screen *screen, float x, float y);
Vector WorldToScreen(Screen *screen, int x, int y);

int main() {
	xcb_connection_t *connection = xcb_connect(NULL, NULL);

	Window window = init_window(connection);

	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	// Does {0, <init other stuff>} work? idk.
	xcb_gcontext_t white_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, white_gc, window.id, XCB_GC_FOREGROUND, &(screen->white_pixel));

	xcb_gcontext_t black_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, black_gc, window.id, XCB_GC_FOREGROUND, &(screen->black_pixel));


	// Have to allocate on the heap because of some C array stupidness
	// (I could also do something like this but I haven't tried and allocating on the heap works soooo¯\_(ツ)_/¯)
	// game.current_board = (int *) game.current_board;
    Game game = { .current_board = malloc(300 * 300 * sizeof(int)), .next_board = malloc(300 * 300 * sizeof(int)), .length = 300, .width = 300 };
	Screen screen2 = { .offset = { 0, 0 }, 1 };

	for(int i = 0; i < game.length; i++) {
		for(int j = 0; j < game.width; j++) {
			game.current_board[INDEX(i, j)] = rand() % 2;
		}
	}

	int done = 1;
	while(done && xcb_connection_has_error(connection) == 0) {

		xcb_generic_event_t *e;
		while((e = xcb_poll_for_event(connection))) {
			switch((e->response_type & ~0x80)) {
				case XCB_EXPOSE: {
					xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, window.id);
					xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(connection, cookie, NULL);

					window.height = reply->height;
					window.width = reply->width;

					free(reply);
					/* clear(black_gc, connection, window, window_width, window_height); */
					break;
				}
				case XCB_MOTION_NOTIFY: {
					xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *) e;
					printf("X: %d, Y: %d, Time: %d.\n", ev->event_x, ev->event_y, ev->time);
					break;
				}
				case XCB_KEY_PRESS: {
					xcb_key_press_event_t *ev = (xcb_key_press_event_t *)e;
					switch(ev->detail) {
						case 55:
						case 111: {
							Vector before = WorldToScreen(&screen2, 0, 0);
							screen2.offset.y -= 10.0;
							xcb_poly_fill_rectangle(connection, window.id, black_gc, 1, (xcb_rectangle_t[]) { before.x, before.y, window.width, 10 });
							/* clear(black_gc, connection, window); */
							break;
						}
						case 54:
						case 116: {
							screen2.offset.y += 10.0;
							Vector before = WorldToScreen(&screen2, 0, (game.length - 1) * 5);
							xcb_poly_fill_rectangle(connection, window.id, black_gc, 1, (xcb_rectangle_t[]) { before.x, before.y, window.width, 10 });
							/* clear(black_gc, connection, window); */
							break;
						}
						case 44:
						case 113: {
							screen2.offset.x -= 10.0;
							Vector before = WorldToScreen(&screen2, 0, 0);
							xcb_poly_fill_rectangle(connection, window.id, black_gc, 1, (xcb_rectangle_t[]) { before.x - 5, before.y, 10, window.height });
							/* clear(black_gc, connection, window); */
							break;
						}
						case 33:
						case 114: {
							Vector before = WorldToScreen(&screen2,(game.width - 1) * 5, 0);
							screen2.offset.x += 10.0;
							xcb_poly_fill_rectangle(connection, window.id, black_gc, 1, (xcb_rectangle_t[]) { before.x, before.y, 10, window.height });
							/* clear(black_gc, connection, window); */
							break;
						}
						case 48: {
							screen2.scale *= 0.99;
							break;
						}
						case 35: {
							screen2.scale *= 1.01;
							break;
						}
						case 31: {
							xcb_poly_fill_rectangle(connection, window.id, black_gc, 1, (xcb_rectangle_t[]) { 0, 0, window.width, window.height });
							break;
						}

					}
					break;
				}
			}
		}

		draw(connection, window.id, white_gc, black_gc, 5, game, &screen2);

		for(int i = 0; i < game.length; i++) {
			for(int j = 0; j < game.width; j++) {
				// Yay that worked from first time
				int neighbours =
					game.current_board[INDEX(i - 1, j - 1)] + game.current_board[INDEX(i - 1, j)] + game.current_board[INDEX(i - 1, j + 1)] +
					game.current_board[INDEX(i, j - 1)] + game.current_board[INDEX(i, j + 1)] +
					game.current_board[INDEX(i + 1, j - 1)] + game.current_board[INDEX(i + 1, j)] + game.current_board[INDEX(i + 1,  j + 1)];

				if(game.current_board[INDEX(i, j)]) {
					game.next_board[INDEX(i, j)] = !(neighbours >  3 || neighbours < 2);
				} else {
					game.next_board[INDEX(i, j)] = neighbours == 3;
				}
			}
		}

		int *temp = game.next_board;
		game.next_board = game.current_board;
		game.current_board = temp;

		xcb_flush(connection);
		usleep(100000);
	}

	free(game.current_board);
	free(game.next_board);

	xcb_disconnect(connection);
}

Window init_window(xcb_connection_t *connection) {
	Window window = {0, .id = xcb_generate_id(connection) };
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window.id,
			screen->root,
			0, 0,
			400, 400,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, (int[]) { screen->black_pixel,  XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE });
			/* XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, (int[]) { screen->black_pixel, XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE }); */
			/* XCB_CW_EVENT_MASK, (int[]) {XCB_EVENT_MASK_EXPOSURE }); */

	xcb_map_window(connection, window.id);
	xcb_flush(connection);
	return window;
}

int wrap(int x, int max) {
	return (x + max) % max;
}

// Just pass game by value
void draw(xcb_connection_t *connection, xcb_window_t window, xcb_gcontext_t foreground, xcb_gcontext_t background, int size, Game game, Screen *screen) {
	// Super lazy mode
	for(int i = 1; i < game.length - 1; i++) {
		for(int j = 1; j < game.width - 1; j++) {
			Vector pos = WorldToScreen(screen, i * size, j * size);
			xcb_poly_fill_rectangle(connection, window, game.current_board[i * game.width + j] ? foreground : background, 1, (xcb_rectangle_t[]) {pos.x, pos.y, size, size });
		}
	}
}

Vector ScreenToWorld(Screen *screen, float x, float y) {
	Vector vec = { (x + screen->offset.x) / screen->scale, (y + screen->offset.y) / screen->scale };
	return vec;
}

Vector WorldToScreen(Screen *screen, int x, int y) {
	Vector vec = { (x - screen->offset.x) * screen->scale, (y - screen->offset.y) * screen->scale };
	return vec;
}

int clear(xcb_gcontext_t gc, xcb_connection_t *connection, Window window) {
	xcb_poly_fill_rectangle(connection, window.id, gc, 1, (xcb_rectangle_t[]) { 0, 0, window.width, window.height});
}
