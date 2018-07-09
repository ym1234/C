// FUCK XCB'S DOCUMENTATION

#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"

// TODO(ym): xcb wrapper for keyboard/mouse input.
int main() {
	xcb_connection_t *connection = xcb_connect(NULL, NULL);

	Drawable window = create_window(connection);
	Drawable back_buffer = { window.height, window.width, xcb_generate_id(connection) };
	xcb_create_pixmap(connection,xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root_depth, back_buffer.id, window.id, back_buffer.width, back_buffer.height);

	Game game = create_game(400, 300);

	// I hate having to do this, why can't I just use the background color of a graphics context for clearing?
	xcb_gcontext_t white_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, white_gc, window.id, XCB_GC_FOREGROUND, &(xcb_setup_roots_iterator(xcb_get_setup(connection)).data->white_pixel));

	xcb_gcontext_t black_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, black_gc, window.id, XCB_GC_FOREGROUND, &(xcb_setup_roots_iterator(xcb_get_setup(connection)).data->black_pixel));


	// TODO(ym): this is hacky af, and doesn't differentiate between holding and pressing
	Vector mouse_pos = {0};
	int mouse_1_pressed = 0;

	int done = 1;
	// TODO(ym): remove xcb_connection_has_error, commmunicate with the window manager (WM_DELETE_WINDOW) instead (you probably should read some of EHWM and ICWWM while you're at it).
	while(done && xcb_connection_has_error(connection) == 0) {
		xcb_generic_event_t *e;
		while((e = xcb_poll_for_event(connection))) {
			switch((e->response_type & ~0x80)) {
				// Should probably handle this in a configure event, but last time I tried it didin't work.
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
				} break;
				case XCB_MOTION_NOTIFY: {
					xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *) e;
					if(mouse_1_pressed) {
						 // NOTE: Both of these seem to work correctly, not sure which one is more accurate.
						 // (Probably the secocnd, but the first one is faster)
						game.screen.offset.x += ((float)(mouse_pos.x - ev->event_x)) / game.screen.scale;
						game.screen.offset.y += ((float)(mouse_pos.y - ev->event_y)) / game.screen.scale;
						/* Vector world_first = ScreenToWorld(&game.screen, ev->event_x, ev->event_y); */
						/* Vector world_second = ScreenToWorld(&game.screen, mouse_pos.x, mouse_pos.y); */
						/* game.screen.offset.x += world_second.x - world_first.x; */
						/* game.screen.offset.y += world_second.y - world_first.y; */
					}
					mouse_pos.x  = ev->event_x;
					mouse_pos.y  = ev->event_y;
				} break;
				case XCB_BUTTON_PRESS: {
					xcb_button_press_event_t *ev = (xcb_button_press_event_t *) e;
					switch(ev->detail) {
						case 1: {
							mouse_1_pressed = 1;
						} break;
						case 4: {
							zoom(&game.screen, ev->event_x, ev->event_y, 1.01);
						} break;
						case 5: {
							zoom(&game.screen, ev->event_x, ev->event_y, 0.99);
						} break;
					}
				} break;
				case XCB_BUTTON_RELEASE: {
					xcb_button_release_event_t *ev = (xcb_button_release_event_t *) e;
					if(ev->detail == 1) {
						mouse_1_pressed = 0;
					}
				} break;
				case XCB_KEY_PRESS: {
					xcb_key_press_event_t *ev = (xcb_key_press_event_t *)e;
					switch(ev->detail) {
						// TODO(ym): smooth keyboard panning?
						case 55:
						case 111: {
							game.screen.offset.y -= 20.0 / game.screen.scale;
						} break;
						case 54:
						case 116: {
							game.screen.offset.y += 20.0 / game.screen.scale;
						} break;
						case 44:
						case 113: {
							game.screen.offset.x -= 20.0 / game.screen.scale;
						} break;
						case 33:
						case 114: {
							game.screen.offset.x += 20.0 / game.screen.scale;
						} break;
						// Should zoom using the keyboard use mouse coords?
						case 48: {
							zoom(&game.screen, window.width / 2, window.height / 2, 0.99);
						} break;
						case 35: {
							zoom(&game.screen, window.width / 2, window.height / 2, 1.01);
						} break;
						case 53: {
							done = 0;
						} break;
					}
				} break;
			}
		}

		xcb_poly_fill_rectangle(connection, back_buffer.id, black_gc, 1, (xcb_rectangle_t[]) {{ 0, 0, back_buffer.width, back_buffer.height }});
		draw(connection, back_buffer, white_gc, &game, 5);
		// Why do I need to pass a gc here? doesn't seem to make a difference.
		xcb_copy_area(connection, back_buffer.id, window.id, black_gc, 0, 0, 0, 0, window.width, window.height);
		update(&(game.board));


		xcb_flush(connection);
		usleep(50000);
	}

	free_board(&game.board);

	// Do I need to free ~~gc~~ anything after my connection with the server is closed?
	xcb_free_gc(connection, black_gc);
	xcb_free_gc(connection, white_gc);
	xcb_free_pixmap(connection, back_buffer.id);

	xcb_disconnect(connection);
}

