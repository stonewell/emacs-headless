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

struct so_display_info *x_display_info_for_name (Lisp_Object name) {
  struct so_display_info *dpyinfo;

  CHECK_STRING (name);

  for (dpyinfo = &one_so_display_info; dpyinfo; dpyinfo = dpyinfo->next)
    if (!NILP (Fstring_equal (XCAR (dpyinfo->name_list_element), name)))
      return dpyinfo;

  /* Use this general default value to start with.  */
  Vx_resource_name = Vinvocation_name;

  validate_x_resource_name ();

  dpyinfo = so_term_init (name, NULL, SSDATA (Vx_resource_name));

  if (dpyinfo == 0)
    error ("Cannot connect to server %s", SDATA (name));

  XSETFASTINT (Vwindow_system_version, so_major_version);

  return dpyinfo;
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

struct so_display_info *
so_term_init (Lisp_Object display_name, char *xrm_option, char *resource_name)
{
  struct so_display_info *dpyinfo;
  struct terminal *terminal;

  block_input ();

  if (!so_initialized)
    {
      so_initialize ();
      so_initialized = 1;
    }

  so_initialize_display_info (display_name);

  dpyinfo = &one_so_display_info;
  terminal = so_create_terminal (dpyinfo);

  /* Set the name of the terminal. */
  terminal->name = xlispstrdup (display_name);

  dpyinfo->xrdb = xrm_option ? so_make_rdb (xrm_option) : NULL;

  /* Put this display on the chain.  */
  dpyinfo->next = x_display_list;
  x_display_list = dpyinfo;

  /* initialize palette with white and black */
  {
    XColor color;
    so_defined_color (0, "white", &color, 1);
    so_defined_color (0, "black", &color, 1);
  }

  /* Create Fringe Bitmaps and store them for later use.

     On W32, bitmaps are all unsigned short, as Windows requires
     bitmap data to be Word aligned.  For some reason they are
     horizontally reflected compared to how they appear on X, so we
     need to bitswap and convert to unsigned shorts before creating
     the bitmaps.  */
  so_init_fringe (terminal->rif);

  unblock_input ();

  return dpyinfo;
}
