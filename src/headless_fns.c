#include <config.h>
#include <math.h>

#include "lisp.h"
#include "headless_term.h"
#include "blockinput.h"
#include "keyboard.h"
#include "buffer.h"
#include "headless_gui.h"
#include "pdumper.h"

/* Some kind of reference count for the image cache.  */
static ptrdiff_t image_cache_refcount;

DEFUN ("x-display-list", Fx_display_list, Sx_display_list, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  Lisp_Object result;

  result = Qnil;

  if (x_display_list)
    result = Fcons (XCAR (x_display_list->name_list_element),
		    result);

  return result;
}

static struct headless_display_info *
headless_display_info_for_name (Lisp_Object name)
{
  struct headless_display_info *dpyinfo;

  CHECK_STRING (name);

  for (dpyinfo = x_display_list; dpyinfo; dpyinfo = dpyinfo->next)
    {
      if (!NILP (Fstring_equal (XCAR (dpyinfo->name_list_element),
				name)))
	return dpyinfo;
    }

  error ("Cannot connect to Headless if it was not initialized"
	 " at startup");
}

static struct headless_display_info *
check_headless_display_info (Lisp_Object object)
{
  struct headless_display_info *dpyinfo;
  struct frame *sf, *f;
  struct terminal *t;

  if (NILP (object))
    {
      sf = XFRAME (selected_frame);

      if (FRAME_HEADLESS_P (sf) && FRAME_LIVE_P (sf))
	dpyinfo = FRAME_DISPLAY_INFO (sf);
      else if (x_display_list)
	dpyinfo = x_display_list;
      else
	error ("Headless windows are not in use or not initialized");
    }
  else if (TERMINALP (object))
    {
      t = decode_live_terminal (object);

      if (t->type != output_headless)
        error ("Terminal %d is not an Headless display", t->id);

      dpyinfo = t->display_info.headless;
    }
  else if (STRINGP (object))
    dpyinfo = headless_display_info_for_name (object);
  else
    {
      f = decode_window_system_frame (object);
      dpyinfo = FRAME_DISPLAY_INFO (f);
    }

  return dpyinfo;
}

void
headless_free_frame_resources (struct frame *f)
{
  struct headless_display_info *dpyinfo;
  Mouse_HLInfo *hlinfo;

  dpyinfo = FRAME_DISPLAY_INFO (f);
  hlinfo = &dpyinfo->mouse_highlight;

  block_input ();
  free_frame_faces (f);

  if (f == dpyinfo->focus_frame)
    dpyinfo->focus_frame = 0;
  if (f == dpyinfo->x_focus_event_frame)
    dpyinfo->x_focus_event_frame = 0;
  if (f == dpyinfo->highlight_frame)
    dpyinfo->highlight_frame = 0;
  if (f == hlinfo->mouse_face_mouse_frame)
    reset_mouse_highlight (hlinfo);

  /* These two need to be freed now that they are used to compute the
     mouse position, I think.  */
  if (f == dpyinfo->last_mouse_motion_frame)
    dpyinfo->last_mouse_motion_frame = NULL;
  if (f == dpyinfo->last_mouse_frame)
    dpyinfo->last_mouse_frame = NULL;

  unblock_input ();
}

Display_Info *
check_x_display_info (Lisp_Object object)
{
  return check_headless_display_info (object);
}

static Lisp_Object
unwind_create_frame (Lisp_Object frame)
{
  struct frame *f = XFRAME (frame);

  /* If frame is already dead, nothing to do.  This can happen if the
     display is disconnected after the frame has become official, but
     before Fx_create_frame removes the unwind protect.  */
  if (!FRAME_LIVE_P (f))
    return Qnil;

  /* If frame is ``official'', nothing to do.  */
  if (NILP (Fmemq (frame, Vframe_list)))
    {
      /* If the frame's image cache refcount is still the same as our
	 private shadow variable, it means we are unwinding a frame
	 for which we didn't yet call init_frame_faces, where the
	 refcount is incremented.  Therefore, we increment it here, so
	 that free_frame_faces, called in x_free_frame_resources
	 below, will not mistakenly decrement the counter that was not
	 incremented yet to account for this new frame.  */
      if (FRAME_IMAGE_CACHE (f) != NULL
	  && FRAME_IMAGE_CACHE (f)->refcount == image_cache_refcount)
	FRAME_IMAGE_CACHE (f)->refcount++;

      headless_free_frame_resources (f);
      free_glyphs (f);
      return Qt;
    }

  return Qnil;
}

