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

#include <math.h>

#include "lisp.h"
#include "frame.h"
#include "blockinput.h"
#include "termchar.h"
#include "font.h"
#include "keyboard.h"
#include "buffer.h"
#include "dispextern.h"

#include "headlessgui.h"
#include "headlessterm.h"
#include "termhooks.h"

#include "bitmaps/leftptr.xbm"
#include "bitmaps/leftpmsk.xbm"

#include <stdlib.h>


static Display_Info *
check_headless_display_info (Lisp_Object object)
{
  return NULL;
}

Display_Info *
check_x_display_info (Lisp_Object object)
{
  return check_headless_display_info(object);
}

DEFUN ("xw-display-color-p", Fxw_display_color_p, Sxw_display_color_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
     (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  return be_is_display_grayscale () ? Qnil : Qt;
}

DEFUN ("xw-color-defined-p", Fxw_color_defined_p, Sxw_color_defined_p, 1, 2, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object color, Lisp_Object frame)
{
  Emacs_Color col;

  CHECK_STRING (color);
  decode_window_system_frame (frame);

  return headless_get_color (SSDATA (color), &col) ? Qnil : Qt;
}

DEFUN ("xw-color-values", Fxw_color_values, Sxw_color_values, 1, 2, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
     (Lisp_Object color, Lisp_Object frame)
{
  Emacs_Color col;
  int rc;

  CHECK_STRING (color);
  decode_window_system_frame (frame);

  block_input ();
  rc = headless_get_color (SSDATA (color), &col);
  unblock_input ();

  if (rc)
    return Qnil;

  return list3i (col.red, col.green, col.blue);
}

DEFUN ("x-display-grayscale-p", Fx_display_grayscale_p, Sx_display_grayscale_p,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  return be_is_display_grayscale () ? Qt : Qnil;
}

DEFUN ("x-open-connection", Fx_open_connection, Sx_open_connection,
       1, 3, 0, doc: /* SKIP: real doc in xfns.c.  */)
     (Lisp_Object display, Lisp_Object resource_string, Lisp_Object must_succeed)
{
  CHECK_STRING (display);

  if (NILP (Fstring_equal (display, build_string ("be"))))
    {
      if (!NILP (must_succeed))
	fatal ("Invalid display %s", SDATA (display));
      else
	signal_error ("Invalid display", display);
    }

  if (x_display_list)
    {
      if (!NILP (must_succeed))
	fatal ("A display is already open");
      else
	error ("A display is already open");
    }

  headless_term_init ();
  return Qnil;
}

DEFUN ("x-display-pixel-width", Fx_display_pixel_width, Sx_display_pixel_width,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)

{
  int width, height;
  check_headless_display_info (terminal);

  be_get_screen_dimensions (&width, &height);
  return make_fixnum (width);
}

DEFUN ("x-display-pixel-height", Fx_display_pixel_height, Sx_display_pixel_height,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)

{
  int width, height;
  check_headless_display_info (terminal);

  be_get_screen_dimensions (&width, &height);
  return make_fixnum (height);
}

DEFUN ("x-display-mm-height", Fx_display_mm_height, Sx_display_mm_height, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  struct headless_display_info *dpyinfo = check_headless_display_info (terminal);
  int width, height;

  be_get_screen_dimensions (&width, &height);
  return make_fixnum (height / (dpyinfo->resy / 25.4));
}


DEFUN ("x-display-mm-width", Fx_display_mm_width, Sx_display_mm_width, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  struct headless_display_info *dpyinfo = check_headless_display_info (terminal);
  int width, height;

  be_get_screen_dimensions (&width, &height);
  return make_fixnum (width / (dpyinfo->resx / 25.4));
}

DEFUN ("x-create-frame", Fx_create_frame, Sx_create_frame,
       1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
     (Lisp_Object parms)
{
  return headless_create_frame (parms);
}

DEFUN ("x-display-visual-class", Fx_display_visual_class,
       Sx_display_visual_class, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  int planes;
  bool grayscale_p;

  check_headless_display_info (terminal);

  grayscale_p = be_is_display_grayscale ();
  if (grayscale_p)
    return Qstatic_gray;

  planes = be_get_display_planes ();
  if (planes == 8)
    return Qstatic_color;

  return Qtrue_color;
}

DEFUN ("x-show-tip", Fx_show_tip, Sx_show_tip, 1, 6, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object string, Lisp_Object frame, Lisp_Object parms,
   Lisp_Object timeout, Lisp_Object dx, Lisp_Object dy)
{
  struct frame *f, *tip_f;
  struct window *w;
  int root_x, root_y;
  struct buffer *old_buffer;
  struct text_pos pos;
  int width, height;
  int old_windows_or_buffers_changed = windows_or_buffers_changed;
  specpdl_ref count = SPECPDL_INDEX ();
  Lisp_Object window, size, tip_buf;
  bool displayed;
#ifdef ENABLE_CHECKING
  struct glyph_row *row, *end;
#endif
  AUTO_STRING (tip, " *tip*");

  specbind (Qinhibit_redisplay, Qt);

  CHECK_STRING (string);
  if (SCHARS (string) == 0)
    string = make_unibyte_string (" ", 1);

  if (NILP (frame))
    frame = selected_frame;
  f = decode_window_system_frame (frame);

  if (NILP (timeout))
    timeout = Vx_show_tooltip_timeout;
  CHECK_FIXNAT (timeout);

  if (NILP (dx))
    dx = make_fixnum (5);
  else
    CHECK_FIXNUM (dx);

  if (NILP (dy))
    dy = make_fixnum (-10);
  else
    CHECK_FIXNUM (dy);

  tip_dx = dx;
  tip_dy = dy;

  if (use_system_tooltips)
    {
      int root_x, root_y;
      CHECK_STRING (string);
      if (STRING_MULTIBYTE (string))
	string = ENCODE_UTF_8 (string);

      if (NILP (frame))
	frame = selected_frame;

      struct frame *f = decode_window_system_frame (frame);
      block_input ();

      char *str = xstrdup (SSDATA (string));
      int height = be_plain_font_height ();
      int width;
      char *tok = strtok (str, "\n");
      width = be_string_width_with_plain_font (tok);

      while ((tok = strtok (NULL, "\n")))
	{
	  height = be_plain_font_height ();
	  int w = be_string_width_with_plain_font (tok);
	  if (w > width)
	    w = width;
	}
      free (str);

      height += 16; /* Default margin.  */
      width += 16; /* Ditto.  Unfortunately there isn't a more
		      reliable way to get it.  */
      compute_tip_xy (f, parms, dx, dy, width, height, &root_x, &root_y);
      BView_convert_from_screen (FRAME_HEADLESS_VIEW (f), &root_x, &root_y);
      be_show_sticky_tooltip (FRAME_HEADLESS_VIEW (f), SSDATA (string),
			      root_x, root_y);
      unblock_input ();
      goto start_timer;
    }

  if (!NILP (tip_frame) && FRAME_LIVE_P (XFRAME (tip_frame)))
    {
      if (FRAME_VISIBLE_P (XFRAME (tip_frame))
	  && !NILP (Fequal_including_properties (tip_last_string, string))
	  && !NILP (Fequal (tip_last_parms, parms)))
	{
	  /* Only DX and DY have changed.  */
	  tip_f = XFRAME (tip_frame);
	  if (!NILP (tip_timer))
	    {
	      call1 (Qcancel_timer, tip_timer);
	      tip_timer = Qnil;
	    }

	  block_input ();
	  compute_tip_xy (tip_f, parms, dx, dy, FRAME_PIXEL_WIDTH (tip_f),
			  FRAME_PIXEL_HEIGHT (tip_f), &root_x, &root_y);
	  BWindow_set_offset (FRAME_HEADLESS_WINDOW (tip_f), root_x, root_y);
	  unblock_input ();

	  goto start_timer;
	}
      else if (tooltip_reuse_hidden_frame && EQ (frame, tip_last_frame))
	{
	  bool delete = false;
	  Lisp_Object tail, elt, parm, last;

	  /* Check if every parameter in PARMS has the same value in
	     tip_last_parms.  This may destruct tip_last_parms which,
	     however, will be recreated below.  */
	  for (tail = parms; CONSP (tail); tail = XCDR (tail))
	    {
	      elt = XCAR (tail);
	      parm = Fcar (elt);
	      /* The left, top, right and bottom parameters are handled
		 by compute_tip_xy so they can be ignored here.  */
	      if (!EQ (parm, Qleft) && !EQ (parm, Qtop)
		  && !EQ (parm, Qright) && !EQ (parm, Qbottom))
		{
		  last = Fassq (parm, tip_last_parms);
		  if (NILP (Fequal (Fcdr (elt), Fcdr (last))))
		    {
		      /* We lost, delete the old tooltip.  */
		      delete = true;
		      break;
		    }
		  else
		    tip_last_parms =
		      call2 (Qassq_delete_all, parm, tip_last_parms);
		}
	      else
		tip_last_parms =
		  call2 (Qassq_delete_all, parm, tip_last_parms);
	    }

	  /* Now check if every parameter in what is left of
	     tip_last_parms with a non-nil value has an association in
	     PARMS.  */
	  for (tail = tip_last_parms; CONSP (tail); tail = XCDR (tail))
	    {
	      elt = XCAR (tail);
	      parm = Fcar (elt);
	      if (!EQ (parm, Qleft) && !EQ (parm, Qtop) && !EQ (parm, Qright)
		  && !EQ (parm, Qbottom) && !NILP (Fcdr (elt)))
		{
		  /* We lost, delete the old tooltip.  */
		  delete = true;
		  break;
		}
	    }

	  headless_hide_tip (delete);
	}
      else
	headless_hide_tip (true);
    }
  else
    headless_hide_tip (true);

  tip_last_frame = frame;
  tip_last_string = string;
  tip_last_parms = parms;

  if (NILP (tip_frame) || !FRAME_LIVE_P (XFRAME (tip_frame)))
    {
      /* Add default values to frame parameters.  */
      if (NILP (Fassq (Qname, parms)))
	parms = Fcons (Fcons (Qname, build_string ("tooltip")), parms);
      if (NILP (Fassq (Qinternal_border_width, parms)))
	parms = Fcons (Fcons (Qinternal_border_width, make_fixnum (3)), parms);
      if (NILP (Fassq (Qborder_width, parms)))
	parms = Fcons (Fcons (Qborder_width, make_fixnum (1)), parms);
      if (NILP (Fassq (Qborder_color, parms)))
	parms = Fcons (Fcons (Qborder_color, build_string ("lightyellow")), parms);
      if (NILP (Fassq (Qbackground_color, parms)))
	parms = Fcons (Fcons (Qbackground_color, build_string ("lightyellow")),
		       parms);

      /* Create a frame for the tooltip, and record it in the global
	 variable tip_frame.  */
      if (NILP (tip_frame = headless_create_tip_frame (parms)))
	/* Creating the tip frame failed.  */
	return unbind_to (count, Qnil);
    }

  tip_f = XFRAME (tip_frame);
  window = FRAME_ROOT_WINDOW (tip_f);
  tip_buf = Fget_buffer_create (tip, Qnil);
  /* We will mark the tip window a "pseudo-window" below, and such
     windows cannot have display margins.  */
  bset_left_margin_cols (XBUFFER (tip_buf), make_fixnum (0));
  bset_right_margin_cols (XBUFFER (tip_buf), make_fixnum (0));
  set_window_buffer (window, tip_buf, false, false);
  w = XWINDOW (window);
  w->pseudo_window_p = true;
  /* Try to avoid that `other-window' select us (Bug#47207).  */
  Fset_window_parameter (window, Qno_other_window, Qt);

  /* Set up the frame's root window.  Note: The following code does not
     try to size the window or its frame correctly.  Its only purpose is
     to make the subsequent text size calculations work.  The right
     sizes should get installed when the toolkit gets back to us.  */
  w->left_col = 0;
  w->top_line = 0;
  w->pixel_left = 0;
  w->pixel_top = 0;

  if (CONSP (Vx_max_tooltip_size)
      && RANGED_FIXNUMP (1, XCAR (Vx_max_tooltip_size), INT_MAX)
      && RANGED_FIXNUMP (1, XCDR (Vx_max_tooltip_size), INT_MAX))
    {
      w->total_cols = XFIXNAT (XCAR (Vx_max_tooltip_size));
      w->total_lines = XFIXNAT (XCDR (Vx_max_tooltip_size));
    }
  else
    {
      w->total_cols = 80;
      w->total_lines = 40;
    }

  w->pixel_width = w->total_cols * FRAME_COLUMN_WIDTH (tip_f);
  w->pixel_height = w->total_lines * FRAME_LINE_HEIGHT (tip_f);
  FRAME_TOTAL_COLS (tip_f) = w->total_cols;
  adjust_frame_glyphs (tip_f);

  /* Insert STRING into root window's buffer and fit the frame to the
     buffer.  */
  specpdl_ref count_1 = SPECPDL_INDEX ();
  old_buffer = current_buffer;
  set_buffer_internal_1 (XBUFFER (w->contents));
  bset_truncate_lines (current_buffer, Qnil);
  specbind (Qinhibit_read_only, Qt);
  specbind (Qinhibit_modification_hooks, Qt);
  specbind (Qinhibit_point_motion_hooks, Qt);
  Ferase_buffer ();
  Finsert (1, &string);
  clear_glyph_matrix (w->desired_matrix);
  clear_glyph_matrix (w->current_matrix);
  SET_TEXT_POS (pos, BEGV, BEGV_BYTE);
  displayed = try_window (window, pos, TRY_WINDOW_IGNORE_FONTS_CHANGE);

  if (!displayed && NILP (Vx_max_tooltip_size))
    {
#ifdef ENABLE_CHECKING
      row = w->desired_matrix->rows;
      end = w->desired_matrix->rows + w->desired_matrix->nrows;

      while (row < end)
	{
	  if (!row->displays_text_p
	      || row->ends_at_zv_p)
	    break;
	  ++row;
	}

      eassert (row < end && row->ends_at_zv_p);
#endif
    }

  /* Calculate size of tooltip window.  */
  size = Fwindow_text_pixel_size (window, Qnil, Qnil, Qnil,
				  make_fixnum (w->pixel_height), Qnil,
				  Qnil);
  /* Add the frame's internal border to calculated size.  */
  width = XFIXNUM (Fcar (size)) + 2 * FRAME_INTERNAL_BORDER_WIDTH (tip_f);
  height = XFIXNUM (Fcdr (size)) + 2 * FRAME_INTERNAL_BORDER_WIDTH (tip_f);

  /* Calculate position of tooltip frame.  */
  compute_tip_xy (tip_f, parms, dx, dy, width, height, &root_x, &root_y);

  /* Show tooltip frame.  */
  block_input ();
  void *wnd = FRAME_HEADLESS_WINDOW (tip_f);
  BWindow_resize (wnd, width, height);
  /* The window decorator might cause the actual width and height to
     be larger than WIDTH and HEIGHT, so use the actual sizes.  */
  BWindow_dimensions (wnd, &width, &height);
  BView_resize_to (FRAME_HEADLESS_VIEW (tip_f), width, height);
  BView_set_view_cursor (FRAME_HEADLESS_VIEW (tip_f),
			 FRAME_OUTPUT_DATA (f)->current_cursor);
  BWindow_set_offset (wnd, root_x, root_y);
  BWindow_set_visible (wnd, true);
  SET_FRAME_VISIBLE (tip_f, true);
  FRAME_PIXEL_WIDTH (tip_f) = width;
  FRAME_PIXEL_HEIGHT (tip_f) = height;
  BWindow_sync (wnd);

  /* This is needed because the app server resets the cursor whenever
     a new window is mapped, so we won't see the cursor set on the
     tooltip if the mouse pointer isn't actually over it.  */
  BView_set_view_cursor (FRAME_HEADLESS_VIEW (f),
			 FRAME_OUTPUT_DATA (f)->current_cursor);
  unblock_input ();

  w->must_be_updated_p = true;
  update_single_window (w);
  flush_frame (tip_f);
  set_buffer_internal_1 (old_buffer);
  unbind_to (count_1, Qnil);
  windows_or_buffers_changed = old_windows_or_buffers_changed;

 start_timer:
  /* Let the tip disappear after timeout seconds.  */
  tip_timer = call3 (Qrun_at_time, timeout, Qnil, Qx_hide_tip);

  return unbind_to (count, Qnil);
}

DEFUN ("x-hide-tip", Fx_hide_tip, Sx_hide_tip, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  return headless_hide_tip (!tooltip_reuse_hidden_frame);
}

DEFUN ("x-close-connection", Fx_close_connection, Sx_close_connection, 1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */
       attributes: noreturn)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  error ("Cannot close Headless displays");
}

DEFUN ("x-display-list", Fx_display_list, Sx_display_list, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c. */)
  (void)
{
  if (!x_display_list)
    return Qnil;

  return list1 (XCAR (x_display_list->name_list_element));
}

DEFUN ("x-server-vendor", Fx_server_vendor, Sx_server_vendor, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);
  return build_string ("Headless, Inc.");
}

DEFUN ("x-server-version", Fx_server_version, Sx_server_version, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c. */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);
  return list3i (5, 1, 1);
}

