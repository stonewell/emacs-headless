#include <config.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "lisp.h"
#include "service_only_term.h"
#include "service_only_font.h"
#include "frame.h"
#include "coding.h"	/* for ENCODE_SYSTEM, DECODE_SYSTEM */

static const char *const sofont_booleans [] = {
  NULL,
};

static const char *const sofont_non_booleans [] = {
  ":script",
  ":antialias",
  ":style",
  NULL,
};

static void
sofont_filter_properties (Lisp_Object font, Lisp_Object alist)
{
  font_filter_properties (font, alist, sofont_booleans, sofont_non_booleans);
}

static
int
sofont_draw (struct glyph_string *s, int from, int to,
	      int x, int y, bool with_background)
{
  fprintf(stderr, "sofont draw:%ls\n", s->char2b);
  return 0;
}

static
Lisp_Object
sofont_get_cache (struct frame *f)
{
  struct so_display_info *dpyinfo = FRAME_DISPLAY_INFO (f);

  return (dpyinfo->name_list_element);
}

static Lisp_Object
sofont_list (struct frame *f, Lisp_Object font_spec) {
  return Qnil;
}

static
Lisp_Object
intern_font_name (const char * string)
{
  Lisp_Object str = DECODE_SYSTEM (build_string (string));
  ptrdiff_t len = SCHARS (str);
  Lisp_Object obarray = check_obarray (Vobarray);
  Lisp_Object tem = oblookup (obarray, SSDATA (str), len, len);
  /* This code is similar to intern function from lread.c.  */
  return SYMBOLP (tem) ? tem : intern_driver (str, obarray, tem);
}

static Lisp_Object
sofont_match (struct frame *f, Lisp_Object font_spec) {
  Lisp_Object entity;

  entity = font_make_entity ();

  ASET (entity, FONT_TYPE_INDEX, Qsofont);
  ASET (entity, FONT_REGISTRY_INDEX, Qnil);
  ASET (entity, FONT_OBJLIST_INDEX, Qnil);

  ASET (entity, FONT_FOUNDRY_INDEX, Qunknown);

  /* Save the generic family in the extra info, as it is likely to be
     useful to users looking for a close match.  */
  /* generic_type = physical_font->ntmTm.tmPitchAndFamily & 0xF0; */
  /* if (generic_type == FF_DECORATIVE) */
  /*   tem = Qdecorative; */
  /* else if (generic_type == FF_MODERN) */
  /*   tem = Qmono; */
  /* else if (generic_type == FF_ROMAN) */
  /*   tem = Qserif; */
  /* else if (generic_type == FF_SCRIPT) */
  /*   tem = Qscript; */
  /* else if (generic_type == FF_SWISS) */
  /*   tem = Qsans; */
  /* else */
  /*   tem = Qnil; */

  ASET (entity, FONT_ADSTYLE_INDEX, Qsans);
  ASET (entity, FONT_SPACING_INDEX, make_fixnum (FONT_SPACING_CHARCELL));
  ASET (entity, FONT_FAMILY_INDEX, intern_font_name ("Source Code Pro"));

  FONT_SET_STYLE (entity, FONT_WEIGHT_INDEX,
		  make_fixnum (80));
  FONT_SET_STYLE (entity, FONT_SLANT_INDEX,
		  make_fixnum (100));
  FONT_SET_STYLE (entity, FONT_WIDTH_INDEX, make_fixnum (100));

  ASET (entity, FONT_SIZE_INDEX, make_fixnum (0));

  font_put_extra (entity, QCformat, intern ("truetype"));

  return entity;
}

static Lisp_Object
sofont_open (struct frame *f, Lisp_Object font_entity, int pixel_size)
{
  pixel_size = 16;
  Lisp_Object font_object
    = font_make_object (VECSIZE (struct sofont_info),
                        font_entity, pixel_size);
  struct sofont_info *so_font
    = (struct sofont_info *) XFONT_OBJECT (font_object);
  struct font * font = (struct font *)so_font;

  ASET (font_object, FONT_TYPE_INDEX, Qsofont);

  so_font->glyph_idx = 0;

  font->space_width = font->average_width = 10;
  font->vertical_centering = 0;
  font->baseline_offset = 0;
  font->relative_compose = 0;
  font->default_ascent = pixel_size;
  font->pixel_size = pixel_size;
  font->driver = &sofont_driver;
  font->encoding_charset = -1;
  font->repertory_charset = -1;
  font->min_width = font->space_width;
  font->ascent = pixel_size;
  font->descent = 1;
  font->height = pixel_size + 2;

  font->props[FONT_NAME_INDEX] = Ffont_xlfd_name (font_object, Qnil);
  return font_object;
}

