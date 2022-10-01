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

#define RGB_TO_ULONG(r, g, b)                   \
  (((r) << 16) | ((g) << 8) | (b));
#define RED_FROM_ULONG(color)	(((color) >> 16) & 0xff)
#define GREEN_FROM_ULONG(color)	(((color) >> 8) & 0xff)
#define BLUE_FROM_ULONG(color)	((color) & 0xff)

/* The frame of the currently visible tooltip.  */
Lisp_Object tip_frame;

/* The X and Y deltas of the last call to `x-show-tip'.  */
Lisp_Object tip_dx, tip_dy;

/* The window-system window corresponding to the frame of the
   currently visible tooltip.  */
static Window tip_window;

/* A timer that hides or deletes the currently visible tooltip when it
   fires.  */
static Lisp_Object tip_timer;

/* STRING argument of last `x-show-tip' call.  */
static Lisp_Object tip_last_string;

/* Normalized FRAME argument of last `x-show-tip' call.  */
static Lisp_Object tip_last_frame;

/* PARMS argument of last `x-show-tip' call.  */
static Lisp_Object tip_last_parms;

static void headless_explicitly_set_name (struct frame *, Lisp_Object, Lisp_Object);
static void headless_set_title (struct frame *, Lisp_Object, Lisp_Object);

/* The number of references to an image cache.  */
static ptrdiff_t image_cache_refcount;

static Lisp_Object
get_geometry_from_preferences (struct haiku_display_info *dpyinfo,
                               Lisp_Object parms)
{
  struct {
    const char *val;
    const char *cls;
    Lisp_Object tem;
  } r[] = {
    { "width",  "Width", Qwidth },
    { "height", "Height", Qheight },
    { "left", "Left", Qleft },
    { "top", "Top", Qtop },
  };
  TRACE_FUNC_CALL;

  int i;
  for (i = 0; i < ARRAYELTS (r); ++i)
    {
      if (NILP (Fassq (r[i].tem, parms)))
        {
          Lisp_Object value
            = gui_display_get_arg (dpyinfo, parms, r[i].tem, r[i].val, r[i].cls,
                                   RES_TYPE_NUMBER);
          if (! BASE_EQ (value, Qunbound))
            parms = Fcons (Fcons (r[i].tem, value), parms);
        }
    }

  return parms;
}

static struct headless_display_info *
headless_display_info_for_name (Lisp_Object name)
{
  TRACE_FUNC_CALL;
  CHECK_STRING (name);

  if (!strcmp (SSDATA (name), "headless"))
    {
      if (x_display_list)
        return x_display_list;

      return headless_term_init ();
    }

  error ("Headless displays can only be named \"be\"");
}

static Display_Info *
check_headless_display_info (Lisp_Object object)
{
  struct headless_display_info *dpyinfo = NULL;
  TRACE_FUNC_CALL;

  if (NILP (object))
    {
      struct frame *sf = XFRAME (selected_frame);

      if (FRAME_HEADLESS_P (sf) && FRAME_LIVE_P (sf))
        dpyinfo = FRAME_DISPLAY_INFO (sf);
      else if (x_display_list)
        dpyinfo = x_display_list;
      else
        error ("Headless windowing not present");
    }
  else if (TERMINALP (object))
    {
      struct terminal *t = decode_live_terminal (object);

      if (t->type != output_headless)
        error ("Terminal %d is not a Headless display", t->id);

      dpyinfo = t->display_info.headless;
    }
  else if (STRINGP (object))
    dpyinfo = headless_display_info_for_name (object);
  else
    {
      struct frame *f = decode_window_system_frame (object);
      dpyinfo = FRAME_DISPLAY_INFO (f);
    }

  return dpyinfo;
}

Display_Info *
check_x_display_info (Lisp_Object object)
{
  TRACE_FUNC_CALL;
  return check_headless_display_info(object);
}