static void
do_unwind_create_frame (Lisp_Object frame)
{
  unwind_create_frame (frame);
}

void
headless_default_font_parameter (struct frame *f, Lisp_Object parms)
{
  struct headless_display_info *dpyinfo = FRAME_DISPLAY_INFO (f);
  Lisp_Object font_param = gui_display_get_arg (dpyinfo, parms, Qfont, NULL, NULL,
                                                RES_TYPE_STRING);
  Lisp_Object font = Qnil;
  if (BASE_EQ (font_param, Qunbound))
    font_param = Qnil;

  if (NILP (font))
    font = (!NILP (font_param)
	    ? font_param
	    : gui_display_get_arg (dpyinfo, parms,
				   Qfont, "font", "Font",
				   RES_TYPE_STRING));

  if (! FONTP (font) && ! STRINGP (font))
    {
      const char *names[] = {
	"Droid Sans Mono-12",
	"Monospace-12",
	"DroidSansMono-12",
	NULL
      };
      int i;

      for (i = 0; names[i]; i++)
	{
	  font = font_open_by_name (f, build_unibyte_string (names[i]));
	  if (! NILP (font))
	    break;
	}

      if (NILP (font))
	error ("No suitable font was found");
    }

  gui_default_parameter (f, parms, Qfont, font, "font", "Font", RES_TYPE_STRING);
}

frame_parm_handler headless_frame_parm_handlers[] =
{
  gui_set_autoraise,
  gui_set_autolower,
  NULL, //android_set_background_color,
  NULL, //android_set_border_color,
  gui_set_border_width,
  NULL, //android_set_cursor_color,
  NULL, //android_set_cursor_type,
  gui_set_font,
  NULL, //android_set_foreground_color,
  NULL,
  NULL,
  NULL, //android_set_child_frame_border_width,
  NULL, //android_set_internal_border_width,
  gui_set_right_divider_width,
  gui_set_bottom_divider_width,
  NULL, //android_set_menu_bar_lines,
  NULL, //android_set_mouse_color,
  NULL, //android_explicitly_set_name,
  gui_set_scroll_bar_width,
  gui_set_scroll_bar_height,
  NULL, //android_set_title,
  gui_set_unsplittable,
  gui_set_vertical_scroll_bars,
  gui_set_horizontal_scroll_bars,
  gui_set_visibility,
  NULL, //android_set_tab_bar_lines,
  NULL, //android_set_tool_bar_lines,
  NULL,
  NULL,
  gui_set_screen_gamma,
  gui_set_line_spacing,
  gui_set_left_fringe,
  gui_set_right_fringe,
  NULL,
  gui_set_fullscreen,
  gui_set_font_backend,
  NULL, //android_set_alpha,
  NULL,
  NULL, //android_set_tool_bar_position,
  NULL,
  NULL,
  NULL, //android_set_parent_frame,
  NULL,
  NULL, //android_set_no_focus_on_map,
  NULL, //android_set_no_accept_focus,
  NULL,
  NULL,
  gui_set_no_special_glyphs,
  NULL,
  NULL,
};

DEFUN ("x-hide-tip", Fx_hide_tip, Sx_hide_tip, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  return Qnil;
}