static unsigned
sofont_encode_char (struct font *font, int c)
{
    return c;
}

static
void
sofont_text_extents (struct font *font, unsigned *code,
		      int nglyphs, struct font_metrics *metrics)
{
  memset (metrics, 0, sizeof (struct font_metrics));
  metrics->width = 10 * nglyphs;
  metrics->lbearing = 0;
  metrics->rbearing = 10 * nglyphs;
  metrics->ascent = font->ascent;
  metrics->descent = font->descent;
}

static
void
sofont_close (struct font *font) {
}


struct font_driver sofont_driver =
  {
    LISPSYM_INITIALLY (Qsofont),
    false, /* case insensitive */
    sofont_get_cache,
    sofont_list,
    sofont_match,
    NULL, // sofont_list_family,
    NULL, /* free_entity */
    sofont_open,
    sofont_close,
    NULL, /* prepare_face */
    NULL, /* done_face */
    NULL, // sofont_has_char,
    sofont_encode_char,
    sofont_text_extents,
    sofont_draw,
    NULL, /* get_bitmap */
    NULL, /* free_bitmap */
    NULL, /* anchor_point */
    NULL, /* otf_capability */
    NULL, /* otf_drive */
    NULL, /* start_for_frame */
    NULL, /* end_for_frame */
    NULL, /* shape */
    NULL, /* check */
    NULL, /* get_variation_glyphs */
    NULL, // sofont_filter_properties,
    NULL, /* cached_font_ok */
  };


/* Initialize state that does not change between invocations. This is only
   called when Emacs is dumped.  */
