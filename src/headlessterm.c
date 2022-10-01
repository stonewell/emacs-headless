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
  TRACE_FUNC_CALL;
  static char value[16];
  sprintf (value, "%d", keysym);
  return value;
}

static const char *
headless_get_string_resource (void *ignored, const char *name,
                              const char *class)
{
  const char *native;
  TRACE_FUNC_CALL;

  if (!name)
    return NULL;

  Lisp_Object lval = assoc_no_quit (build_string (name), rdb);

  if (!NILP (lval))
    return SSDATA (XCDR (lval));

  /* if ((native = be_find_setting (name))) */
  /*   return native; */

  return NULL;
}

static struct redisplay_interface headless_redisplay_interface =
  {
    headless_frame_parm_handlers,
    gui_produce_glyphs,
    gui_write_glyphs,
    gui_insert_glyphs,
    gui_clear_end_of_line,
    NULL, /* headless_scroll_run, */
    NULL, /* headless_after_update_window_line, */
    NULL, /* update_window_begin */
    NULL, /* update_window_end */
    NULL, /* headless_flush, */
    gui_clear_window_mouse_face,
    gui_get_glyph_overhangs,
    gui_fix_overlapping_area,
    NULL, /* headless_draw_fringe_bitmap, */
    NULL, /* headless_define_fringe_bitmap, */
    NULL, /* headless_destroy_fringe_bitmap, */
    NULL, /* headless_compute_glyph_string_overhangs, */
    NULL, /* headless_draw_glyph_string, */
    NULL, /* headless_define_frame_cursor, */
    NULL, /* headless_clear_frame_area, */
    NULL, /* headless_clear_under_internal_border, */
    NULL, /* headless_draw_window_cursor, */
    NULL, /* headless_draw_vertical_window_border, */
    NULL, /* headless_draw_window_divider, */
    NULL, /* shift glyphs for insert */
    NULL, /* headless_show_hourglass, */
    NULL, /* headless_hide_hourglass, */
    NULL, /* headless_default_font_parameter, */
  };

static struct terminal *
headless_create_terminal (struct headless_display_info *dpyinfo)
{
  struct terminal *terminal;

  terminal = create_terminal (output_headless, &headless_redisplay_interface);

  terminal->display_info.headless = dpyinfo;
  dpyinfo->terminal = terminal;
  terminal->kboard = allocate_kboard (Qheadless);

  /* terminal->iconify_frame_hook = headless_iconify_frame; */
  /* /terminal->focus_frame_hook = headless_focus_frame; */
  /* terminal->ring_bell_hook = headless_beep; */
  /* terminal->popup_dialog_hook = headless_popup_dialog; */
  /* terminal->frame_visible_invisible_hook = headless_set_frame_visible_invisible; */
  /* terminal->set_frame_offset_hook = headless_set_offset; */
  /* terminal->delete_terminal_hook = headless_delete_terminal; */
  terminal->get_string_resource_hook = headless_get_string_resource;
  /* terminal->set_new_font_hook = headless_new_font; */
  /* terminal->defined_color_hook = headless_defined_color; */
  /* terminal->set_window_size_hook = headless_set_window_size; */
  /* terminal->read_socket_hook = headless_read_socket; */
  /* terminal->implicit_set_name_hook = headless_implicitly_set_name; */
  /* terminal->mouse_position_hook = headless_mouse_position; */
  /* terminal->delete_frame_hook = headless_delete_window; */
  /* terminal->frame_up_to_date_hook = headless_frame_up_to_date; */
  /* terminal->buffer_flipping_unblocked_hook = headless_buffer_flipping_unblocked_hook; */
  /* terminal->clear_frame_hook = headless_clear_frame; */
  /* terminal->change_tab_bar_height_hook = headless_change_tab_bar_height; */
  /* terminal->change_tool_bar_height_hook = headless_change_tool_bar_height; */
  /* terminal->set_vertical_scroll_bar_hook = headless_set_vertical_scroll_bar; */
  /* terminal->set_horizontal_scroll_bar_hook = headless_set_horizontal_scroll_bar; */
  /* terminal->set_scroll_bar_default_height_hook = headless_set_scroll_bar_default_height; */
  /* terminal->set_scroll_bar_default_width_hook = headless_set_scroll_bar_default_width; */
  /* terminal->judge_scroll_bars_hook = headless_judge_scroll_bars; */
  /* terminal->condemn_scroll_bars_hook = headless_condemn_scroll_bars; */
  /* terminal->redeem_scroll_bar_hook = headless_redeem_scroll_bar; */
  /* terminal->update_begin_hook = headless_update_begin; */
  /* terminal->update_end_hook = headless_update_end; */
  /* terminal->frame_rehighlight_hook = headless_frame_rehighlight; */
  /* terminal->query_frame_background_color = headless_query_frame_background_color; */
  /* terminal->free_pixmap = headless_free_pixmap; */
  /* terminal->frame_raise_lower_hook = headless_frame_raise_lower; */
  /* terminal->menu_show_hook = headless_menu_show; */
  /* terminal->toggle_invisible_pointer_hook = headless_toggle_invisible_pointer; */
  /* terminal->fullscreen_hook = headless_fullscreen; */
  /* terminal->toolkit_position_hook = headless_toolkit_position; */
  /* terminal->activate_menubar_hook = headless_activate_menubar; */
  /* terminal->get_focus_frame = headless_get_focus_frame; */

  TRACE_FUNC_CALL;

  return terminal;
}

