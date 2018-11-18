#include <config.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "lisp.h"
#include "service_only_term.h"
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
  return 0;
}

static
Lisp_Object
sofont_get_cache (struct frame *f)
{
  struct so_display_info *dpyinfo = FRAME_DISPLAY_INFO (f);

  return (dpyinfo->name_list_element);
}


struct font_driver sofont_driver =
  {
    LISPSYM_INITIALLY (Qsofont),
    false, /* case insensitive */
    sofont_get_cache,
    NULL, // sofont_list,
    NULL, // sofont_match,
    NULL, // sofont_list_family,
    NULL, /* free_entity */
    NULL, // sofont_open,
    NULL, // sofont_close,
    NULL, /* prepare_face */
    NULL, /* done_face */
    NULL, // sofont_has_char,
    NULL, // sofont_encode_char,
    NULL, // sofont_text_extents,
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
