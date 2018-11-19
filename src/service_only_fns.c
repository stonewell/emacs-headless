#include <config.h>

#include <c-ctype.h>
#include <stdio.h>

#include "lisp.h"
#include "service_only_term.h"
#include "frame.h"
#include "window.h"
#include "buffer.h"
#include "keyboard.h"
#include "blockinput.h"
#include "coding.h"

/**
 * x_hide_tip:
 *
 * Hide currently visible tooltip and cancel its timer.
 *
 * This will try to make tooltip_frame invisible (if DELETE is false)
 * or delete tooltip_frame (if DELETE is true).
 *
 * Return Qt if the tooltip was either deleted or made invisible, Qnil
 * otherwise.
 */
static Lisp_Object
x_hide_tip (bool delete)
{
  return Qnil;
}

DEFUN ("x-hide-tip", Fx_hide_tip, Sx_hide_tip, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  return x_hide_tip (!tooltip_reuse_hidden_frame);
}

DEFUN ("xw-display-color-p", Fxw_display_color_p, Sxw_display_color_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  if ((dpyinfo->n_planes * dpyinfo->n_cbits) <= 2)
    return Qnil;

  return Qt;
}

DEFUN ("x-display-grayscale-p", Fx_display_grayscale_p,
       Sx_display_grayscale_p, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  if ((dpyinfo->n_planes * dpyinfo->n_cbits) <= 1)
    return Qnil;

  return Qt;
}

DEFUN ("x-display-pixel-width", Fx_display_pixel_width,
       Sx_display_pixel_width, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  return make_fixnum (x_display_pixel_width (dpyinfo));
}

DEFUN ("x-display-pixel-height", Fx_display_pixel_height,
       Sx_display_pixel_height, 0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  return make_fixnum (x_display_pixel_height (dpyinfo));
}

DEFUN ("x-display-planes", Fx_display_planes, Sx_display_planes,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  return make_fixnum (dpyinfo->n_planes * dpyinfo->n_cbits);
}

DEFUN ("x-display-color-cells", Fx_display_color_cells, Sx_display_color_cells,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);
  int cap;

  /* Don't use NCOLORS: it returns incorrect results under remote
   * desktop.  We force 24+ bit depths to 24-bit, both to prevent an
   * overflow and because probably is more meaningful on Windows
   * anyway.  */

  cap = 1 << min (dpyinfo->n_planes * dpyinfo->n_cbits, 24);
  return make_fixnum (cap);
}

DEFUN ("x-server-max-request-size", Fx_server_max_request_size,
       Sx_server_max_request_size,
       0, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  return make_fixnum (1);
}

GC
x_create_gc (struct frame *f,
	     unsigned long mask,
	     XGCValues *xgcv)
{
  GC gc = xmalloc (sizeof *gc);
  *gc = *xgcv;
  return gc;
}

void
x_free_gc (struct frame *f, GC gc)
{
  xfree (gc);
}

struct so_display_info *
check_x_display_info (Lisp_Object object)
{
  if (NILP (object))
    {
      struct frame *sf = XFRAME (selected_frame);

      if (FRAME_SO_P (sf) && FRAME_LIVE_P (sf))
	return FRAME_DISPLAY_INFO (sf);
      else {
	return &one_so_display_info;
      }
    }
  else if (TERMINALP (object))
    {
      struct terminal *t = decode_live_terminal (object);

      if (t->type != output_service_only)
	error ("Terminal %d is not a service only display", t->id);

      return t->display_info.so;
    }
  else if (STRINGP (object)) {
    return x_display_info_for_name (object);
  }
  else
    {
      struct frame *f;

      CHECK_LIVE_FRAME (object);
      f = XFRAME (object);
      if (! FRAME_SO_P (f))
	error ("Non-ServiceOnly frame used");
      return FRAME_DISPLAY_INFO (f);
    }
}