DEFUN ("x-create-frame", Fx_create_frame, Sx_create_frame,
       1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object parms)
{
  struct frame *f;
  Lisp_Object frame, tem;
  Lisp_Object name;
  bool minibuffer_only;
  bool undecorated, override_redirect;
  long window_prompting;
  specpdl_ref count;
  Lisp_Object display;
  struct headless_display_info *dpyinfo;
  Lisp_Object parent, parent_frame;
  struct kboard *kb;

  minibuffer_only = false;
  undecorated = false;
  override_redirect = false;
  window_prompting = 0;
  count = SPECPDL_INDEX ();
  dpyinfo = NULL;

  /* Not actually used, but be consistent with X.  */
  ((void) window_prompting);

  parms = Fcopy_alist (parms);

  /* Use this general default value to start with
     until we know if this frame has a specified name.  */
  Vx_resource_name = Vinvocation_name;

  display = gui_display_get_arg (dpyinfo, parms, Qterminal, 0, 0,
                                 RES_TYPE_NUMBER);
  if (BASE_EQ (display, Qunbound))
    display = gui_display_get_arg (dpyinfo, parms, Qdisplay, 0, 0,
                                   RES_TYPE_STRING);
  if (BASE_EQ (display, Qunbound))
    display = Qnil;
  dpyinfo = check_headless_display_info (display);
  kb = dpyinfo->terminal->kboard;

  if (!dpyinfo->terminal->name)
    error ("Terminal is not live, can't create new frames on it");

  name = gui_display_get_arg (dpyinfo, parms, Qname, "name", "Name",
                              RES_TYPE_STRING);
  if (!STRINGP (name)
      && ! BASE_EQ (name, Qunbound)
      && ! NILP (name))
    error ("Invalid frame name--not a string or nil");

  if (STRINGP (name))
    Vx_resource_name = name;

  /* See if parent window is specified.  */
  parent = gui_display_get_arg (dpyinfo, parms, Qparent_id, NULL, NULL,
                                RES_TYPE_NUMBER);
  if (BASE_EQ (parent, Qunbound))
    parent = Qnil;
  if (! NILP (parent))
    CHECK_FIXNUM (parent);

  frame = Qnil;
  tem = gui_display_get_arg (dpyinfo,
                             parms, Qminibuffer, "minibuffer", "Minibuffer",
                             RES_TYPE_SYMBOL);
  if (EQ (tem, Qnone) || NILP (tem))
    f = make_frame_without_minibuffer (Qnil, kb, display);
  else if (EQ (tem, Qonly))
    {
      f = make_minibuffer_frame ();
      minibuffer_only = true;
    }
  else if (WINDOWP (tem))
    f = make_frame_without_minibuffer (tem, kb, display);
  else
    f = make_frame (true);

  parent_frame = gui_display_get_arg (dpyinfo,
                                      parms,
                                      Qparent_frame,
                                      NULL,
                                      NULL,
                                      RES_TYPE_SYMBOL);
  /* Accept parent-frame iff parent-id was not specified.  */
  if (!NILP (parent)
      || BASE_EQ (parent_frame, Qunbound)
      || NILP (parent_frame)
      || !FRAMEP (parent_frame)
      || !FRAME_LIVE_P (XFRAME (parent_frame))
      || !FRAME_HEADLESS_P (XFRAME (parent_frame)))
    parent_frame = Qnil;

  fset_parent_frame (f, parent_frame);
  store_frame_param (f, Qparent_frame, parent_frame);

  if (!NILP (tem = (gui_display_get_arg (dpyinfo,
                                         parms,
                                         Qundecorated,
                                         NULL,
                                         NULL,
                                         RES_TYPE_BOOLEAN)))
      && !(BASE_EQ (tem, Qunbound)))
    undecorated = true;

  FRAME_UNDECORATED (f) = undecorated;
  store_frame_param (f, Qundecorated, undecorated ? Qt : Qnil);

  if (!NILP (tem = (gui_display_get_arg (dpyinfo,
                                         parms,
                                         Qoverride_redirect,
                                         NULL,
                                         NULL,
                                         RES_TYPE_BOOLEAN)))
      && !(BASE_EQ (tem, Qunbound)))
    override_redirect = true;

  FRAME_OVERRIDE_REDIRECT (f) = override_redirect;
  store_frame_param (f, Qoverride_redirect, override_redirect ? Qt : Qnil);

  XSETFRAME (frame, f);

  f->terminal = dpyinfo->terminal;

  f->output_method = output_headless;
  f->output_data.headless = xzalloc (sizeof *f->output_data.headless);
  FRAME_FONTSET (f) = -1;
  f->output_data.headless->scroll_bar_foreground_pixel = -1;
  f->output_data.headless->scroll_bar_background_pixel = -1;
  /* f->output_data.headless->white_relief.pixel = -1; */
  /* f->output_data.headless->black_relief.pixel = -1; */

  fset_icon_name (f, gui_display_get_arg (dpyinfo,
                                          parms,
                                          Qicon_name,
                                          "iconName",
                                          "Title",
                                          RES_TYPE_STRING));
  if (! STRINGP (f->icon_name))
    fset_icon_name (f, Qnil);

  FRAME_DISPLAY_INFO (f) = dpyinfo;

  /* With FRAME_DISPLAY_INFO set up, this unwind-protect is safe.  */
  record_unwind_protect (do_unwind_create_frame, frame);

  /* These colors will be set anyway later, but it's important
     to get the color reference counts right, so initialize them!

     (Not really on Headless, but it's best to be consistent with
     X.) */
  {
    Lisp_Object black;

    /* Function x_decode_color can signal an error.  Make
       sure to initialize color slots so that we won't try
       to free colors we haven't allocated.  */
    FRAME_FOREGROUND_PIXEL (f) = -1;
    FRAME_BACKGROUND_PIXEL (f) = -1;
    f->output_data.headless->cursor_pixel = -1;
    f->output_data.headless->cursor_foreground_pixel = -1;
    f->output_data.headless->mouse_pixel = -1;

    black = build_string ("black");
    /* FRAME_FOREGROUND_PIXEL (f) */
    /*   = android_decode_color (f, black, BLACK_PIX_DEFAULT (f)); */
    /* FRAME_BACKGROUND_PIXEL (f) */
    /*   = android_decode_color (f, black, BLACK_PIX_DEFAULT (f)); */
    /* f->output_data.android->cursor_pixel */
    /*   = android_decode_color (f, black, BLACK_PIX_DEFAULT (f)); */
    /* f->output_data.android->cursor_foreground_pixel */
    /*   = android_decode_color (f, black, BLACK_PIX_DEFAULT (f)); */
    /* f->output_data.android->mouse_pixel */
    /*   = android_decode_color (f, black, BLACK_PIX_DEFAULT (f)); */
  }

  /* Set the name; the functions to which we pass f expect the name to
     be set.  */
  if (BASE_EQ (name, Qunbound) || NILP (name))
    {
      fset_name (f, build_string ("GNU Emacs"));
      f->explicit_name = false;
    }
  else
    {
      fset_name (f, name);
      f->explicit_name = true;
      /* Use the frame's title when getting resources for this frame.  */
      specbind (Qx_resource_name, name);
    }

  register_font_driver (&headlessfont_driver, f);

  image_cache_refcount = (FRAME_IMAGE_CACHE (f)
			  ? FRAME_IMAGE_CACHE (f)->refcount
			  : 0);

  gui_default_parameter (f, parms, Qfont_backend, Qnil,
                         "fontBackend", "FontBackend", RES_TYPE_STRING);

  /* Extract the window parameters from the supplied values
     that are needed to determine window geometry.  */
  headless_default_font_parameter (f, parms);
  if (!FRAME_FONT (f))
    {
      delete_frame (frame, Qnoelisp);
      error ("Invalid frame font");
    }
  /* Extract the window parameters from the supplied values
     that are needed to determine window geometry.  */

  if (NILP (Fassq (Qinternal_border_width, parms)))
    {
      Lisp_Object value;

      value = gui_display_get_arg (dpyinfo, parms, Qinternal_border_width,
                                   "internalBorder", "internalBorder",
                                   RES_TYPE_NUMBER);
      if (! BASE_EQ (value, Qunbound))
	parms = Fcons (Fcons (Qinternal_border_width, value),
		       parms);
    }

  gui_default_parameter (f, parms, Qinternal_border_width,
			 make_fixnum (0),
			 "internalBorderWidth", "internalBorderWidth",
			 RES_TYPE_NUMBER);

  /* Same for child frames.  */
  if (NILP (Fassq (Qchild_frame_border_width, parms)))
    {
      Lisp_Object value;

      value = gui_display_get_arg (dpyinfo, parms, Qchild_frame_border_width,
                                   "childFrameBorder", "childFrameBorder",
                                   RES_TYPE_NUMBER);
      if (! BASE_EQ (value, Qunbound))
	parms = Fcons (Fcons (Qchild_frame_border_width, value),
		       parms);
    }

  gui_default_parameter (f, parms, Qchild_frame_border_width, Qnil,
			 "childFrameBorderWidth", "childFrameBorderWidth",
			 RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qright_divider_width, make_fixnum (0),
                         NULL, NULL, RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qbottom_divider_width, make_fixnum (0),
                         NULL, NULL, RES_TYPE_NUMBER);

  /* `vertical-scroll-bars' defaults to nil on Headless as a
     consequence of scroll bars not being supported at all.  */

  gui_default_parameter (f, parms, Qvertical_scroll_bars, Qnil,
                         "verticalScrollBars", "ScrollBars",
                         RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qhorizontal_scroll_bars, Qnil,
                         "horizontalScrollBars", "ScrollBars",
                         RES_TYPE_SYMBOL);

  /* Also do the stuff which must be set before the window exists.  */
  gui_default_parameter (f, parms, Qforeground_color, build_string ("black"),
                         "foreground", "Foreground", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qbackground_color, build_string ("white"),
                         "background", "Background", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qmouse_color, build_string ("black"),
                         "pointerColor", "Foreground", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qborder_color, build_string ("black"),
                         "borderColor", "BorderColor", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qscreen_gamma, Qnil,
                         "screenGamma", "ScreenGamma", RES_TYPE_FLOAT);
  gui_default_parameter (f, parms, Qline_spacing, Qnil,
                         "lineSpacing", "LineSpacing", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qleft_fringe, Qnil,
                         "leftFringe", "LeftFringe", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qright_fringe, Qnil,
                         "rightFringe", "RightFringe", RES_TYPE_NUMBER);
  gui_default_parameter (f, parms, Qno_special_glyphs, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);

  /* Init faces before gui_default_parameter is called for the
     scroll-bar-width parameter because otherwise we end up in
     init_iterator with a null face cache, which should not
     happen.  */

  init_frame_faces (f);

  tem = gui_display_get_arg (dpyinfo, parms, Qmin_width, NULL, NULL,
                             RES_TYPE_NUMBER);
  if (FIXNUMP (tem))
    store_frame_param (f, Qmin_width, tem);
  tem = gui_display_get_arg (dpyinfo, parms, Qmin_height, NULL, NULL,
                             RES_TYPE_NUMBER);
  if (FIXNUMP (tem))
    store_frame_param (f, Qmin_height, tem);

  adjust_frame_size (f, FRAME_COLS (f) * FRAME_COLUMN_WIDTH (f),
		     FRAME_LINES (f) * FRAME_LINE_HEIGHT (f), 5, true,
		     Qx_create_frame_1);

  /* Set the menu-bar-lines and tool-bar-lines parameters.  We don't
     look up the X resources controlling the menu-bar and tool-bar
     here; they are processed specially at startup, and reflected in
     the values of the mode variables.  */

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

  gui_default_parameter (f, parms, Qbuffer_predicate, Qnil,
                         "bufferPredicate", "BufferPredicate",
                         RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qtitle, Qnil,
                         "title", "Title", RES_TYPE_STRING);
  gui_default_parameter (f, parms, Qwait_for_wm, Qt,
                         "waitForWM", "WaitForWM", RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qtool_bar_position,
                         FRAME_TOOL_BAR_POSITION (f), 0, 0, RES_TYPE_SYMBOL);
  gui_default_parameter (f, parms, Qinhibit_double_buffering, Qnil,
                         "inhibitDoubleBuffering", "InhibitDoubleBuffering",
                         RES_TYPE_BOOLEAN);

  /* Compute the size of the X window.  */
  window_prompting = gui_figure_window_size (f, parms, true, true);

  tem = gui_display_get_arg (dpyinfo, parms, Qunsplittable, 0, 0,
                             RES_TYPE_BOOLEAN);
  f->no_split = minibuffer_only || EQ (tem, Qt);

  /* Now consider the frame official.  */
  f->terminal->reference_count++;
  Vframe_list = Fcons (frame, Vframe_list);

  /* We need to do this after creating the window, so that the
     icon-creation functions can say whose icon they're
     describing.  */
  gui_default_parameter (f, parms, Qicon_type, Qt,
                         "bitmapIcon", "BitmapIcon", RES_TYPE_BOOLEAN);

  gui_default_parameter (f, parms, Qauto_raise, Qnil,
                         "autoRaise", "AutoRaiseLower", RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qauto_lower, Qnil,
                         "autoLower", "AutoRaiseLower", RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qcursor_type, Qbox,
                         "cursorType", "CursorType", RES_TYPE_SYMBOL);
  /* Scroll bars are not supported on Headless, as they are near
     useless.  */
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

  if (!NILP (parent_frame))
    {
      struct frame *p = XFRAME (parent_frame);
    }

  gui_default_parameter (f, parms, Qno_focus_on_map, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);
  gui_default_parameter (f, parms, Qno_accept_focus, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);

  /* Consider frame official, now.  */
  f->can_set_window_size = true;

  adjust_frame_size (f, FRAME_TEXT_WIDTH (f), FRAME_TEXT_HEIGHT (f),
		     0, true, Qx_create_frame_2);

  /* Process fullscreen parameter here in the hope that normalizing a
     fullheight/fullwidth frame will produce the size set by the last
     adjust_frame_size call.  Note that Headless only supports the
     `maximized' state.  */
  gui_default_parameter (f, parms, Qfullscreen, Qmaximized,
                         "fullscreen", "Fullscreen", RES_TYPE_SYMBOL);

  /* When called from `x-create-frame-with-faces' visibility is
     always explicitly nil.  */
  Lisp_Object visibility
    = gui_display_get_arg (dpyinfo, parms, Qvisibility, 0, 0,
			   RES_TYPE_SYMBOL);
  Lisp_Object height
    = gui_display_get_arg (dpyinfo, parms, Qheight, 0, 0, RES_TYPE_NUMBER);
  Lisp_Object width
    = gui_display_get_arg (dpyinfo, parms, Qwidth, 0, 0, RES_TYPE_NUMBER);

  if (EQ (visibility, Qicon))
    {
      f->was_invisible = true;
    }
  else
    {
      if (BASE_EQ (visibility, Qunbound))
	visibility = Qt;

      if (!NILP (visibility))
        ;
      else
	f->was_invisible = true;
    }

  /* Leave f->was_invisible true only if height or width were
     specified too.  This takes effect only when we are not called
     from `x-create-frame-with-faces' (see above comment).  */
  f->was_invisible
    = (f->was_invisible
       && (!BASE_EQ (height, Qunbound) || !BASE_EQ (width, Qunbound)));

  store_frame_param (f, Qvisibility, visibility);

  /* Set whether or not frame synchronization is enabled.  */
  gui_default_parameter (f, parms, Quse_frame_synchronization, Qt,
			 NULL, NULL, RES_TYPE_BOOLEAN);

  /* Works iff frame has been already mapped.  */
  gui_default_parameter (f, parms, Qskip_taskbar, Qnil,
                         NULL, NULL, RES_TYPE_BOOLEAN);
  /* The `z-group' parameter works only for visible frames.  */
  gui_default_parameter (f, parms, Qz_group, Qnil,
                         NULL, NULL, RES_TYPE_SYMBOL);

  /* Initialize `default-minibuffer-frame' in case this is the first
     frame on this terminal.  */
  if (FRAME_HAS_MINIBUF_P (f)
      && (!FRAMEP (KVAR (kb, Vdefault_minibuffer_frame))
          || !FRAME_LIVE_P (XFRAME (KVAR (kb, Vdefault_minibuffer_frame)))))
    kset_default_minibuffer_frame (kb, frame);

  /* All remaining specified parameters, which have not been "used" by
     gui_display_get_arg and friends, now go in the misc. alist of the
     frame.  */
  for (tem = parms; CONSP (tem); tem = XCDR (tem))
    if (CONSP (XCAR (tem)) && !NILP (XCAR (XCAR (tem))))
      fset_param_alist (f, Fcons (XCAR (tem), f->param_alist));

  /* Make sure windows on this frame appear in calls to next-window
     and similar functions.  */
  Vwindow_list = Qnil;

  return unbind_to (count, frame);
}

