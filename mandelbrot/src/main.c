#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb.h>

typedef struct {
	int height;
	int width;
	xcb_window_t id;
} Drawable;

typedef struct {
	float x;
	float y;
} Vector;

Drawable create_window(xcb_connection_t *connection);
void render_mandelbrot(uint8_t *image, int width, int height, Vector start, Vector end, int max_iteration);
Vector zoom(double *zoom, Vector *offset, Vector focus_point, double amount);
Vector screen_to_world(double zoom, Vector offset, Vector point);

#define WIDTH 600
#define HEIGHT 600
#define QUALITY 50
int main() {

	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	Drawable window = create_window(connection);
	xcb_image_t *image = xcb_image_create(
			WIDTH, HEIGHT, XCB_IMAGE_FORMAT_Z_PIXMAP,
			8, 24, 32, 0,
			XCB_IMAGE_ORDER_MSB_FIRST, XCB_IMAGE_ORDER_MSB_FIRST, // No idea what these do lol
			NULL, WIDTH * HEIGHT * 4, NULL);

	xcb_gcontext_t gc = xcb_generate_id(connection);
	xcb_create_gc(connection, gc, window.id, XCB_GC_BACKGROUND , (int[]) { xcb_setup_roots_iterator(xcb_get_setup(connection)).data->white_pixel } );

	Vector start = { -2.5, -1.0 };
	Vector end = { 1.0, 1.0 };

	int done = 0;
	double scale = 1;
	Vector offset = {0};
	while(!done && !xcb_connection_has_error(connection)) {
		xcb_generic_event_t *e = NULL;
		int changed = 0;
		while((e = xcb_poll_for_event(connection))) {

			switch((e->response_type & ~0x80)) {
				case 0: {
					xcb_generic_error_t *error = (xcb_generic_error_t *) e;
					printf("Error code: %d\n", error->error_code);
				} break;
				case XCB_MOTION_NOTIFY: {
				} break;
				case XCB_BUTTON_PRESS: {
				} break;
				case XCB_CONFIGURE_NOTIFY: {
				} break;
				case XCB_KEY_PRESS: {
					xcb_key_press_event_t *ev = (xcb_key_press_event_t *) e;
					changed = 1;
					switch(ev->detail) {
						case 53: {
							done = 1;
						} break;
						case 44: { // h, left
							offset.y -= 0.2 / scale;
						} break;
						case 33: {// l, right
							offset.y += 0.2 / scale;
						} break;
						case 54: {// j, down
							offset.x += 0.2 / scale;
						} break;
						case 55: {// k, up
							offset.x -= 0.2 / scale;
						} break;
						case 35: { // plus
							scale *= 1.01;
						} break;
						case 48: { // minus
							scale *= 0.99;
						} break;
					}
				} break;
			}
			free(e);
		}

		if(changed || !done) {
			render_mandelbrot(image->data, WIDTH, HEIGHT, (Vector) { (start.x / scale) + offset.x, (start.y / scale ) + offset.y}, (Vector) { (end.x / scale) + offset.x, (end.y / scale) + offset.y}, QUALITY);
		}
		xcb_image_put(connection, window.id, gc, image, 0, 0, 0);
		xcb_flush(connection);
		usleep(5000);
	}
}

void render_mandelbrot(uint8_t *image, int width, int height, Vector start, Vector end, int max_iteration) {
	static double scale = 0;
	scale = fabs(start.x - end.x) / (double) width;

	double current_x = start.x;
	double current_y = start.y;
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			double x = 0.0;
			double y = 0.0;

			int iterator = 0;

			while(x * x + y * y < 2 * 2 && iterator < max_iteration) {
				double xtemp = x * x - y * y + current_y;
				y = 2 * x * y + current_x;
				x = xtemp;
				++iterator;
			}

			int val = 0xFF * ((float)iterator / max_iteration);
			*image++ = val;
			*image++ = val;
			*image++ = val;
			(void)++image;
			current_y += scale;
		}
		current_y = start.y;
		current_x += scale;
	}
}

Drawable create_window(xcb_connection_t *connection) {
	Drawable window = { 400, 400, xcb_generate_id(connection) };
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

	xcb_create_window(
			connection, 24, window.id, screen->root,
			0, 0,
			400, 400,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
			(int[]) { screen->black_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION });

	xcb_map_window(connection, window.id);
	return window;
}
