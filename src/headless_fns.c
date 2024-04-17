#include <config.h>
#include <math.h>

#include "lisp.h"
#include "headless_term.h"
#include "blockinput.h"
#include "keyboard.h"
#include "buffer.h"
#include "headless_gui.h"
#include "pdumper.h"

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

Display_Info *
check_x_display_info (Lisp_Object object)
{
  return check_headless_display_info (object);
}

DEFUN ("x-hide-tip", Fx_hide_tip, Sx_hide_tip, 0, 0, 0,
       doc: /* SKIP: real doc in xfns.c.  */)
  (void)
{
  return Qnil;
}
