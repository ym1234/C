#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcb/xproto.h>
#include <xcb/xcb.h>

void draw(xcb_connection_t *, xcb_window_t, xcb_gcontext_t, xcb_gcontext_t, int, int *, int, int);
int clear(xcb_gcontext_t gc, xcb_connection_t *connection, xcb_window_t window, int width, int height);
int wrap(int x, int max);

int main() {
	xcb_connection_t *connection = xcb_connect(NULL, NULL);

	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	xcb_window_t  window = xcb_generate_id(connection);

	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			0, 0,
			400, 400,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, (int[]) { screen->black_pixel, XCB_EVENT_MASK_EXPOSURE });
			/* XCB_CW_EVENT_MASK, (int[]) {XCB_EVENT_MASK_EXPOSURE }); */

	xcb_map_window(connection, window);
	xcb_flush(connection);

	xcb_gcontext_t white_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, white_gc, window, XCB_GC_FOREGROUND, &(screen->white_pixel));

	xcb_gcontext_t black_gc = xcb_generate_id(connection);
	xcb_create_gc(connection, black_gc, window, XCB_GC_FOREGROUND, &(screen->black_pixel));

	int width = 300;
	int length = 300;

	int *gol = malloc(length * width * sizeof(int));
	int *gol_out = malloc(length * width * sizeof(int));

	for(int i = 0; i < length; i++) {
		for(int j = 0; j < width; j++) {
			gol[i * width + j] = rand() % 2;
		}
	}

	int window_width = 400;
	int window_height = 400;

	int done = 1;
	while(done && xcb_connection_has_error(connection) == 0) {

		xcb_generic_event_t *e;
		if((e = xcb_poll_for_event(connection))) {
			switch((e->response_type & ~0x80)) {
				case XCB_EXPOSE: {
					xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, window);
					xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(connection, cookie, NULL);

					window_height = reply->height;
					window_width = reply->width;

					free(reply);
					/* clear(black_gc, connection, window, window_width, window_height); */
					break;
				}
				case XCB_MOTION_NOTIFY: {
					xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *) e;
					printf("X: %d, Y: %d, Time: %d.\n", ev->event_x, ev->event_y, ev->time);
					break;
				}
			}
		}

		draw(connection, window, white_gc, black_gc, 5, gol, length, width);

		for(int i = 1; i < length - 1; i++) {
			for(int j = 1; j < width - 1; j++) {
				int neighbours = gol[(i - 1) * width + (j - 1)] + gol[(i - 1) * width + j] + gol[(i - 1) * width + (j + 1)] +
					gol[i * width + (j - 1)] + gol[i * width + (j + 1)] +
					gol[(i + 1) * width + (j - 1)] + gol[(i + 1) * width + j] + gol[(i + 1) * width + (j + 1)];

				if(gol[i * width + j]) {
					gol_out[i * width + j] = !(neighbours >  3 || neighbours < 2);
				} else {
					gol_out[i * width + j] = neighbours == 3;
				}
			}
		}

		int *temp = gol_out;
		gol_out = gol;
		gol = temp;

		xcb_flush(connection);
		usleep(20000);
	}

	free(gol);
	free(gol_out);

	xcb_disconnect(connection);
}

int wrap(int x, int max) {
	return (x % max) + max;
}

void draw(xcb_connection_t *connection, xcb_window_t window, xcb_gcontext_t foreground, xcb_gcontext_t background, int size, int *gol, int length, int width) {
	// Super lazy mode
	for(int i = 1; i < length - 1; i++) {
		for(int j = 1; j < width - 1; j++) {
			xcb_poly_fill_rectangle(connection, window, gol[i * width + j] ? foreground : background, 1, (xcb_rectangle_t[]) { i * size, j * size, size, size });
		}
	}
}

int clear(xcb_gcontext_t gc, xcb_connection_t *connection, xcb_window_t window, int width, int height) {
	xcb_poly_fill_rectangle(connection, window, gc, 1, (xcb_rectangle_t[]) { 0, 0, width, height});
}