void
syms_of_sofont (void)
{
  DEFSYM (Qsofont, "sofont");
  DEFSYM (QCformat, ":format");

  /* Generic font families.  */
  DEFSYM (Qmonospace, "monospace");
  DEFSYM (Qserif, "serif");
  DEFSYM (Qsansserif, "sansserif");
  DEFSYM (Qscript, "script");
  DEFSYM (Qdecorative, "decorative");
  /* Aliases.  */
  DEFSYM (Qsans_serif, "sans_serif");
  DEFSYM (Qsans, "sans");
  DEFSYM (Qmono, "mono");

  /* Fake foundries.  */
  DEFSYM (Qraster, "raster");
  DEFSYM (Qoutline, "outline");
  DEFSYM (Qunknown, "unknown");

  /* Antialiasing.  */
  DEFSYM (Qstandard, "standard");
  DEFSYM (Qsubpixel, "subpixel");
  DEFSYM (Qnatural, "natural");

  /* Languages  */
  DEFSYM (Qzh, "zh");

  /* Scripts  */
  DEFSYM (Qlatin, "latin");
  DEFSYM (Qgreek, "greek");
  DEFSYM (Qcoptic, "coptic");
  DEFSYM (Qcyrillic, "cyrillic");
  DEFSYM (Qarmenian, "armenian");
  DEFSYM (Qhebrew, "hebrew");
  DEFSYM (Qvai, "vai");
  DEFSYM (Qarabic, "arabic");
  DEFSYM (Qsyriac, "syriac");
  DEFSYM (Qnko, "nko");
  DEFSYM (Qthaana, "thaana");
  DEFSYM (Qdevanagari, "devanagari");
  DEFSYM (Qbengali, "bengali");
  DEFSYM (Qgurmukhi, "gurmukhi");
  DEFSYM (Qgujarati, "gujarati");
  DEFSYM (Qoriya, "oriya");
  DEFSYM (Qtamil, "tamil");
  DEFSYM (Qtelugu, "telugu");
  DEFSYM (Qkannada, "kannada");
  DEFSYM (Qmalayalam, "malayalam");
  DEFSYM (Qsinhala, "sinhala");
  DEFSYM (Qthai, "thai");
  DEFSYM (Qlao, "lao");
  DEFSYM (Qtibetan, "tibetan");
  DEFSYM (Qmyanmar, "myanmar");
  DEFSYM (Qgeorgian, "georgian");
  DEFSYM (Qhangul, "hangul");
  DEFSYM (Qethiopic, "ethiopic");
  DEFSYM (Qcherokee, "cherokee");
  DEFSYM (Qcanadian_aboriginal, "canadian-aboriginal");
  DEFSYM (Qogham, "ogham");
  DEFSYM (Qrunic, "runic");
  DEFSYM (Qkhmer, "khmer");
  DEFSYM (Qmongolian, "mongolian");
  DEFSYM (Qbraille, "braille");
  DEFSYM (Qhan, "han");
  DEFSYM (Qideographic_description, "ideographic-description");
  DEFSYM (Qcjk_misc, "cjk-misc");
  DEFSYM (Qkana, "kana");
  DEFSYM (Qbopomofo, "bopomofo");
  DEFSYM (Qkanbun, "kanbun");
  DEFSYM (Qyi, "yi");
  DEFSYM (Qbyzantine_musical_symbol, "byzantine-musical-symbol");
  DEFSYM (Qmusical_symbol, "musical-symbol");
  DEFSYM (Qmathematical_bold, "mathematical-bold");
  DEFSYM (Qmathematical_italic, "mathematical-italic");
  DEFSYM (Qmathematical_bold_italic, "mathematical-bold-italic");
  DEFSYM (Qmathematical_script, "mathematical-script");
  DEFSYM (Qmathematical_bold_script, "mathematical-bold-script");
  DEFSYM (Qmathematical_fraktur, "mathematical-fraktur");
  DEFSYM (Qmathematical_double_struck, "mathematical-double-struck");
  DEFSYM (Qmathematical_bold_fraktur, "mathematical-bold-fraktur");
  DEFSYM (Qmathematical_sans_serif, "mathematical-sans-serif");
  DEFSYM (Qmathematical_sans_serif_bold, "mathematical-sans-serif-bold");
  DEFSYM (Qmathematical_sans_serif_italic, "mathematical-sans-serif-italic");
  DEFSYM (Qmathematical_sans_serif_bold_italic, "mathematical-sans-serif-bold-italic");
  DEFSYM (Qmathematical_monospace, "mathematical-monospace");
  DEFSYM (Qcham, "cham");
  DEFSYM (Qphonetic, "phonetic");
  DEFSYM (Qbalinese, "balinese");
  DEFSYM (Qbuginese, "buginese");
  DEFSYM (Qbuhid, "buhid");
  DEFSYM (Qcuneiform, "cuneiform");
  DEFSYM (Qcypriot, "cypriot");
  DEFSYM (Qdeseret, "deseret");
  DEFSYM (Qglagolitic, "glagolitic");
  DEFSYM (Qgothic, "gothic");
  DEFSYM (Qhanunoo, "hanunoo");
  DEFSYM (Qkharoshthi, "kharoshthi");
  DEFSYM (Qlimbu, "limbu");
  DEFSYM (Qlinear_b, "linear_b");
  DEFSYM (Qold_italic, "old_italic");
  DEFSYM (Qold_persian, "old_persian");
  DEFSYM (Qosmanya, "osmanya");
  DEFSYM (Qphags_pa, "phags-pa");
  DEFSYM (Qphoenician, "phoenician");
  DEFSYM (Qshavian, "shavian");
  DEFSYM (Qsyloti_nagri, "syloti_nagri");
  DEFSYM (Qtagalog, "tagalog");
  DEFSYM (Qtagbanwa, "tagbanwa");
  DEFSYM (Qtai_le, "tai_le");
  DEFSYM (Qtifinagh, "tifinagh");
  DEFSYM (Qugaritic, "ugaritic");
  DEFSYM (Qlycian, "lycian");
  DEFSYM (Qcarian, "carian");
  DEFSYM (Qlydian, "lydian");
  DEFSYM (Qdomino_tile, "domino-tile");
  DEFSYM (Qmahjong_tile, "mahjong-tile");
  DEFSYM (Qtai_xuan_jing_symbol, "tai-xuan-jing-symbol");
  DEFSYM (Qcounting_rod_numeral, "counting-rod-numeral");
  DEFSYM (Qancient_symbol, "ancient-symbol");
  DEFSYM (Qphaistos_disc, "phaistos-disc");
  DEFSYM (Qancient_greek_number, "ancient-greek-number");
  DEFSYM (Qsundanese, "sundanese");
  DEFSYM (Qlepcha, "lepcha");
  DEFSYM (Qol_chiki, "ol-chiki");
  DEFSYM (Qsaurashtra, "saurashtra");
  DEFSYM (Qkayah_li, "kayah-li");
  DEFSYM (Qrejang, "rejang");
}