DEFUN ("xw-display-color-p", Fxw_display_color_p,
       Sxw_display_color_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  return Qt;
}

DEFUN ("x-display-grayscale-p", Fx_display_grayscale_p,
       Sx_display_grayscale_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object terminal)
{
  return Qnil;
}

void
syms_of_headlessfns (void)
{
  /* Miscellaneous symbols used by some functions here.  */
  DEFSYM (Qtrue_color, "true-color");
  DEFSYM (Qwhen_mapped, "when-mapped");

  DEFVAR_LISP ("x-pointer-shape", Vx_pointer_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_pointer_shape = Qnil;

#if false /* This doesn't really do anything.  */
  DEFVAR_LISP ("x-nontext-pointer-shape", Vx_nontext_pointer_shape,
    doc: /* SKIP: real doc in xfns.c.  */);
#endif
  Vx_nontext_pointer_shape = Qnil;

  DEFVAR_LISP ("x-hourglass-pointer-shape", Vx_hourglass_pointer_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_hourglass_pointer_shape = Qnil;

  DEFVAR_LISP ("x-sensitive-text-pointer-shape",
	      Vx_sensitive_text_pointer_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_sensitive_text_pointer_shape = Qnil;

  DEFVAR_LISP ("x-window-horizontal-drag-cursor",
	      Vx_window_horizontal_drag_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_horizontal_drag_shape = Qnil;

  DEFVAR_LISP ("x-window-vertical-drag-cursor",
	      Vx_window_vertical_drag_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_vertical_drag_shape = Qnil;

  DEFVAR_LISP ("x-window-left-edge-cursor",
	       Vx_window_left_edge_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_left_edge_shape = Qnil;

  DEFVAR_LISP ("x-window-top-left-corner-cursor",
	       Vx_window_top_left_corner_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_top_left_corner_shape = Qnil;

  DEFVAR_LISP ("x-window-top-edge-cursor",
	       Vx_window_top_edge_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_top_edge_shape = Qnil;

  DEFVAR_LISP ("x-window-top-right-corner-cursor",
	       Vx_window_top_right_corner_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_top_right_corner_shape = Qnil;

  DEFVAR_LISP ("x-window-right-edge-cursor",
	       Vx_window_right_edge_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_right_edge_shape = Qnil;

  DEFVAR_LISP ("x-window-bottom-right-corner-cursor",
	       Vx_window_bottom_right_corner_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_bottom_right_corner_shape = Qnil;

  DEFVAR_LISP ("x-window-bottom-edge-cursor",
	       Vx_window_bottom_edge_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_bottom_edge_shape = Qnil;

#if false /* This doesn't really do anything.  */
  DEFVAR_LISP ("x-mode-pointer-shape", Vx_mode_pointer_shape,
    doc: /* SKIP: real doc in xfns.c.  */);
#endif
  Vx_mode_pointer_shape = Qnil;

  DEFVAR_LISP ("x-window-bottom-left-corner-cursor",
	       Vx_window_bottom_left_corner_shape,
    doc: /* SKIP: real text in xfns.c.  */);
  Vx_window_bottom_left_corner_shape = Qnil;

  DEFVAR_LISP ("x-cursor-fore-pixel", Vx_cursor_fore_pixel,
    doc: /* SKIP: real doc in xfns.c.  */);
  Vx_cursor_fore_pixel = Qnil;

  /* Used by Fx_show_tip.  */
  DEFSYM (Qrun_at_time, "run-at-time");
  DEFSYM (Qx_hide_tip, "x-hide-tip");
  DEFSYM (Qcancel_timer, "cancel-timer");
  DEFSYM (Qassq_delete_all, "assq-delete-all");
  DEFSYM (Qcolor, "color");

  DEFVAR_LISP ("x-max-tooltip-size", Vx_max_tooltip_size,
    doc: /* SKIP: real doc in xfns.c.  */);
  Vx_max_tooltip_size = Qnil;

  /* Functions defined.  */
  defsubr (&Sx_create_frame);
  defsubr (&Sxw_display_color_p);
  defsubr (&Sx_display_grayscale_p);
}