DEFUN ("x-display-screens", Fx_display_screens, Sx_display_screens, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);
  return make_fixnum (be_get_display_screens ());
}

DEFUN ("headless-get-version-string", Fheadless_get_version_string,
       Sheadless_get_version_string, 0, 0, 0,
       doc: /* Return a string describing the current Headless version.  */)
  (void)
{
  char buf[1024];

  be_get_version_string ((char *) &buf, sizeof buf);
  return build_string (buf);
}

DEFUN ("x-display-color-cells", Fx_display_color_cells, Sx_display_color_cells,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  return make_fixnum (be_get_display_color_cells ());
}

DEFUN ("x-display-planes", Fx_display_planes, Sx_display_planes,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  return make_fixnum (be_get_display_planes ());
}

DEFUN ("x-double-buffered-p", Fx_double_buffered_p, Sx_double_buffered_p,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object frame)
{
  struct frame *f = decode_window_system_frame (frame);

  return EmacsView_double_buffered_p (FRAME_HEADLESS_VIEW (f)) ? Qt : Qnil;
}

DEFUN ("x-display-backing-store", Fx_display_backing_store, Sx_display_backing_store,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  return Qnot_useful;
}

frame_parm_handler headless_frame_parm_handlers[] =
  {
    gui_set_autoraise,
    gui_set_autolower,
    headless_set_background_color,
    NULL, /* x_set_border_color */
    gui_set_border_width,
    headless_set_cursor_color,
    headless_set_cursor_type,
    gui_set_font,
    headless_set_foreground_color,
    NULL, /* set icon name */
    NULL, /* set icon type */
    headless_set_child_frame_border_width,
    headless_set_internal_border_width,
    gui_set_right_divider_width,
    gui_set_bottom_divider_width,
    headless_set_menu_bar_lines,
    headless_set_mouse_color,
    headless_explicitly_set_name,
    gui_set_scroll_bar_width,
    gui_set_scroll_bar_height,
    headless_set_title,
    gui_set_unsplittable,
    gui_set_vertical_scroll_bars,
    gui_set_horizontal_scroll_bars,
    gui_set_visibility,
    headless_set_tab_bar_lines,
    headless_set_tool_bar_lines,
    NULL, /* set scroll bar fg */
    NULL, /* set scroll bar bkg */
    gui_set_screen_gamma,
    gui_set_line_spacing,
    gui_set_left_fringe,
    gui_set_right_fringe,
    NULL, /* x wait for wm */
    gui_set_fullscreen,
    gui_set_font_backend,
    gui_set_alpha,
    headless_set_sticky,
    NULL, /* set tool bar pos */
    headless_set_inhibit_double_buffering,
    headless_set_undecorated,
    headless_set_parent_frame,
    NULL, /* set skip taskbar */
    headless_set_no_focus_on_map,
    headless_set_no_accept_focus,
    headless_set_z_group,
    headless_set_override_redirect,
    gui_set_no_special_glyphs,
    gui_set_alpha_background,
    headless_set_use_frame_synchronization,
  };