DEFUN ("x-create-frame", Fx_create_frame, Sx_create_frame,
       1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
     (Lisp_Object parameters) {
  struct frame *f;
  Lisp_Object frame, tem;
  Lisp_Object name;
  bool minibuffer_only = false;
  long window_prompting = 0;
  ptrdiff_t count = SPECPDL_INDEX ();
  Lisp_Object display;
  struct so_display_info *dpyinfo = NULL;
  Lisp_Object parent, parent_frame;
  struct kboard *kb;
  int x_width = 0, x_height = 0;

  if (!FRAME_SO_P (SELECTED_FRAME ())
      && !FRAME_INITIAL_P (SELECTED_FRAME ()))
    error ("Cannot create a GUI frame in a -nw session");

  /* Make copy of frame parameters because the original is in pure
     storage now. */
  parameters = Fcopy_alist (parameters);

  /* Use this general default value to start with
     until we know if this frame has a specified name.  */
  Vx_resource_name = Vinvocation_name;

  display = x_get_arg (dpyinfo, parameters, Qterminal, 0, 0, RES_TYPE_NUMBER);
  if (EQ (display, Qunbound))
    display = x_get_arg (dpyinfo, parameters, Qdisplay, 0, 0, RES_TYPE_STRING);
  if (EQ (display, Qunbound))
    display = Qnil;
  dpyinfo = check_x_display_info (display);
  kb = dpyinfo->terminal->kboard;

  if (!dpyinfo->terminal->name)
    error ("Terminal is not live, can't create new frames on it");

  name = x_get_arg (dpyinfo, parameters, Qname, "name", "Name", RES_TYPE_STRING);
  if (!STRINGP (name)
      && ! EQ (name, Qunbound)
      && ! NILP (name))
    error ("Invalid frame name--not a string or nil");

  if (STRINGP (name))
    Vx_resource_name = name;

  /* See if parent window is specified.  */
  parent = x_get_arg (dpyinfo, parameters, Qparent_id, NULL, NULL,
		      RES_TYPE_NUMBER);
  if (EQ (parent, Qunbound))
    parent = Qnil;
  else if (!NILP (parent))
    CHECK_FIXNUM (parent);

  /* make_frame_without_minibuffer can run Lisp code and garbage collect.  */
  /* No need to protect DISPLAY because that's not used after passing
     it to make_frame_without_minibuffer.  */
  frame = Qnil;
  tem = x_get_arg (dpyinfo, parameters, Qminibuffer, "minibuffer", "Minibuffer",
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

  XSETFRAME (frame, f);

  parent_frame = x_get_arg (dpyinfo, parameters, Qparent_frame, NULL, NULL,
			    RES_TYPE_SYMBOL);
  /* Apply `parent-frame' parameter only when no `parent-id' was
     specified.  */
  if (!NILP (parent_frame)
      && (!NILP (parent)
	  || !FRAMEP (parent_frame)
	  || !FRAME_LIVE_P (XFRAME (parent_frame))
	  || !FRAME_SO_P (XFRAME (parent_frame))))
    parent_frame = Qnil;

  fset_parent_frame (f, parent_frame);
  store_frame_param (f, Qparent_frame, parent_frame);

  tem = x_get_arg (dpyinfo, parameters, Qundecorated, NULL, NULL,
		   RES_TYPE_BOOLEAN);
  FRAME_UNDECORATED (f) = !NILP (tem) && !EQ (tem, Qunbound);
  store_frame_param (f, Qundecorated, FRAME_UNDECORATED (f) ? Qt : Qnil);

  tem = x_get_arg (dpyinfo, parameters, Qskip_taskbar, NULL, NULL,
		   RES_TYPE_BOOLEAN);
  FRAME_SKIP_TASKBAR (f) = !NILP (tem) && !EQ (tem, Qunbound);
  store_frame_param (f, Qskip_taskbar,
		     (NILP (tem) || EQ (tem, Qunbound)) ? Qnil : Qt);

  /* By default, make scrollbars the system standard width and height. */
  /* FRAME_CONFIG_SCROLL_BAR_WIDTH (f) = GetSystemMetrics (SM_CXVSCROLL); */
  /* FRAME_CONFIG_SCROLL_BAR_HEIGHT (f) = GetSystemMetrics (SM_CXHSCROLL); */

  f->terminal = dpyinfo->terminal;

  f->output_method = output_service_only;
  f->output_data.so = xzalloc (sizeof (struct so_output));
  FRAME_FONTSET (f) = -1;

  fset_icon_name
    (f, x_get_arg (dpyinfo, parameters, Qicon_name, "iconName", "Title",
		   RES_TYPE_STRING));
  if (! STRINGP (f->icon_name))
    fset_icon_name (f, Qnil);

  /*  FRAME_DISPLAY_INFO (f) = dpyinfo; */

  /* With FRAME_DISPLAY_INFO set up, this unwind-protect is safe.  */
  /* record_unwind_protect (do_unwind_create_frame, frame); */

#ifdef GLYPH_DEBUG
  image_cache_refcount =
    FRAME_IMAGE_CACHE (f) ? FRAME_IMAGE_CACHE (f)->refcount : 0;
  dpyinfo_refcount = dpyinfo->reference_count;
#endif /* GLYPH_DEBUG */

  /* Specify the parent under which to make this window - this seems to
     have no effect on Windows because parent_desc is explicitly reset
     below.  */
  if (!NILP (parent))
    {
      /* Cast to UINT_PTR shuts up compiler warnings about cast to
	 pointer from integer of different size.  */
      f->output_data.so->parent_desc = (Window) (uintptr_t) XFIXNAT (parent);
      f->output_data.so->explicit_parent = true;
    }
  else
    {
      f->output_data.so->parent_desc = FRAME_DISPLAY_INFO (f)->root_window;
      f->output_data.so->explicit_parent = false;
    }

  /* Set the name; the functions to which we pass f expect the name to
     be set.  */
  if (EQ (name, Qunbound) || NILP (name))
    {
      fset_name (f, build_string (dpyinfo->so_id_name));
      f->explicit_name = false;
    }
  else
    {
      fset_name (f, name);
      f->explicit_name = true;
      /* Use the frame's title when getting resources for this frame.  */
      specbind (Qx_resource_name, name);
    }

  register_font_driver (&sofont_driver, f);

  x_default_parameter (f, parameters, Qfont_backend, Qnil,
		       "fontBackend", "FontBackend", RES_TYPE_STRING);

  /* Extract the window parameters from the supplied values
     that are needed to determine window geometry.  */
  /* x_default_font_parameter (f, parameters); */

  /* Default BorderWidth to 0 to match other platforms.  */
  x_default_parameter (f, parameters, Qborder_width, make_fixnum (0),
		       "borderWidth", "BorderWidth", RES_TYPE_NUMBER);

  /* We recognize either internalBorderWidth or internalBorder
     (which is what xterm calls it).  */
  if (NILP (Fassq (Qinternal_border_width, parameters)))
    {
      Lisp_Object value;

      value = x_get_arg (dpyinfo, parameters, Qinternal_border_width,
			 "internalBorder", "InternalBorder", RES_TYPE_NUMBER);
      if (! EQ (value, Qunbound))
	parameters = Fcons (Fcons (Qinternal_border_width, value),
			    parameters);
    }

  x_default_parameter (f, parameters, Qinternal_border_width, make_fixnum (0),
		       "internalBorderWidth", "InternalBorder", RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qright_divider_width, make_fixnum (0),
		       NULL, NULL, RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qbottom_divider_width, make_fixnum (0),
		       NULL, NULL, RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qvertical_scroll_bars, Qright,
		       "verticalScrollBars", "ScrollBars", RES_TYPE_SYMBOL);
  x_default_parameter (f, parameters, Qhorizontal_scroll_bars, Qnil,
		       "horizontalScrollBars", "ScrollBars", RES_TYPE_SYMBOL);

  /* Also do the stuff which must be set before the window exists.  */
  x_default_parameter (f, parameters, Qforeground_color, build_string ("black"),
		       "foreground", "Foreground", RES_TYPE_STRING);
  x_default_parameter (f, parameters, Qbackground_color, build_string ("white"),
		       "background", "Background", RES_TYPE_STRING);
  x_default_parameter (f, parameters, Qmouse_color, build_string ("black"),
		       "pointerColor", "Foreground", RES_TYPE_STRING);
  x_default_parameter (f, parameters, Qborder_color, build_string ("black"),
		       "borderColor", "BorderColor", RES_TYPE_STRING);
  x_default_parameter (f, parameters, Qscreen_gamma, Qnil,
		       "screenGamma", "ScreenGamma", RES_TYPE_FLOAT);
  x_default_parameter (f, parameters, Qline_spacing, Qnil,
		       "lineSpacing", "LineSpacing", RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qleft_fringe, Qnil,
		       "leftFringe", "LeftFringe", RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qright_fringe, Qnil,
		       "rightFringe", "RightFringe", RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qno_focus_on_map, Qnil,
		       NULL, NULL, RES_TYPE_BOOLEAN);
  x_default_parameter (f, parameters, Qno_accept_focus, Qnil,
		       NULL, NULL, RES_TYPE_BOOLEAN);
  x_default_parameter (f, parameters, Qno_special_glyphs, Qnil,
		       NULL, NULL, RES_TYPE_BOOLEAN);

  /* Process alpha here (Bug#16619).  On XP this fails with child
     frames.  For `no-focus-on-map' frames delay processing of alpha
     until the frame becomes visible.  */
  if (!FRAME_NO_FOCUS_ON_MAP (f))
    x_default_parameter (f, parameters, Qalpha, Qnil,
			 "alpha", "Alpha", RES_TYPE_NUMBER);

  /* Init faces first since we need the frame's column width/line
     height in various occasions.  */
  init_frame_faces (f);

  /* We have to call adjust_frame_size here since otherwise
     x_set_tool_bar_lines will already work with the character sizes
     installed by init_frame_faces while the frame's pixel size is still
     calculated from a character size of 1 and we subsequently hit the
     (height >= 0) assertion in window_box_height.

     The non-pixelwise code apparently worked around this because it
     had one frame line vs one toolbar line which left us with a zero
     root window height which was obviously wrong as well ...

     Also process `min-width' and `min-height' parameters right here
     because `frame-windows-min-size' needs them.  */
  tem = x_get_arg (dpyinfo, parameters, Qmin_width, NULL, NULL,
		   RES_TYPE_NUMBER);
  if (FIXNUMP (tem))
    store_frame_param (f, Qmin_width, tem);
  tem = x_get_arg (dpyinfo, parameters, Qmin_height, NULL, NULL,
		   RES_TYPE_NUMBER);
  if (FIXNUMP (tem))
    store_frame_param (f, Qmin_height, tem);
  adjust_frame_size (f, FRAME_COLS (f) * FRAME_COLUMN_WIDTH (f),
		     FRAME_LINES (f) * FRAME_LINE_HEIGHT (f), 5, true,
		     Qx_create_frame_1);

  /* The X resources controlling the menu-bar and tool-bar are
     processed specially at startup, and reflected in the mode
     variables; ignore them here.  */
  if (NILP (parent_frame))
    {
      x_default_parameter (f, parameters, Qmenu_bar_lines,
			   NILP (Vmenu_bar_mode)
			   ? make_fixnum (0) : make_fixnum (1),
			   NULL, NULL, RES_TYPE_NUMBER);
    }
  else
    /* No menu bar for child frames.  */
    store_frame_param (f, Qmenu_bar_lines, make_fixnum (0));

  x_default_parameter (f, parameters, Qtool_bar_lines,
		       NILP (Vtool_bar_mode)
		       ? make_fixnum (0) : make_fixnum (1),
		       NULL, NULL, RES_TYPE_NUMBER);

  x_default_parameter (f, parameters, Qbuffer_predicate, Qnil,
		       "bufferPredicate", "BufferPredicate", RES_TYPE_SYMBOL);
  x_default_parameter (f, parameters, Qtitle, Qnil,
		       "title", "Title", RES_TYPE_STRING);

  f->output_data.so->parent_desc = FRAME_DISPLAY_INFO (f)->root_window;
  /* f->output_data.so->text_cursor = so_load_cursor (IDC_IBEAM); */
  /* f->output_data.so->nontext_cursor = so_load_cursor (IDC_ARROW); */
  /* f->output_data.so->modeline_cursor = so_load_cursor (IDC_ARROW); */
  /* f->output_data.so->hand_cursor = so_load_cursor (IDC_HAND); */
  /* f->output_data.so->hourglass_cursor = so_load_cursor (IDC_WAIT); */
  /* f->output_data.so->horizontal_drag_cursor = so_load_cursor (IDC_SIZEWE); */
  /* f->output_data.so->vertical_drag_cursor = so_load_cursor (IDC_SIZENS); */
  /* f->output_data.so->left_edge_cursor = so_load_cursor (IDC_SIZEWE); */
  /* f->output_data.so->top_left_corner_cursor = so_load_cursor (IDC_SIZENWSE); */
  /* f->output_data.so->top_edge_cursor = so_load_cursor (IDC_SIZENS); */
  /* f->output_data.so->top_right_corner_cursor = so_load_cursor (IDC_SIZENESW); */
  /* f->output_data.so->right_edge_cursor = so_load_cursor (IDC_SIZEWE); */
  /* f->output_data.so->bottom_right_corner_cursor = so_load_cursor (IDC_SIZENWSE); */
  /* f->output_data.so->bottom_edge_cursor = so_load_cursor (IDC_SIZENS); */
  /* f->output_data.so->bottom_left_corner_cursor = so_load_cursor (IDC_SIZENESW); */

  /* f->output_data.so->current_cursor = f->output_data.so->nontext_cursor; */

  window_prompting = x_figure_window_size (f, parameters, true, &x_width, &x_height);

  tem = x_get_arg (dpyinfo, parameters, Qunsplittable, 0, 0, RES_TYPE_BOOLEAN);
  f->no_split = minibuffer_only || EQ (tem, Qt);

  /* so_window (f, window_prompting, minibuffer_only); */
  /* x_icon (f, parameters); */

  /* x_make_gc (f); */

  /* Now consider the frame official.  */
  f->terminal->reference_count++;
  FRAME_DISPLAY_INFO (f)->reference_count++;
  Vframe_list = Fcons (frame, Vframe_list);

  /* We need to do this after creating the window, so that the
     icon-creation functions can say whose icon they're describing.  */
  x_default_parameter (f, parameters, Qicon_type, Qnil,
		       "bitmapIcon", "BitmapIcon", RES_TYPE_SYMBOL);

  x_default_parameter (f, parameters, Qauto_raise, Qnil,
		       "autoRaise", "AutoRaiseLower", RES_TYPE_BOOLEAN);
  x_default_parameter (f, parameters, Qauto_lower, Qnil,
		       "autoLower", "AutoRaiseLower", RES_TYPE_BOOLEAN);
  x_default_parameter (f, parameters, Qcursor_type, Qbox,
		       "cursorType", "CursorType", RES_TYPE_SYMBOL);
  x_default_parameter (f, parameters, Qscroll_bar_width, Qnil,
		       "scrollBarWidth", "ScrollBarWidth", RES_TYPE_NUMBER);
  x_default_parameter (f, parameters, Qscroll_bar_height, Qnil,
		       "scrollBarHeight", "ScrollBarHeight", RES_TYPE_NUMBER);

  /* Allow x_set_window_size, now.  */
  f->can_x_set_window_size = true;

  if (x_width > 0)
    SET_FRAME_WIDTH (f, x_width);
  if (x_height > 0)
    SET_FRAME_HEIGHT (f, x_height);

  /* Tell the server what size and position, etc, we want, and how
     badly we want them.  This should be done after we have the menu
     bar so that its size can be taken into account.  */
  /* block_input (); */
  /* x_wm_set_size_hint (f, window_prompting, false); */
  /* unblock_input (); */

  adjust_frame_size (f, FRAME_TEXT_WIDTH (f), FRAME_TEXT_HEIGHT (f), 0, true,
		     Qx_create_frame_2);

  /* Process fullscreen parameter here in the hope that normalizing a
     fullheight/fullwidth frame will produce the size set by the last
     adjust_frame_size call.  */
  x_default_parameter (f, parameters, Qfullscreen, Qnil,
		       "fullscreen", "Fullscreen", RES_TYPE_SYMBOL);
  x_default_parameter (f, parameters, Qz_group, Qnil,
		       NULL, NULL, RES_TYPE_SYMBOL);

  /* Make the window appear on the frame and enable display, unless
     the caller says not to.  However, with explicit parent, Emacs
     cannot control visibility, so don't try.  */
  if (!f->output_data.so->explicit_parent)
    {
      Lisp_Object visibility
	= x_get_arg (dpyinfo, parameters, Qvisibility, 0, 0, RES_TYPE_SYMBOL);

      if (EQ (visibility, Qicon)) {
	/* x_iconify_frame (f); */
      }
      else
	{
	  if (EQ (visibility, Qunbound))
	    visibility = Qt;

	  /* if (!NILP (visibility)) */
	  /*   x_make_frame_visible (f); */
	}

      store_frame_param (f, Qvisibility, visibility);
    }

  /* For `no-focus-on-map' frames set alpha here.  */
  if (FRAME_NO_FOCUS_ON_MAP (f))
    x_default_parameter (f, parameters, Qalpha, Qnil,
			 "alpha", "Alpha", RES_TYPE_NUMBER);

  /* Initialize `default-minibuffer-frame' in case this is the first
     frame on this terminal.  */
  if (FRAME_HAS_MINIBUF_P (f)
      && (!FRAMEP (KVAR (kb, Vdefault_minibuffer_frame))
	  || !FRAME_LIVE_P (XFRAME (KVAR (kb, Vdefault_minibuffer_frame)))))
    kset_default_minibuffer_frame (kb, frame);

  /* All remaining specified parameters, which have not been "used"
     by x_get_arg and friends, now go in the misc. alist of the frame.  */
  for (tem = parameters; CONSP (tem); tem = XCDR (tem))
    if (CONSP (XCAR (tem)) && !NILP (XCAR (XCAR (tem))))
      fset_param_alist (f, Fcons (XCAR (tem), f->param_alist));

  /* Make sure windows on this frame appear in calls to next-window
     and similar functions.  */
  Vwindow_list = Qnil;

  return unbind_to (count, frame);
}

frame_parm_handler so_frame_parm_handlers[] =
{
  x_set_autoraise,
  x_set_autolower,
  0, //x_set_background_color,
  0, //x_set_border_color,
  0, //x_set_border_width,
  0, //x_set_cursor_color,
  0, //x_set_cursor_type,
  0, //x_set_font,
  0, //x_set_foreground_color,
  0, //x_set_icon_name,
  0, //x_set_icon_type,
  0, //x_set_internal_border_width,
  x_set_right_divider_width,
  x_set_bottom_divider_width,
  0, //x_set_menu_bar_lines,
  0, //x_set_mouse_color,
  0, //x_explicitly_set_name,
  x_set_scroll_bar_width,
  x_set_scroll_bar_height,
  0, //x_set_title,
  x_set_unsplittable,
  x_set_vertical_scroll_bars,
  x_set_horizontal_scroll_bars,
  x_set_visibility,
  0, //x_set_tool_bar_lines,
  0, /* x_set_scroll_bar_foreground, */
  0, /* x_set_scroll_bar_background, */
  x_set_screen_gamma,
  x_set_line_spacing,
  x_set_left_fringe,
  x_set_right_fringe,
  0, /* x_set_wait_for_wm, */
  x_set_fullscreen,
  x_set_font_backend,
  x_set_alpha,
  0, /* x_set_sticky */
  0, /* x_set_tool_bar_position */
  0, /* x_set_inhibit_double_buffering */
  0, //x_set_undecorated,
  0, //x_set_parent_frame,
  0, //x_set_skip_taskbar,
  0, //x_set_no_focus_on_map,
  0, //x_set_no_accept_focus,
  0, //x_set_z_group,
  0, /* x_set_override_redirect */
  x_set_no_special_glyphs,
};

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

  XSETFASTINT (Vwindow_system_version, 1);

  return dpyinfo;
}

DEFUN ("x-open-connection", Fx_open_connection, Sx_open_connection,
       1, 3, 0, doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display, Lisp_Object xrm_string, Lisp_Object must_succeed)
{
  char *xrm_option;
  struct so_display_info *dpyinfo;

  CHECK_STRING (display);

  /* Signal an error in order to encourage correct use from callers.
   * If we ever support multiple window systems in the same Emacs,
   * we'll need callers to be precise about what window system they
   * want.  */

  if (strcmp (SSDATA (display), "service_only_gui") != 0)
    error ("The name of the display in this Emacs must be \"service_only_gui\"");

  /* If initialization has already been done, return now to avoid
     overwriting critical parts of one_so_display_info.  */
  if (window_system_available (NULL))
    return Qnil;

  if (! NILP (xrm_string))
    CHECK_STRING (xrm_string);

  /* /\* Allow color mapping to be defined externally; first look in user's */
  /*    HOME directory, then in Emacs etc dir for a file called rgb.txt. *\/ */
  /* { */
  /*   Lisp_Object color_file; */

  /*   color_file = build_string ("~/rgb.txt"); */

  /*   if (NILP (Ffile_readable_p (color_file))) */
  /*     color_file = */
  /*       Fexpand_file_name (build_string ("rgb.txt"), */
  /*       		   Fsymbol_value (intern ("data-directory"))); */

  /*   Vso_color_map = Fx_load_color_file (color_file); */
  /* } */
  /* if (NILP (Vso_color_map)) */
  /*   Vso_color_map = so_default_color_map (); */

  /* /\* Merge in system logical colors.  *\/ */
  /* add_system_logical_colors_to_map (&Vso_color_map); */

  if (! NILP (xrm_string))
    xrm_option = SSDATA (xrm_string);
  else
    xrm_option = NULL;

  /* Use this general default value to start with.  */
  /* First remove .exe suffix from invocation-name - it looks ugly. */
  {
    char basename[ 255 ], *str;

    lispstpcpy (basename, Vinvocation_name);
    str = strrchr (basename, '.');
    if (str) *str = 0;
    Vinvocation_name = build_string (basename);
  }
  Vx_resource_name = Vinvocation_name;

  validate_x_resource_name ();

  /* This is what opens the connection and sets x_current_display.
     This also initializes many symbols, such as those used for input.  */
  dpyinfo = so_term_init (display, xrm_option, SSDATA (Vx_resource_name));

  if (dpyinfo == 0)
    {
      if (!NILP (must_succeed))
	fatal ("Cannot connect to server %s.\n",
	       SDATA (display));
      else
	error ("Cannot connect to server %s", SDATA (display));
    }

  XSETFASTINT (Vwindow_system_version, 1);
  return Qnil;
}

DEFUN ("x-close-connection", Fx_close_connection,
       Sx_close_connection, 1, 1, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object display)
{
  struct so_display_info *dpyinfo = check_x_display_info (display);

  if (dpyinfo->reference_count > 0)
    error ("Display still has frames on it");

  block_input ();
  x_destroy_all_bitmaps (dpyinfo);

  x_delete_display (dpyinfo);
  unblock_input ();

  return Qnil;
}

DEFUN ("x-display-list", Fx_display_list, Sx_display_list, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  Lisp_Object result = Qnil;
  struct so_display_info *wdi;

  for (wdi = x_display_list; wdi; wdi = wdi->next)
    result = Fcons (XCAR (wdi->name_list_element), result);

  return result;
}

DEFUN ("x-synchronize", Fx_synchronize, Sx_synchronize, 1, 2, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (Lisp_Object on, Lisp_Object display)
{
  return Qnil;
}

DEFUN ("set-message-beep", Fset_message_beep, Sset_message_beep, 1, 1, 0,
       doc: /* Set the sound generated when the bell is rung.
SOUND is `asterisk', `exclamation', `hand', `question', `ok', or `silent'
to use the corresponding system sound for the bell.  The `silent' sound
prevents Emacs from making any sound at all.
SOUND is nil to use the normal beep.  */)
  (Lisp_Object sound)
{
  CHECK_SYMBOL (sound);

  return sound;
}

void x_change_tool_bar_height (struct frame *f, int h) {
}

Lisp_Object
x_get_focus_frame (struct frame *frame)
{
  struct so_display_info *dpyinfo = FRAME_DISPLAY_INFO (frame);
  Lisp_Object xfocus;
  if (! dpyinfo->so_focus_frame)
    return Qnil;

  XSETFRAME (xfocus, dpyinfo->so_focus_frame);
  return xfocus;
}

void syms_of_sofns (void) {
  syms_of_sofont();

  defsubr (&Sx_create_frame);
  defsubr (&Sx_open_connection);
  defsubr (&Sx_close_connection);
  defsubr (&Sx_display_list);
  defsubr (&Sx_synchronize);
  defsubr (&Sset_message_beep);
}
