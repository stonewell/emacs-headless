#include <config.h>

#include "lisp.h"
#include "dispextern.h"
#include "composite.h"
#include "blockinput.h"
#include "charset.h"
#include "frame.h"
#include "window.h"
#include "fontset.h"
#include "headless_term.h"
#include "character.h"
#include "coding.h"
#include "font.h"
#include "termchar.h"
#include "pdumper.h"

static Lisp_Object font_cache;

static int
headlessfont_draw (struct glyph_string *s, int from, int to,
		  int x, int y, bool with_background)
{
  return 0;
}

static Lisp_Object
headlessfont_get_cache (struct frame *frame)
{
  return font_cache;
}

static Lisp_Object
headlessfont_list (struct frame *f, Lisp_Object font_spec)
{
  error("list font");
  return Qnil;
}

static Lisp_Object
headlessfont_match (struct frame *f, Lisp_Object font_spec)
{
  error("match font");
  return Qnil;
}

static Lisp_Object
headlessfont_open_font (struct frame *f, Lisp_Object font_entity,
		                                    int pixel_size)
{
  error("open font");
  return Qnil;
}

static int
headlessfont_has_char (Lisp_Object font, int c)
{
  return 0;
}

static void
headlessfont_close_font (struct font *font)
{
}

static unsigned
headlessfont_encode_char (struct font *font, int c)
{
  return (unsigned)c;
}

static void
headlessfont_text_extents (struct font *font, const unsigned int *code,
			  int nglyphs, struct font_metrics *metrics)
{
}

static Lisp_Object
headlessfont_list_family (struct frame *f)
{
  error("list font family");
  return Qnil;
}

struct font_driver headlessfont_driver =
  {
    .type = LISPSYM_INITIALLY (Qheadless),
    .case_sensitive = true,
    .get_cache = headlessfont_get_cache,
    .list = headlessfont_list,
    .match = headlessfont_match,
    .draw = headlessfont_draw,
    .open_font = headlessfont_open_font,
    .close_font = headlessfont_close_font,
    .has_char = headlessfont_has_char,
    .encode_char = headlessfont_encode_char,
    .text_extents = headlessfont_text_extents,
    .list_family = headlessfont_list_family,
  };

void
syms_of_headlessfont (void)
{
  DEFSYM (Qfontsize, "fontsize");

  font_cache = list (Qnil);
  staticpro (&font_cache);
}