// Use a vector?
void zoom(Screen *screen, float focus_x, float focus_y, float scale) {
	Vector previous_focus = ScreenToWorld(screen, focus_x, focus_y);
	screen->scale *= scale;
	Vector new_focus = ScreenToWorld(screen, focus_x, focus_y);
	screen->offset.x += previous_focus.x - new_focus.x;
	screen->offset.y += previous_focus.y - new_focus.y;
}

void update(Board *board) {
	board->alive = 0;
	for(int i = 0; i < board->length; i++) {
		for(int j = 0; j < board->width; j++) {
			int neighbours =
				board->current_board[INDEX(i - 1, j - 1)] + board->current_board[INDEX(i - 1, j)] + board->current_board[INDEX(i - 1, j + 1)] +
				board->current_board[INDEX(i, j - 1)] 	  + board->current_board[INDEX(i, j + 1)] +
				board->current_board[INDEX(i + 1, j - 1)] + board->current_board[INDEX(i + 1, j)] + board->current_board[INDEX(i + 1,  j + 1)];

			if(board->current_board[INDEX(i, j)]) {
				board->next_board[INDEX(i, j)] = !(neighbours >  3 || neighbours < 2);
			} else {
				board->next_board[INDEX(i, j)] = neighbours == 3;
			}
			if(board->next_board[INDEX(i, j)]) {
				board->alive++;
			}
		}
	}

	int *temp = board->next_board;
	board->next_board = board->current_board;
	board->current_board = temp;
}


// TODO(ym): make the spacing between cells consistent, currently because of zoom floats are truncated so sizes & spaceing between the cells isn't consistent.
void draw(xcb_connection_t *connection, Drawable window , xcb_gcontext_t foreground, Game *game, int size) {
	xcb_rectangle_t *rects = malloc(sizeof(xcb_rectangle_t) * game->board.alive);

	int k = 0;
	for(int i = 0; i < game->board.length; i++) {
		for(int j = 0; j < game->board.width; j++) {
			if(game->board.current_board[i * game->board.width + j]) {
				Vector pos = WorldToScreen(&game->screen, j * size, i * size);
				if((pos.x + size * game->screen.scale > 0 && pos.y + size * game->screen.scale > 0) &&
						(pos.x - size * game->screen.scale < window.width && pos.y - size * game->screen.scale < window.height) ) {
					rects[k++] = (xcb_rectangle_t) { pos.x, pos.y, size * game->screen.scale, size * game->screen.scale };
				}
			}
		}
	}

	xcb_poly_fill_rectangle(connection, window.id, foreground, k, rects);
	free(rects);
}

void free_board(Board *board) {
	free(board->current_board);
	free(board->next_board);
}

// (This comment applies for all create/init functions)
// Allocate on the stack and copy? or on the heap and return a pointer? OR make the user pass a pointer and the set the value of it to whatever you want?
Drawable create_window(xcb_connection_t *connection) {
	Drawable window = { 400, 400, xcb_generate_id(connection) };
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

	xcb_create_window(
			connection, XCB_COPY_FROM_PARENT, window.id, screen->root,
			0, 0,
			1980, 1080,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, (int[]) { screen->black_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION });

	xcb_map_window(connection, window.id);
	return window;
}

Game create_game(int length, int width) {
    Board board = { malloc(length * width * sizeof(int)), malloc(length * width * sizeof(int)), length, width, 0 };
	for(int i = 0; i < board.length; i++) {
		for(int j = 0; j < board.width; j++) {
			if((board.current_board[i  * board.width + j] = rand() % 2)) {
				board.alive++;
			}
		}
	}
	return (Game) { board, { { 0, 0 }, 1 } };
}


// Use a vector?
Vector ScreenToWorld(Screen *screen, float x, float y) {
	return (Vector) { (x  / screen->scale) + screen->offset.x, (y / screen->scale) + screen->offset.y};
}

Vector WorldToScreen(Screen *screen, float x, float y) {
	return (Vector) { (x - screen->offset.x) * screen->scale, (y - screen->offset.y) * screen->scale };
}
