/* Headless window system support
   Copyright (C) 2021-2022 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include "dispextern.h"
#include "frame.h"
#include "lisp.h"
#include "headlessgui.h"
#include "keyboard.h"
#include "headlessterm.h"
#include "blockinput.h"
#include "termchar.h"
#include "termhooks.h"
#include "menu.h"
#include "buffer.h"
#include "thread.h"
#include "window.h"

#include <math.h>
#include <stdlib.h>

/* The single Headless display (if any).  */
struct headless_display_info *x_display_list;

/* This is used to determine when to evict the font lookup cache,
   which we do every 50 updates.  */
static int up_to_date_count;

/* List of defined fringe bitmaps.  */
static void **fringe_bmps;

/* The amount of fringe bitmaps in that list.  */
static int max_fringe_bmp;

/* Alist of resources to their values.  */
static Lisp_Object rdb;

/* Non-zero means that a HELP_EVENT has been generated since Emacs
   start.  */
static bool any_help_event_p;

char *
get_keysym_name (int keysym)
{
  static char value[16];
  sprintf (value, "%d", keysym);
  return value;
}

struct headless_display_info *
headless_term_init (void)
{
  struct headless_display_info *dpyinfo;

  block_input ();

  Fset_input_interrupt_mode (Qt);
  dpyinfo = xzalloc (sizeof *dpyinfo);

  return dpyinfo;
}

unsigned long
headless_get_pixel (headless img, int x, int y)
{
  (void)img;
  (void)x;
  (void)y;

  return 0;
}

void
headless_draw_cross_on_pixmap (Emacs_Pixmap pixmap,
			 int x, int y, unsigned int width, unsigned int height,
			 unsigned long color)
{
  (void)pixmap;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)color;
}

void
headless_put_pixel (headless bitmap, int x, int y, unsigned long pixel)
{
}

void
mark_headless_display (void)
{
  if (x_display_list)
    {
      mark_object (x_display_list->color_map);
      mark_object (x_display_list->default_name);
    }
}

void
syms_of_headlessterm (void)
{
  DEFVAR_BOOL ("headless-initialized", headless_initialized,
     doc: /* Non-nil if the Headless terminal backend has been initialized.  */);

  DEFVAR_BOOL ("x-use-underline-position-properties",
	       x_use_underline_position_properties,
     doc: /* SKIP: real doc in xterm.c.  */);
  x_use_underline_position_properties = 1;

  DEFVAR_BOOL ("x-underline-at-descent-line",
	       x_underline_at_descent_line,
     doc: /* SKIP: real doc in xterm.c.  */);
  x_underline_at_descent_line = 0;

  DEFVAR_LISP ("x-toolkit-scroll-bars", Vx_toolkit_scroll_bars,
     doc: /* SKIP: real doc in xterm.c.  */);
  Vx_toolkit_scroll_bars = Qt;

  DEFVAR_BOOL ("headless-debug-on-fatal-error", headless_debug_on_fatal_error,
     doc: /* If non-nil, Emacs will launch the system debugger upon a fatal error.  */);
  headless_debug_on_fatal_error = 1;

  DEFSYM (Qshift, "shift");
  DEFSYM (Qcontrol, "control");
  DEFSYM (Qoption, "option");
  DEFSYM (Qcommand, "command");

  DEFSYM (Qdata_directory, "data-directory");

  DEFVAR_LISP ("headless-meta-keysym", Vheadless_meta_keysym,
     doc: /* Which key Emacs uses as the meta modifier.
This is either one of the symbols `shift', `control', `command', and
`option', or nil, in which case it is treated as `command'.

Setting it to any other value is equivalent to `command'.  */);
  Vheadless_meta_keysym = Qnil;

  DEFVAR_LISP ("headless-control-keysym", Vheadless_control_keysym,
     doc: /* Which key Emacs uses as the control modifier.
This is either one of the symbols `shift', `control', `command', and
`option', or nil, in which case it is treated as `control'.

Setting it to any other value is equivalent to `control'.  */);
  Vheadless_control_keysym = Qnil;

  DEFVAR_LISP ("headless-super-keysym", Vheadless_super_keysym,
     doc: /* Which key Emacs uses as the super modifier.
This is either one of the symbols `shift', `control', `command', and
`option', or nil, in which case it is treated as `option'.

Setting it to any other value is equivalent to `option'.  */);
  Vheadless_super_keysym = Qnil;

  DEFVAR_LISP ("headless-shift-keysym", Vheadless_shift_keysym,
     doc: /* Which key Emacs uses as the shift modifier.
This is either one of the symbols `shift', `control', `command', and
`option', or nil, in which case it is treated as `shift'.

Setting it to any other value is equivalent to `shift'.  */);
  Vheadless_shift_keysym = Qnil;

  DEFSYM (Qx_use_underline_position_properties,
	  "x-use-underline-position-properties");

  DEFSYM (Qx_underline_at_descent_line, "x-underline-at-descent-line");

  rdb = Qnil;
  staticpro (&rdb);

  Fprovide (Qheadless, Qnil);
}
