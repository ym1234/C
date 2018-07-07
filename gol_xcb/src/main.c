// FUCK XCB'S DOCUMENTATION

#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"


int main() {

	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	Drawable window = create_window(connection);

	Game game = create_game(300, 300);

	Drawable back_buffer = { game.board.length * 5, game.board.width * 5, xcb_generate_id(connection) };
	xcb_create_pixmap(connection,xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root_depth, back_buffer.id, window.id, back_buffer.width, back_buffer.height);

	// i hate having to do this, why can't i just use the background color of a graphics context for clearing?
	xcb_gcontext_t white_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, white_gc, window.id, XCB_GC_FOREGROUND, &(xcb_setup_roots_iterator(xcb_get_setup(connection)).data->white_pixel));

	xcb_gcontext_t black_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, black_gc, window.id, XCB_GC_FOREGROUND, &(xcb_setup_roots_iterator(xcb_get_setup(connection)).data->black_pixel));

	int done = 1;
	while(done && xcb_connection_has_error(connection) == 0) {
		xcb_generic_event_t *e;
		while((e = xcb_poll_for_event(connection))) {
			switch((e->response_type & ~0x80)) {
				// Should probably handle this in a configure event, but last time a tried it didin't work.
				case XCB_EXPOSE: {
					xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, window.id);
					xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(connection, cookie, NULL);

					if(window.height != reply->height || window.width != reply->width) {
						window.height = reply->height;
						window.width = reply->width;
						xcb_free_pixmap(connection, back_buffer.id);

						back_buffer.id = xcb_generate_id(connection);
						back_buffer.width = reply->width;
						back_buffer.height = reply->height;

						xcb_create_pixmap(connection, xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root_depth, back_buffer.id, window.id, back_buffer.width, back_buffer.height);
					}

					free(reply);
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
							game.screen.offset.y -= 20.0;
						} break;
						case 54:
						case 116: {
							game.screen.offset.y += 20.0;
						} break;
						case 44:
						case 113: {
							game.screen.offset.x -= 20.0;
						} break;
						case 33:
						case 114: {
							game.screen.offset.x += 20.0;
						} break;
						case 48: {
							game.screen.scale *= 0.99;
						} break;
						case 35: {
							game.screen.scale *= 1.01;
						} break;
						case 31: {
							xcb_clear_area(connection, 0, window.id, 0, 0, window.width, window.height);
						} break;
						case 53: {
							done = 0;
						} break;
					}
					break;
				}
			}
		}

		draw(connection, back_buffer, white_gc, black_gc, &game, 5);
		xcb_copy_area(connection, back_buffer.id, window.id, white_gc, 0, 0, 0, 0, window.width, window.height);
		update(&(game.board));


		xcb_flush(connection);
		usleep(50000);
	}

	free_board(&(game.board));
	// Do I need to free the gc after my connection with the server is closed?
	xcb_free_gc(connection, black_gc);
	xcb_free_gc(connection, white_gc);

	xcb_disconnect(connection);
}

void update(Board *board) {
	board->alive = 0;
	for(int i = 0; i < board->length; i++) {
		for(int j = 0; j < board->width; j++) {
			int neighbours =
				board->current_board[INDEX(i - 1, j - 1)] + board->current_board[INDEX(i - 1, j)] + board->current_board[INDEX(i - 1, j + 1)] +
				board->current_board[INDEX(i, j - 1)] + board->current_board[INDEX(i, j + 1)] +
				board->current_board[INDEX(i + 1, j - 1)] + board->current_board[INDEX(i + 1, j)] + board->current_board[INDEX(i + 1,  j + 1)];

			if(board->current_board[INDEX(i, j)]) {
				board->next_board[INDEX(i, j)] = !(neighbours >  3 || neighbours < 2);
			} else {
				board->next_board[INDEX(i, j)] = neighbours == 3;
			}
			if(board->current_board[INDEX(i, j)]) {
				board->alive++;
			}
		}
	}

	int *temp = board->next_board;
	board->next_board = board->current_board;
	board->current_board = temp;
}


void draw(xcb_connection_t *connection, Drawable window , xcb_gcontext_t foreground, xcb_gcontext_t background, Game *game, int size) {
	xcb_rectangle_t *rects = malloc(sizeof(xcb_rectangle_t) * game->board.alive);
	/* printf("alive: %d\n", game->alive); */
	/* Vector whatever3 = ScreenToWorld(screen, 0, 0); */
	/* Vector whatever4 = ScreenToWorld(screen, window.width, window.height); */
	/* printf("0x: %d, 0y: %d.\n", whatever3.x, whatever3.y); */
	/* printf("max x: %d, max y: %d.\n", whatever4.x, whatever4.y); */

	int k = 0;
	for(int i = 0; i < game->board.length; i++) {
		for(int j = 0; j < game->board.width; j++) {
			if(game->board.current_board[i * game->board.width + j]) {
				if(k < game->board.alive) {
					Vector pos = WorldToScreen(&game->screen, i * size, j * size);
					xcb_rectangle_t rect = { round(pos.x), round(pos.y), round(size * game->screen.scale), round(size * game->screen.scale) };
					rects[k++] = rect;
				}
			}
		}
	}

	/* xcb_clear_area(connection, 0, window.id, 0, 0, window.width, window.height); */
	xcb_poly_fill_rectangle(connection, window.id, background, 1, (xcb_rectangle_t[]) {{ 0, 0, window.width, window.height }});
	xcb_poly_fill_rectangle(connection, window.id, foreground, game->board.alive, rects);

	free(rects);
}

void free_board(Board *board) {
	free(board->current_board);
	free(board->next_board);
}

// (This comment applies for all create/init functions)
// Allocate on the stack and copy? or on the heap and return a pointer?
Drawable create_window(xcb_connection_t *connection) {
	Drawable window = {0, .id = xcb_generate_id(connection) };
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

	xcb_create_window(
			connection, XCB_COPY_FROM_PARENT, window.id, screen->root,
			0, 0,
			400, 400,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, (int[]) { screen->black_pixel,  XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE });
			/* XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, (int[]) { screen->black_pixel, XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE }); */
			/* XCB_CW_EVENT_MASK, (int[]) {XCB_EVENT_MASK_EXPOSURE }); */

	xcb_map_window(connection, window.id);
	return window;
}

Game create_game(int length, int width) {
    Board board = { malloc(length * width * sizeof(int)), malloc(length * width * sizeof(int)), length, width, 0 };
	for(int i = 0; i < board.length; i++) {
		for(int j = 0; j < board.width; j++) {
			board.current_board[i  * board.width + j] = rand() % 2;
			if(board.current_board[i * board.width + j]) {
				board.alive++;
			}
		}
	}
	return (Game) { board, { { 0, 0 }, 1 } };
}


Vector ScreenToWorld(Screen *screen, float x, float y) {
	Vector vec = { (x  / screen->scale) + screen->offset.x, (y / screen->scale) + screen->offset.y};
	return vec;
}

Vector WorldToScreen(Screen *screen, float x, float y) {
	Vector vec = { (x - screen->offset.x) * screen->scale, (y - screen->offset.y) * screen->scale };
	return vec;
}

int wrap(int x, int max) {
	return (x + max) % max;
}