struct headless_display_info *
headless_term_init (void)
{
  struct headless_display_info *dpyinfo;
  struct terminal *terminal;
  Lisp_Object color_file, color_map, system_name;
  ptrdiff_t nbytes;
  void *name_buffer;

  TRACE_FUNC_CALL;

  block_input ();

  Fset_input_interrupt_mode (Qt);
  dpyinfo = xzalloc (sizeof *dpyinfo);

  color_file = Fexpand_file_name (build_string ("rgb.txt"),
                                  Fsymbol_value (Qdata_directory));
  color_map = Fx_load_color_file (color_file);

  if (NILP (color_map))
    fatal ("Could not read %s.\n", SDATA (color_file));

  dpyinfo->color_map = color_map;
  dpyinfo->display = Qnil;
  dpyinfo->next = x_display_list;
  dpyinfo->n_planes = 1;
  dpyinfo->resx = 1920;
  dpyinfo->resy = 1080;

  x_display_list = dpyinfo;

  terminal = headless_create_terminal (dpyinfo);

  terminal->kboard->reference_count++;
  terminal->reference_count++;
  terminal->name = xstrdup ("headless");

  dpyinfo->name_list_element = Fcons (build_string ("headless"), Qnil);
  dpyinfo->smallest_font_height = 1;
  dpyinfo->smallest_char_width = 1;

  gui_init_fringe (terminal->rif);

  system_name = Fsystem_name ();

  if (STRINGP (system_name))
    {
      nbytes = sizeof "GNU Emacs" + sizeof " at ";

      if (INT_ADD_WRAPV (nbytes, SBYTES (system_name), &nbytes))
        memory_full (SIZE_MAX);

      name_buffer = alloca (nbytes);
      sprintf (name_buffer, "%s%s%s", "GNU Emacs",
               " at ", SDATA (system_name));
      dpyinfo->default_name = build_string (name_buffer);
    }
  else
    dpyinfo->default_name = build_string ("GNU Emacs");

  unblock_input ();

  return dpyinfo;
}

unsigned long
headless_get_pixel (headless img, int x, int y)
{
  (void)img;
  (void)x;
  (void)y;

  TRACE_FUNC_CALL;
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
  TRACE_FUNC_CALL;
}

void
headless_put_pixel (headless bitmap, int x, int y, unsigned long pixel)
{
  TRACE_FUNC_CALL;
}

void
mark_headless_display (void)
{
  TRACE_FUNC_CALL;
  if (x_display_list)
    {
      mark_object (x_display_list->color_map);
      mark_object (x_display_list->default_name);
    }
}

void
syms_of_headlessterm (void)
{
  TRACE_FUNC_CALL;
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
