#include <config.h>

#include "frame.h"
#include "service_only_term.h"

struct so_display_info one_so_display_info;
struct so_display_info *x_display_list;

int
x_display_pixel_height (struct so_display_info *dpyinfo)
{
  return 0;
}

int
x_display_pixel_width (struct so_display_info *dpyinfo)
{
  return 0;
}

Pixmap so_create_pixmap(Display *display,
                        void * d,
                        char *data, unsigned int width,
                        unsigned int height,
                        unsigned long fg,
                        unsigned long bg, unsigned int depth) {
  return 0;
}

unsigned long
so_get_pixel(XImagePtr ximage, int x, int y) {
  return 0;
}

void
so_put_pixel(XImagePtr ximage, int x, int y, unsigned long pixel) {
}

void
so_free_pixmap(XImagePtr ximage) {
}

void
x_set_offset (struct frame *f, register int xoff, register int yoff,
	      int change_gravity) {
}

void
x_focus_frame (struct frame *f, bool noactivate) {
}

Lisp_Object
x_new_font (struct frame *f, Lisp_Object font_object, int fontset) {
  return 0;
}

void
x_set_scroll_bar_default_width (struct frame *f) {
}

void
x_set_scroll_bar_default_height (struct frame *f) {
}

char *
x_get_string_resource (XrmDatabase rdb, const char *name, const char *class) {
  return 0;
}

void x_clear_under_internal_border (struct frame *f) {
}

void x_implicitly_set_name (struct frame * f, Lisp_Object arg, Lisp_Object oldval) {
}

struct so_display_info *x_display_info_for_name (Lisp_Object ag) {
  return 0;
}

bool x_bitmap_icon (struct frame * f, Lisp_Object arg) {
  return false;
}

char *x_get_keysym_name (int key) {
  return 0;
}

void
x_query_colors (struct frame *f, XColor *colors, int ncolors) {
}

int
so_defined_color (struct frame *f, const char *color, XColor *color_def,
                  bool alloc_p) {
  return 0;
}

void syms_of_soterm (void) {
}
