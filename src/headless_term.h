/* headless Terminal control module
   Copyright (C) 1985-1987, 1993-1995, 1998, 2000-2022 Free Software
   Foundation, Inc.

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

#ifndef __HEADLESS_TERM_H__
#define __HEADLESS_TERM_H__

#include "headless_gui.h"
#include "frame.h"
#include "character.h"
#include "dispextern.h"
#include "font.h"
#include "termhooks.h"

struct headless_display_info
{
  /* Chain of all struct headless_display_info structures.  */
  struct headless_display_info *next;

  /* The terminal.  */
  struct terminal *terminal;

  /* The root window.  This field is unused.  */
  Emacs_Window root_window;

  /* List possibly used only for the font cache but probably used for
     something else too.  */
  Lisp_Object name_list_element;

  /* List of predefined X colors.  */
  Lisp_Object color_map;

  /* DPI of the display.  */
  double resx, resy;

  /* Mouse highlight information.  */
  Mouse_HLInfo mouse_highlight;

  /* Number of planes on this screen.  Always 24.  */
  int n_planes;

  /* Mask of things causing the mouse to be grabbed.  */
  int grabbed;

  /* Minimum width over all characters in all fonts in font_table.  */
  int smallest_char_width;

  /* Minimum font height over all fonts in font_table.  */
  int smallest_font_height;

  /* The frame currently with the input focus.  */
  struct frame *focus_frame;

  /* The last frame mentioned in a focus event.  */
  struct frame *x_focus_event_frame;

  /* The frame which currently has the visual highlight, and should
     get keyboard input.  It points to the focus frame's selected
     window's frame, but can differ.  */
  struct frame *highlight_frame;

  /* The frame waiting to be auto-raised in headless_read_socket.  */
  struct frame *pending_autoraise_frame;

  /* The frame where the mouse was the last time a button event
     happened.  */
  struct frame *last_mouse_frame;

  /* The frame where the mouse was the last time the mouse glyph
     changed.  */
  struct frame *last_mouse_glyph_frame;

  /* The frame where the mouse was the last time mouse motion
     happened.  */
  struct frame *last_mouse_motion_frame;

  /* Position where the mouse was last time we reported a motion.
     This is a position on last_mouse_motion_frame.  It is used in to
     report the mouse position as well: see
     headless_mouse_position.  */
  int last_mouse_motion_x, last_mouse_motion_y;
};

struct headless_output
{
  /* The window used for this frame.  */
  Emacs_Window window;

  /* Unused field.  */
  Emacs_Window parent_desc;

  struct headless_display_info *display_info;

  /* Default ASCII font of this frame.  */
  struct font *font;

  /* The baseline offset of the default ASCII font.  */
  int baseline_offset;

  /* If a fontset is specified for this frame instead of font, this
     value contains an ID of the fontset, else -1.  */
  int fontset;

  /* Various colors.  */
  unsigned long cursor_pixel;
  unsigned long mouse_pixel;
  unsigned long cursor_foreground_pixel;

  /* Foreground color for scroll bars.  A value of -1 means use the
     default (black for non-toolkit scroll bars).  */
  unsigned long scroll_bar_foreground_pixel;

  /* Background color for scroll bars.  A value of -1 means use the
     default (background color of the frame for non-toolkit scroll
     bars).  */
  unsigned long scroll_bar_background_pixel;

  /* Cursors associated with this frame.  */
  Emacs_Cursor text_cursor;
  Emacs_Cursor nontext_cursor;
  Emacs_Cursor modeline_cursor;
  Emacs_Cursor hand_cursor;
  Emacs_Cursor hourglass_cursor;
  Emacs_Cursor horizontal_drag_cursor;
  Emacs_Cursor vertical_drag_cursor;
  Emacs_Cursor current_cursor;
  Emacs_Cursor left_edge_cursor;
  Emacs_Cursor top_left_corner_cursor;
  Emacs_Cursor top_edge_cursor;
  Emacs_Cursor top_right_corner_cursor;
  Emacs_Cursor right_edge_cursor;
  Emacs_Cursor bottom_right_corner_cursor;
  Emacs_Cursor bottom_edge_cursor;
  Emacs_Cursor bottom_left_corner_cursor;
};

#define FRAME_OUTPUT_DATA(f)	((f)->output_data.headless)
#define FRAME_NATIVE_WINDOW(f)	((f)->output_data.headless->window)

#define FRAME_FONT(f)		((f)->output_data.headless->font)
#define FRAME_FONTSET(f)	((f)->output_data.headless->fontset)

#define FRAME_BASELINE_OFFSET(f)		\
  ((f)->output_data.headless->baseline_offset)

/* This gives the headless_display_info structure for the display F is
   on.  */
#define FRAME_DISPLAY_INFO(f) ((f)->output_data.headless->display_info)

/* Some things for X compatibility.  */
#define BLACK_PIX_DEFAULT(f) 0
#define WHITE_PIX_DEFAULT(f) 0xffffffff

/* This is a chain of structures for all the Headless displays
   currently in use.  There is only ever one, but the rest of Emacs is
   written with systems on which there can be many in mind.  */
extern struct headless_display_info *x_display_list;

void headless_set_hooks (struct terminal *terminal);

#endif //__HEADLESS_TERM_H__