void
syms_of_headlessfns (void)
{
  DEFSYM (Qfont_parameter, "font-parameter");
  DEFSYM (Qcancel_timer, "cancel-timer");
  DEFSYM (Qassq_delete_all, "assq-delete-all");

  DEFSYM (Qrun_at_time, "run-at-time");
  DEFSYM (Qx_hide_tip, "x-hide-tip");

  DEFSYM (Qalways, "always");
  DEFSYM (Qnot_useful, "not-useful");
  DEFSYM (Qwhen_mapped, "when-mapped");
  DEFSYM (Qtooltip_reuse_hidden_frame, "tooltip-reuse-hidden-frame");

  DEFSYM (Qstatic_color, "static-color");
  DEFSYM (Qstatic_gray, "static-gray");
  DEFSYM (Qtrue_color, "true-color");
  DEFSYM (Qmono, "mono");
  DEFSYM (Qgrayscale, "grayscale");
  DEFSYM (Qcolor, "color");

  defsubr (&Sx_hide_tip);
  defsubr (&Sxw_display_color_p);
  defsubr (&Sx_display_grayscale_p);
  defsubr (&Sx_open_connection);
  defsubr (&Sx_create_frame);
  defsubr (&Sx_display_pixel_width);
  defsubr (&Sx_display_pixel_height);
  defsubr (&Sxw_color_values);
  defsubr (&Sxw_color_defined_p);
  defsubr (&Sx_display_visual_class);
  defsubr (&Sx_show_tip);
  defsubr (&Sx_display_mm_height);
  defsubr (&Sx_display_mm_width);
  defsubr (&Sx_close_connection);
  defsubr (&Sx_display_list);
  defsubr (&Sx_server_vendor);
  defsubr (&Sx_server_version);
  defsubr (&Sx_display_screens);
  defsubr (&Sheadless_get_version_string);
  defsubr (&Sx_display_color_cells);
  defsubr (&Sx_display_planes);
  defsubr (&Sheadless_set_mouse_absolute_pixel_position);
  defsubr (&Sheadless_mouse_absolute_pixel_position);
  defsubr (&Sheadless_frame_geometry);
  defsubr (&Sheadless_frame_edges);
  defsubr (&Sx_double_buffered_p);
  defsubr (&Sx_display_backing_store);
  defsubr (&Sheadless_read_file_name);
  defsubr (&Sheadless_put_resource);
  defsubr (&Sheadless_frame_list_z_order);
  defsubr (&Sx_display_save_under);
  defsubr (&Sheadless_frame_restack);
  defsubr (&Sheadless_save_session_reply);
  defsubr (&Sheadless_display_monitor_attributes_list);

  tip_timer = Qnil;
  staticpro (&tip_timer);
  tip_frame = Qnil;
  staticpro (&tip_frame);
  tip_last_frame = Qnil;
  staticpro (&tip_last_frame);
  tip_last_string = Qnil;
  staticpro (&tip_last_string);
  tip_last_parms = Qnil;
  staticpro (&tip_last_parms);
  tip_dx = Qnil;
  staticpro (&tip_dx);
  tip_dy = Qnil;
  staticpro (&tip_dy);

  DEFVAR_LISP ("x-max-tooltip-size", Vx_max_tooltip_size,
	       doc: /* SKIP: real doc in xfns.c.  */);
  Vx_max_tooltip_size = Qnil;

  DEFVAR_LISP ("x-cursor-fore-pixel", Vx_cursor_fore_pixel,
	       doc: /* SKIP: real doc in xfns.c.  */);
  Vx_cursor_fore_pixel = Qnil;

  DEFVAR_LISP ("x-pointer-shape", Vx_pointer_shape,
	       doc: /* SKIP: real doc in xfns.c.  */);
  Vx_pointer_shape = Qnil;

  DEFVAR_LISP ("x-hourglass-pointer-shape", Vx_hourglass_pointer_shape,
	       doc: /* SKIP: real doc in xfns.c.  */);
  Vx_hourglass_pointer_shape = Qnil;

  DEFVAR_LISP ("x-sensitive-text-pointer-shape",
	       Vx_sensitive_text_pointer_shape,
	       doc: /* SKIP: real doc in xfns.c.  */);
  Vx_sensitive_text_pointer_shape = Qnil;

  DEFVAR_LISP ("headless-allowed-ui-colors", Vheadless_allowed_ui_colors,
	       doc: /* Vector of UI colors that Emacs can look up from the system.
If this is set up incorrectly, Emacs can crash when encoutering an
invalid color.  */);
  Vheadless_allowed_ui_colors = Qnil;

  return;
}