static void
headless_set_title_bar_text (struct frame *f, Lisp_Object text)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_title (struct frame *f, Lisp_Object name, Lisp_Object old_name)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_child_frame_border_width (struct frame *f,
                                       Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_parent_frame (struct frame *f, Lisp_Object new_value,
                           Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_z_group (struct frame *f, Lisp_Object new_value,
                      Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

static void
headless_explicitly_set_name (struct frame *f, Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_no_accept_focus (struct frame *f, Lisp_Object new_value, Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_foreground_color (struct frame *f, Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
unwind_create_frame (Lisp_Object frame)
{
  struct frame *f = XFRAME (frame);
  TRACE_FUNC_CALL;

  /* If frame is already dead, nothing to do.  This can happen if the
     display is disconnected after the frame has become official, but
     before x_create_frame removes the unwind protect.  */
  if (!FRAME_LIVE_P (f))
    return;

  /* If frame is ``official'', nothing to do.  */
  if (NILP (Fmemq (frame, Vframe_list)))
    {
#if defined GLYPH_DEBUG && defined ENABLE_CHECKING
      struct headless_display_info *dpyinfo = FRAME_DISPLAY_INFO (f);
#endif

      /* If the frame's image cache refcount is still the same as our
         private shadow variable, it means we are unwinding a frame
         for which we didn't yet call init_frame_faces, where the
         refcount is incremented.  Therefore, we increment it here, so
         that free_frame_faces, called in free_frame_resources later,
         will not mistakenly decrement the counter that was not
         incremented yet to account for this new frame.  */
      if (FRAME_IMAGE_CACHE (f) != NULL
          && FRAME_IMAGE_CACHE (f)->refcount == image_cache_refcount)
        FRAME_IMAGE_CACHE (f)->refcount++;

      headless_free_frame_resources (f);
      free_glyphs (f);

#if defined GLYPH_DEBUG && defined ENABLE_CHECKING
      /* Check that reference counts are indeed correct.  */
      if (dpyinfo->terminal->image_cache)
        eassert (dpyinfo->terminal->image_cache->refcount == image_cache_refcount);
#endif
    }
}

static Lisp_Object
headless_create_frame (Lisp_Object parms)
{
  struct frame *f, *cascade_target;
  Lisp_Object frame, tem;
  Lisp_Object name;
  bool minibuffer_only = false;
  long window_prompting = 0;
  specpdl_ref count = SPECPDL_INDEX ();
  Lisp_Object display;
  struct headless_display_info *dpyinfo = NULL;
  struct kboard *kb;

  TRACE_FUNC_CALL;

  if (x_display_list->focused_frame)
    cascade_target = x_display_list->focused_frame;
  else if (x_display_list->focus_event_frame)
    cascade_target = x_display_list->focus_event_frame;
  else
    cascade_target = NULL;

  /* Always cascade from the most toplevel frame.  */

  while (cascade_target && FRAME_PARENT_FRAME (cascade_target))
    cascade_target = FRAME_PARENT_FRAME (cascade_target);

  parms = Fcopy_alist (parms);

  Vx_resource_name = Vinvocation_name;

  display = gui_display_get_arg (dpyinfo, parms, Qterminal, 0, 0,
                                 RES_TYPE_STRING);
  if (BASE_EQ (display, Qunbound))
    display = Qnil;
  dpyinfo = check_headless_display_info (display);
  kb = dpyinfo->terminal->kboard;

  if (!dpyinfo->terminal->name)
    error ("Terminal is not live, can't create new frames on it");

  name = gui_display_get_arg (dpyinfo, parms, Qname, 0, 0,
                              RES_TYPE_STRING);
  if (!STRINGP (name)
      && ! BASE_EQ (name, Qunbound)
      && ! NILP (name))
    error ("Invalid frame name--not a string or nil");

  if (STRINGP (name))
    Vx_resource_name = name;

  /* make_frame_without_minibuffer can run Lisp code and garbage collect.  */
  /* No need to protect DISPLAY because that's not used after passing
     it to make_frame_without_minibuffer.  */
  frame = Qnil;
  tem = gui_display_get_arg (dpyinfo, parms, Qminibuffer,
                             "minibuffer", "Minibuffer",
                             RES_TYPE_SYMBOL);
  if (EQ (tem, Qnone) || NILP (tem))
    f = make_frame_without_minibuffer (Qnil, kb, display);
  else if (EQ (tem, Qonly))
    {
      f = make_minibuffer_frame ();
      minibuffer_only = 1;
    }
  else if (WINDOWP (tem))
    f = make_frame_without_minibuffer (tem, kb, display);
  else
    f = make_frame (1);

  XSETFRAME (frame, f);

  f->terminal = dpyinfo->terminal;

  f->output_method = output_headless;
  f->output_data.headless = xzalloc (sizeof *f->output_data.headless);
  f->output_data.headless->wait_for_event_type = -1;
  f->output_data.headless->relief_background = -1;

  fset_icon_name (f, gui_display_get_arg (dpyinfo, parms, Qicon_name,
                                          "iconName", "Title",
                                          RES_TYPE_STRING));
  if (! STRINGP (f->icon_name))
    fset_icon_name (f, Qnil);

  FRAME_DISPLAY_INFO (f) = dpyinfo;

  /* With FRAME_DISPLAY_INFO set up, this unwind-protect is safe.  */
  record_unwind_protect (unwind_create_frame, frame);

  /* Set the name; the functions to which we pass f expect the name to
     be set.  */
  if (BASE_EQ (name, Qunbound) || NILP (name) || ! STRINGP (name))
    {
      fset_name (f, Vinvocation_name);
      f->explicit_name = 0;
    }
  else
    {
      fset_name (f, name);
      f->explicit_name = 1;
      specbind (Qx_resource_name, name);
    }

#ifdef USE_BE_CAIRO
  register_font_driver (&ftcrfont_driver, f);
#ifdef HAVE_HARFBUZZ
  register_font_driver (&ftcrhbfont_driver, f);
#endif
#endif
  /* register_font_driver (&haikufont_driver, f); */

  image_cache_refcount =
    FRAME_IMAGE_CACHE (f) ? FRAME_IMAGE_CACHE (f)->refcount : 0;

  gui_default_parameter (f, parms, Qfont_backend, Qnil,
                         "fontBackend", "FontBackend", RES_TYPE_STRING);

  FRAME_RIF (f)->default_font_parameter (f, parms);

  if (!FRAME_FONT (f))
    {
      delete_frame (frame, Qnoelisp);
      error ("Invalid frame font");
    }

  gui_default_parameter (f, parms, Qborder_width, make_fixnum (0),
                         "borderwidth", "BorderWidth", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qinternal_border_width, make_fixnum (0),
                         "internalBorderWidth", "InternalBorderWidth",
                         RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qchild_frame_border_width, Qnil,
                         "childFrameBorderWidth", "childFrameBorderWidth",
                         RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qright_divider_width, make_fixnum (0),
                         NULL, NULL, RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qbottom_divider_width, make_fixnum (0),
                         NULL, NULL, RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qvertical_scroll_bars, Qt,
                         "verticalScrollBars", "VerticalScrollBars",
                         RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qhorizontal_scroll_bars, Qnil,
                         "horizontalScrollBars", "HorizontalScrollBars",
                         RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qforeground_color, build_string ("black"),
                         "foreground", "Foreground", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qbackground_color, build_string ("white"),
                         "background", "Background", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qmouse_color, build_string ("font-color"),
                         "pointerColor", "Foreground", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qline_spacing, Qnil,
                         "lineSpacing", "LineSpacing", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qleft_fringe, Qnil,
                         "leftFringe", "LeftFringe", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qright_fringe, Qnil,
                         "rightFringe", "RightFringe", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qno_special_glyphs, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);

  init_frame_faces (f);

  /* Read comment about this code in corresponding place in xfns.c.  */
  tem = gui_display_get_arg (dpyinfo, parms, Qmin_width, NULL, NULL,
                             RES_TYPE_NUMBER);
  if (FIXNUMP (tem))
    store_frame_param (f, Qmin_width, tem);
  tem = gui_display_get_arg (dpyinfo, parms, Qmin_height, NULL, NULL,
                             RES_TYPE_NUMBER);
  if (FIXNUMP (tem))
    store_frame_param (f, Qmin_height, tem);

  adjust_frame_size (f, FRAME_COLS (f) * FRAME_COLUMN_WIDTH (f),
                     FRAME_LINES (f) * FRAME_LINE_HEIGHT (f), 5, 1,
                     Qx_create_frame_1);

  gui_default_parameter (f, parms, Qno_focus_on_map, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qno_accept_focus, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);

  /* The resources controlling the menu-bar, tool-bar, and tab-bar are
     processed specially at startup, and reflected in the mode
     variables; ignore them here.  */
  gui_default_parameter (f, parms, Qmenu_bar_lines,
                         NILP (Vmenu_bar_mode)
                         ? make_fixnum (0) : make_fixnum (1),
                         NULL, NULL, RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qtab_bar_lines,
                         NILP (Vtab_bar_mode)
                         ? make_fixnum (0) : make_fixnum (1),
                         NULL, NULL, RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qtool_bar_lines,
                         NILP (Vtool_bar_mode)
                         ? make_fixnum (0) : make_fixnum (1),
                         NULL, NULL, RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qbuffer_predicate, Qnil, "bufferPredicate",
                         "BufferPredicate", RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qtitle, Qnil, "title", "Title",
                         RES_TYPE_STRING);

  parms = get_geometry_from_preferences (dpyinfo, parms);
  window_prompting = gui_figure_window_size (f, parms, false, true);

  tem = gui_display_get_arg (dpyinfo, parms, Qunsplittable, 0, 0,
                             RES_TYPE_BOOLEAN);
  f->no_split = minibuffer_only || (!BASE_EQ (tem, Qunbound) && !NILP (tem));

  f->terminal->reference_count++;

  /* FRAME_OUTPUT_DATA (f)->window = BWindow_new (&FRAME_OUTPUT_DATA (f)->view); */

  if (!FRAME_OUTPUT_DATA (f)->window)
    xsignal1 (Qerror, build_unibyte_string ("Could not create window"));

  block_input ();
  if (!minibuffer_only && FRAME_EXTERNAL_MENU_BAR (f))
    initialize_frame_menubar (f);
  unblock_input ();

  Vframe_list = Fcons (frame, Vframe_list);

  Lisp_Object parent_frame = gui_display_get_arg (dpyinfo, parms,
                                                  Qparent_frame, NULL, NULL,
                                                  RES_TYPE_SYMBOL);

  if (BASE_EQ (parent_frame, Qunbound)
      || NILP (parent_frame)
      || !FRAMEP (parent_frame)
      || !FRAME_LIVE_P (XFRAME (parent_frame)))
    parent_frame = Qnil;

  /* It doesn't make sense to center child frames, the resulting
     position makes no sense.  */
  if (!NILP (parent_frame))
    window_prompting |= PPosition;

  fset_parent_frame (f, parent_frame);
  store_frame_param (f, Qparent_frame, parent_frame);

  /* if (!NILP (parent_frame)) */
  /*   haiku_set_parent_frame (f, parent_frame, Qnil); */

  gui_default_parameter (f, parms, Qundecorated, Qnil, NULL, NULL, RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qoverride_redirect, Qnil, NULL, NULL, RES_TYPE_BOOLEAN);

  gui_default_parameter (f, parms, Qicon_type, Qnil,
                         "bitmapIcon", "BitmapIcon", RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qauto_raise, Qnil,
                         "autoRaise", "AutoRaiseLower", RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qauto_lower, Qnil,
                         "autoLower", "AutoLower", RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qcursor_type, Qbox,
                         "cursorType", "CursorType", RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qscroll_bar_width, Qnil,
                         "scrollBarWidth", "ScrollBarWidth",
                         RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qscroll_bar_height, Qnil,
                         "scrollBarHeight", "ScrollBarHeight",
                         RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qalpha, Qnil,
                         "alpha", "Alpha", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qalpha_background, Qnil,
                         "alphaBackground", "AlphaBackground", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qfullscreen, Qnil,
                         "fullscreen", "Fullscreen", RES_TYPE_SYMBOL);

  gui_default_parameter (f, parms, Qinhibit_double_buffering, Qnil,
                         "inhibitDoubleBuffering", "InhibitDoubleBuffering",
                         RES_TYPE_BOOLEAN);

  f->can_set_window_size = true;

  adjust_frame_size (f, FRAME_TEXT_WIDTH (f), FRAME_TEXT_HEIGHT (f),
                     0, true, Qx_create_frame_2);

  Lisp_Object visibility;

  visibility = gui_display_get_arg (dpyinfo, parms, Qvisibility, 0, 0,
                                    RES_TYPE_SYMBOL);
  if (BASE_EQ (visibility, Qunbound))
    visibility = Qt;
  /* if (EQ (visibility, Qicon)) */
  /*   haiku_iconify_frame (f); */
  /* else if (!NILP (visibility)) */
  /*   haiku_visualize_frame (f); */
  /* else /\* Qnil *\/ */
  /*   { */
  /*     f->was_invisible = true; */
  /*   } */

  if (FRAME_HAS_MINIBUF_P (f)
      && (!FRAMEP (KVAR (kb, Vdefault_minibuffer_frame))
          || !FRAME_LIVE_P (XFRAME (KVAR (kb, Vdefault_minibuffer_frame)))))
    kset_default_minibuffer_frame (kb, frame);

  /* Set whether or not frame synchronization is enabled.  */
  gui_default_parameter (f, parms, Quse_frame_synchronization, Qt,
                         NULL, NULL, RES_TYPE_BOOLEAN);

  gui_default_parameter (f, parms, Qz_group, Qnil,
                         NULL, NULL, RES_TYPE_SYMBOL);

  for (tem = parms; CONSP (tem); tem = XCDR (tem))
    if (CONSP (XCAR (tem)) && !NILP (XCAR (XCAR (tem))))
      fset_param_alist (f, Fcons (XCAR (tem), f->param_alist));

  block_input ();
  /* if (window_prompting & (USPosition | PPosition)) */
  /*   haiku_set_offset (f, f->left_pos, f->top_pos, 1); */
  /* else if (cascade_target) */
  /*   haiku_set_offset (f, cascade_target->left_pos + 15, */
	/* 	      cascade_target->top_pos + 15, 1); */
  /* else */
  /*   BWindow_center_on_screen (FRAME_HAIKU_WINDOW (f)); */
  unblock_input ();

  FRAME_OUTPUT_DATA (f)->configury_done = true;

  if (f->want_fullscreen != FULLSCREEN_NONE)
    FRAME_TERMINAL (f)->fullscreen_hook (f);

  /* Make sure windows on this frame appear in calls to next-window
     and similar functions.  */
  Vwindow_list = Qnil;

  return unbind_to (count, frame);
}

static unsigned long
headless_decode_color (struct frame *f, Lisp_Object color_name)
{
  Emacs_Color cdef;

  CHECK_STRING (color_name);

  TRACE_FUNC_CALL;
  if (!headless_get_color (SSDATA (color_name), &cdef))
    return cdef.pixel;

  signal_error ("Undefined color", color_name);
}

static Lisp_Object
headless_create_tip_frame (Lisp_Object parms)
{
  TRACE_FUNC_CALL;
  return NULL;
}

static void
headless_set_menu_bar_lines (struct frame *f, Lisp_Object value, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_undecorated (struct frame *f, Lisp_Object new_value,
                          Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_override_redirect (struct frame *f, Lisp_Object new_value,
                                Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

int
headless_get_color (const char *name, Emacs_Color *color)
{
  TRACE_FUNC_CALL;
  return 0;
}

void
headless_set_internal_border_width (struct frame *f, Lisp_Object arg,
                                    Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

void
headless_set_background_color (struct frame *f, Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

void
headless_set_cursor_color (struct frame *f, Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

void
headless_set_cursor_type (struct frame *f, Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_mouse_color (struct frame *f, Lisp_Object arg, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_use_frame_synchronization (struct frame *f, Lisp_Object arg,
                                        Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_sticky (struct frame *f, Lisp_Object new_value,
                     Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_inhibit_double_buffering (struct frame *f,
                                       Lisp_Object new_value,
                                       Lisp_Object old_value)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_no_focus_on_map (struct frame *f, Lisp_Object value,
                              Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_tool_bar_lines (struct frame *f, Lisp_Object value, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

static void
headless_set_tab_bar_lines (struct frame *f, Lisp_Object value, Lisp_Object oldval)
{
  TRACE_FUNC_CALL;
}

void
frame_set_mouse_pixel_position (struct frame *f, int pix_x, int pix_y)
{
  TRACE_FUNC_CALL;
}

DEFUN ("xw-display-color-p", Fxw_display_color_p, Sxw_display_color_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  TRACE_FUNC_CALL;
  return Qt;
}

DEFUN ("xw-color-defined-p", Fxw_color_defined_p, Sxw_color_defined_p, 1, 2, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object color, Lisp_Object frame)
{
  Emacs_Color col;

  CHECK_STRING (color);
  decode_window_system_frame (frame);

  TRACE_FUNC_CALL;
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

  TRACE_FUNC_CALL;
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
  TRACE_FUNC_CALL;

  return Qnil;
}

DEFUN ("x-open-connection", Fx_open_connection, Sx_open_connection,
       1, 3, 0, doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display, Lisp_Object resource_string, Lisp_Object must_succeed)
{
  CHECK_STRING (display);
  TRACE_FUNC_CALL;

  if (NILP (Fstring_equal (display, build_string ("headless"))))
    {
      if (!NILP (must_succeed) || true)
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
  TRACE_FUNC_CALL;

  return make_fixnum (width);
}

DEFUN ("x-display-pixel-height", Fx_display_pixel_height, Sx_display_pixel_height,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)

{
  int width, height;
  check_headless_display_info (terminal);
  TRACE_FUNC_CALL;

  return make_fixnum (height);
}

DEFUN ("x-display-mm-height", Fx_display_mm_height, Sx_display_mm_height, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  struct headless_display_info *dpyinfo = check_headless_display_info (terminal);
  int width, height;
  TRACE_FUNC_CALL;

  return make_fixnum (height / (dpyinfo->resy / 25.4));
}


DEFUN ("x-display-mm-width", Fx_display_mm_width, Sx_display_mm_width, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  struct headless_display_info *dpyinfo = check_headless_display_info (terminal);
  int width, height;

  TRACE_FUNC_CALL;
  return make_fixnum (width / (dpyinfo->resx / 25.4));
}

DEFUN ("x-create-frame", Fx_create_frame, Sx_create_frame,
       1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object parms)
{
  TRACE_FUNC_CALL;
  return headless_create_frame (parms);
}

DEFUN ("x-display-visual-class", Fx_display_visual_class,
       Sx_display_visual_class, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  check_headless_display_info (terminal);

  return Qtrue_color;
}

DEFUN ("x-show-tip", Fx_show_tip, Sx_show_tip, 1, 6, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object string, Lisp_Object frame, Lisp_Object parms,
   Lisp_Object timeout, Lisp_Object dx, Lisp_Object dy)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("x-hide-tip", Fx_hide_tip, Sx_hide_tip, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("x-close-connection", Fx_close_connection, Sx_close_connection, 1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */
       attributes: noreturn)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);
  TRACE_FUNC_CALL;

  error ("Cannot close Headless displays");
}

DEFUN ("x-display-list", Fx_display_list, Sx_display_list, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c. */)
  (void)
{
  TRACE_FUNC_CALL;
  if (!x_display_list)
    return Qnil;

  return list1 (XCAR (x_display_list->name_list_element));
}

DEFUN ("x-server-vendor", Fx_server_vendor, Sx_server_vendor, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  check_headless_display_info (terminal);
  return build_string ("Headless, Inc.");
}

DEFUN ("x-server-version", Fx_server_version, Sx_server_version, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c. */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  check_headless_display_info (terminal);
  return list3i (5, 1, 1);
}

DEFUN ("x-display-screens", Fx_display_screens, Sx_display_screens, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  check_headless_display_info (terminal);
  return make_fixnum (1);
}

DEFUN ("headless-get-version-string", Fheadless_get_version_string,
       Sheadless_get_version_string, 0, 0, 0,
       doc: /* Return a string describing the current Headless version.  */)
  (void)
{
  TRACE_FUNC_CALL;
  return build_string ("version 1.0");
}

DEFUN ("x-display-color-cells", Fx_display_color_cells, Sx_display_color_cells,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  check_headless_display_info (terminal);

  return make_fixnum (1);
}

DEFUN ("x-display-planes", Fx_display_planes, Sx_display_planes,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  check_headless_display_info (terminal);

  return make_fixnum (1);
}

DEFUN ("x-double-buffered-p", Fx_double_buffered_p, Sx_double_buffered_p,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object frame)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-set-mouse-absolute-pixel-position",
       Fheadless_set_mouse_absolute_pixel_position,
       Sheadless_set_mouse_absolute_pixel_position, 2, 2, 0,
       doc: /* Move mouse pointer to a pixel position at (X, Y).  The
               coordinates X and Y are interpreted to start from the top-left
               corner of the screen.  */)
  (Lisp_Object x, Lisp_Object y)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-mouse-absolute-pixel-position", Fheadless_mouse_absolute_pixel_position,
       Sheadless_mouse_absolute_pixel_position, 0, 0, 0,
       doc: /* Return absolute position of mouse cursor in pixels.
               The position is returned as a cons cell (X . Y) of the coordinates of
               the mouse cursor position in pixels relative to a position (0, 0) of the
               selected frame's display.  */)
  (void)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-frame-geometry", Fheadless_frame_geometry, Sheadless_frame_geometry, 0, 1, 0,
       doc: /* Return geometric attributes of FRAME.
               FRAME must be a live frame and defaults to the selected one.  The return
               value is an association list of the attributes listed below.  All height
               and width values are in pixels.

               `outer-position' is a cons of the outer left and top edges of FRAME
               relative to the origin - the position (0, 0) - of FRAME's display.

               `outer-size' is a cons of the outer width and height of FRAME.  The
               outer size includes the title bar and the external borders as well as
               any menu and/or tool bar of frame.

               `external-border-size' is a cons of the horizontal and vertical width of
               FRAME's external borders as supplied by the window manager.

               `title-bar-size' is a cons of the width and height of the title bar of
               FRAME as supplied by the window manager.  If both of them are zero,
               FRAME has no title bar.  If only the width is zero, Emacs was not
               able to retrieve the width information.

               `menu-bar-external', if non-nil, means the menu bar is external (never
               included in the inner edges of FRAME).

               `menu-bar-size' is a cons of the width and height of the menu bar of
               FRAME.

               `tool-bar-external', if non-nil, means the tool bar is external (never
               included in the inner edges of FRAME).

               `tool-bar-position' tells on which side the tool bar on FRAME is and can
               be one of `left', `top', `right' or `bottom'.  If this is nil, FRAME
               has no tool bar.

               `tool-bar-size' is a cons of the width and height of the tool bar of
               FRAME.

               `internal-border-width' is the width of the internal border of
               FRAME.  */)
  (Lisp_Object frame)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-frame-edges", Fheadless_frame_edges, Sheadless_frame_edges, 0, 2, 0,
       doc: /* Return edge coordinates of FRAME.
               FRAME must be a live frame and defaults to the selected one.  The return
               value is a list of the form (LEFT, TOP, RIGHT, BOTTOM).  All values are
               in pixels relative to the origin - the position (0, 0) - of FRAME's
               display.

               If optional argument TYPE is the symbol `outer-edges', return the outer
               edges of FRAME.  The outer edges comprise the decorations of the window
               manager (like the title bar or external borders) as well as any external
               menu or tool bar of FRAME.  If optional argument TYPE is the symbol
               `native-edges' or nil, return the native edges of FRAME.  The native
               edges exclude the decorations of the window manager and any external
               menu or tool bar of FRAME.  If TYPE is the symbol `inner-edges', return
               the inner edges of FRAME.  These edges exclude title bar, any borders,
               menu bar or tool bar of FRAME.  */)
  (Lisp_Object frame, Lisp_Object type)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-read-file-name", Fheadless_read_file_name, Sheadless_read_file_name, 1, 6, 0,
       doc: /* Use a graphical panel to read a file name, using prompt PROMPT.
               Optional arg FRAME specifies a frame on which to display the file panel.
               If it is nil, the current frame is used instead.
               The frame being used will be brought to the front of
               the display after the file panel is closed.
               Optional arg DIR, if non-nil, supplies a default directory.
               Optional arg MUSTMATCH, if non-nil, means the returned file or
               directory must exist.
               Optional arg DIR_ONLY_P, if non-nil, means choose only directories.
               Optional arg SAVE_TEXT, if non-nil, specifies some text to show in the entry field.  */)
  (Lisp_Object prompt, Lisp_Object frame, Lisp_Object dir,
   Lisp_Object mustmatch, Lisp_Object dir_only_p, Lisp_Object save_text)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-put-resource", Fheadless_put_resource, Sheadless_put_resource,
       2, 2, 0, doc: /* Place STRING by the key RESOURCE in the resource database.
                        It can later be retrieved with `x-get-resource'.  */)
  (Lisp_Object resource, Lisp_Object string)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-frame-list-z-order", Fheadless_frame_list_z_order,
       Sheadless_frame_list_z_order, 0, 1, 0,
       doc: /* Return list of Emacs' frames, in Z (stacking) order.
               If TERMINAL is non-nil and specifies a live frame, return the child
               frames of that frame in Z (stacking) order.

               As it is impossible to reliably determine the frame stacking order on
               Headless, the selected frame is always the first element of the returned
               list, while the rest are not guaranteed to be in any particular order.

               Frames are listed from topmost (first) to bottommost (last).  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("x-display-save-under", Fx_display_save_under,
       Sx_display_save_under, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-frame-restack", Fheadless_frame_restack, Sheadless_frame_restack, 2, 3, 0,
       doc: /* Restack FRAME1 below FRAME2.
               This means that if both frames are visible and the display areas of
               these frames overlap, FRAME2 (partially) obscures FRAME1.  If optional
               third argument ABOVE is non-nil, restack FRAME1 above FRAME2.  This
               means that if both frames are visible and the display areas of these
               frames overlap, FRAME1 (partially) obscures FRAME2.

               Some window managers may refuse to restack windows.  */)
  (Lisp_Object frame1, Lisp_Object frame2, Lisp_Object above)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-save-session-reply", Fheadless_save_session_reply,
       Sheadless_save_session_reply, 1, 1, 0,
       doc: /* Reply to a `save-session' event.
               QUIT-REPLY means whether or not all files were saved and program
               termination should proceed.

               Calls to this function must be balanced by the amount of
               `save-session' events received.  This is done automatically, so do not
               call this function yourself.  */)
  (Lisp_Object quit_reply)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("headless-display-monitor-attributes-list",
       Fheadless_display_monitor_attributes_list,
       Sheadless_display_monitor_attributes_list,
       0, 1, 0,
       doc: /* Return a list of physical monitor attributes on the display TERMINAL.

The optional argument TERMINAL specifies which display to ask about.
TERMINAL should be a terminal object, a frame or a display name (a string).
If omitted or nil, that stands for the selected frame's display.

Internal use only, use `display-monitor-attributes-list' instead.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
  return Qnil;
}

DEFUN ("x-display-backing-store", Fx_display_backing_store, Sx_display_backing_store,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  TRACE_FUNC_CALL;
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
  TRACE_FUNC_CALL;
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

void
headless_free_frame_resources (struct frame *f)
{
  headless window, drawable, mbar;
  Mouse_HLInfo *hlinfo;
  struct headless_display_info *dpyinfo;
  Lisp_Object bar;
  struct scroll_bar *b;

  TRACE_FUNC_CALL;

  check_window_system (f);
  block_input ();

  hlinfo = MOUSE_HL_INFO (f);
  window = FRAME_HEADLESS_WINDOW (f);
  drawable = FRAME_HEADLESS_VIEW (f);
  mbar = FRAME_HEADLESS_MENU_BAR (f);
  dpyinfo = FRAME_DISPLAY_INFO (f);

  free_frame_faces (f);
  /* headless_free_custom_cursors (f); */

  /* Free scroll bars */
  for (bar = FRAME_SCROLL_BARS (f); !NILP (bar); bar = b->next)
    {
      b = XSCROLL_BAR (bar);
      /* headless_scroll_bar_remove (b); */
    }

  if (f == dpyinfo->highlight_frame)
    dpyinfo->highlight_frame = 0;
  if (f == dpyinfo->focused_frame)
    dpyinfo->focused_frame = 0;
  if (f == dpyinfo->last_mouse_motion_frame)
    dpyinfo->last_mouse_motion_frame = NULL;
  if (f == dpyinfo->last_mouse_frame)
    dpyinfo->last_mouse_frame = NULL;
  if (f == dpyinfo->focus_event_frame)
    dpyinfo->focus_event_frame = NULL;

  if (f == hlinfo->mouse_face_mouse_frame)
    reset_mouse_highlight (hlinfo);

  if (mbar)
    {
      /* BMenuBar_delete (mbar); */
      if (f->output_data.headless->menu_bar_open_p)
        {
          f->output_data.headless->menu_bar_open_p = 0;
        }
    }

  /* if (drawable) */
  /*   BView_emacs_delete (drawable); */

  /* if (window) */
  /*   BWindow_quit (window); */

  if (FRAME_OUTPUT_DATA (f)->saved_menu_event)
    xfree (FRAME_OUTPUT_DATA (f)->saved_menu_event);

  xfree (FRAME_OUTPUT_DATA (f));
  FRAME_OUTPUT_DATA (f) = NULL;

  unblock_input ();
}
