#ifndef __SERVICE_ONLY_TERM_H__
#define __SERVICE_ONLY_TERM_H__

#include "service_only_gui.h"
#include "frame.h"
#include "atimer.h"

/* Stack alignment stuff.  Every CALLBACK and thread function should
   have the ALIGN_STACK attribute if it manipulates Lisp objects,
   because Windows x86 32-bit ABI only guarantees 4-byte stack
   alignment, and that is what we will get when a Windows function
   calls us.  The ALIGN_STACK attribute forces GCC to emit a preamble
   code to re-align the stack at function entry.  Further details
   about this can be found in
   http://www.peterstock.co.uk/games/mingw_sse/.  */
#ifdef __GNUC__
# if USE_STACK_LISP_OBJECTS && !defined _WIN64 && !defined __x86_64__	\
    && __GNUC__ + (__GNUC_MINOR__ > 1) >= 5
#  define ALIGN_STACK __attribute__((force_align_arg_pointer))
# else
#  define ALIGN_STACK
# endif	 /* USE_STACK_LISP_OBJECTS */
#endif

struct x_output
{
    /* Keep track of focus.  May be EXPLICIT if we received a FocusIn for this
       frame, or IMPLICIT if we received an EnterNotify.
       FocusOut and LeaveNotify clears EXPLICIT/IMPLICIT. */
    int focus_state;
};

struct so_output
{
    /* Placeholder for things accessed through output_data.x.  */
    struct x_output x_compatible;

    /* Menubar "widget" handle.  */
    HANDLE menubar_widget;

    /* Original palette (used to deselect real palette after drawing) */
    HANDLE old_palette;

    /* Here are the Graphics Contexts for the default font.  */
    XGCValues *cursor_gc;				/* cursor drawing */

    /* The window used for this frame.
       May be zero while the frame object is being created
       and the window has not yet been created.  */
    Window window_desc;

    /* The window that is the parent of this window.
       Usually this is a window that was made by the window manager,
       but it can be the root window, and it can be explicitly specified
       (see the explicit_parent field, below).  */
    Window parent_desc;

    /* Default ASCII font of this frame. */
    struct font *font;

    /* The baseline offset of the default ASCII font.  */
    int baseline_offset;

    /* If a fontset is specified for this frame instead of font, this
       value contains an ID of the fontset, else -1.  */
    int fontset;

    /* Pixel values used for various purposes.
       border_pixel may be -1 meaning use a gray tile.  */
    COLORREF cursor_pixel;
    COLORREF border_pixel;
    COLORREF mouse_pixel;
    COLORREF cursor_foreground_pixel;

    /* Foreground color for scroll bars.  A value of -1 means use the
       default (black for non-toolkit scroll bars).  */
    COLORREF scroll_bar_foreground_pixel;

    /* Background color for scroll bars.  A value of -1 means use the
       default (background color of the frame for non-toolkit scroll
       bars).  */
    COLORREF scroll_bar_background_pixel;

    /* Descriptor for the cursor in use for this window.  */
    Cursor text_cursor;
    Cursor nontext_cursor;
    Cursor modeline_cursor;
    Cursor hand_cursor;
    Cursor hourglass_cursor;
    Cursor horizontal_drag_cursor;
    Cursor vertical_drag_cursor;
    Cursor left_edge_cursor;
    Cursor top_left_corner_cursor;
    Cursor top_edge_cursor;
    Cursor top_right_corner_cursor;
    Cursor right_edge_cursor;
    Cursor bottom_right_corner_cursor;
    Cursor bottom_edge_cursor;
    Cursor bottom_left_corner_cursor;

    /* Non-zero means hourglass cursor is currently displayed.  */
    unsigned hourglass_p : 1;

    /* Non-hourglass cursor that is currently active.  */
    Cursor current_cursor;

    unsigned int dwStyle;

    /* This is the Emacs structure for the display this frame is on.  */
    /* struct so_display_info *display_info; */

    /* Nonzero means our parent is another application's window
       and was explicitly specified.  */
    unsigned explicit_parent : 1;

    /* Nonzero means tried already to make this frame visible.  */
    unsigned asked_for_visible : 1;

    /* Nonzero means menubar is currently active.  */
    unsigned menubar_active : 1;

    /* Relief GCs, colors etc.  */
    struct relief
    {
        XGCValues *gc;
        unsigned long pixel;
    }
        black_relief, white_relief;

    /* The background for which the above relief GCs were set up.
       They are changed only when a different background is involved.  */
    unsigned long relief_background;

    /* Frame geometry and full-screen mode before it was resized by
       specifying the 'fullscreen' frame parameter.  Used to restore the
       geometry when 'fullscreen' is reset to nil.  */
    struct WINDOWPLACEMENT {
    } normal_placement;
    int prev_fsmode;
};

extern struct so_output soterm_display;

struct so_bitmap_record
{
    Pixmap pixmap;
    char *file;
    HANDLE hinst; /* Used to load the file */
    int refcount;
    /* Record some info about this pixmap.  */
    int height, width, depth;
};

typedef struct so_bitmap_record Bitmap_Record;

struct so_palette_entry {
    struct so_palette_entry * next;
    struct {} entry;
};

struct so_display_info
{
    /* Chain of all so_display_info structures.  */
    struct so_display_info *next;

    /* The generic display parameters corresponding to this so display.  */
    struct terminal *terminal;

    /* This is a cons cell of the form (NAME . FONT-LIST-CACHE).  */
    Lisp_Object name_list_element;

    /* Number of frames that are on this display.  */
    int reference_count;

    /* Dots per inch of the screen.  */
    double resx, resy;

    /* Number of planes on this screen.  */
    int n_planes;

    /* Number of bits per pixel on this screen.  */
    int n_cbits;

    /* Mask of things that cause the mouse to be grabbed.  */
    int grabbed;

    /* Emacs bitmap-id of the default icon bitmap for this frame.
       Or -1 if none has been allocated yet.  */
    ptrdiff_t icon_bitmap_id;

    /* The root window of this screen.  */
    Window root_window;

    /* The cursor to use for vertical scroll bars.  */
    Cursor vertical_scroll_bar_cursor;

    /* The cursor to use for horizontal scroll bars.  */
    Cursor horizontal_scroll_bar_cursor;

    /* Resource data base */
    XrmDatabase xrdb;

    /* color palette information.  */
    int has_palette;
    struct so_palette_entry * color_list;
    unsigned num_colors;
    HANDLE palette;

    /* deferred action flags checked when starting frame update.  */
    int regen_palette;

    /* Keystroke that has been faked by Emacs and will be ignored when
       received; value is reset after key is received.  */
    int faked_key;

    /* Minimum width over all characters in all fonts in font_table.  */
    int smallest_char_width;

    /* Minimum font height over all fonts in font_table.  */
    int smallest_font_height;

    /* Reusable Graphics Context for drawing a cursor in a non-default face. */
    XGCValues *scratch_cursor_gc;

    /* Information about the range of text currently shown in
       mouse-face.  */
    Mouse_HLInfo mouse_highlight;

    char *so_id_name;

    /* The number of fonts actually stored in so_font_table.
       font_table[n] is used and valid if 0 <= n < n_fonts. 0 <=
       n_fonts <= font_table_size. and font_table[i].name != 0. */
    int n_fonts;

    /* Pointer to bitmap records.  */
    struct so_bitmap_record *bitmaps;

    /* Allocated size of bitmaps field.  */
    ptrdiff_t bitmaps_size;

    /* Last used bitmap index.  */
    ptrdiff_t bitmaps_last;

    /* The frame (if any) which has the window that has keyboard focus.
       Zero if none.  This is examined by Ffocus_frame in sofns.c.  Note
       that a mere EnterNotify event can set this; if you need to know the
       last frame specified in a FocusIn or FocusOut event, use
       so_focus_event_frame.  */
    struct frame *so_focus_frame;

    /* The last frame mentioned in a FocusIn or FocusOut event.  This is
       separate from so_focus_frame, because whether or not LeaveNotify
       events cause us to lose focus depends on whether or not we have
       received a FocusIn event for it.  */
    struct frame *so_focus_event_frame;

    /* The frame which currently has the visual highlight, and should get
       keyboard input (other sorts of input have the frame encoded in the
       event).  It points to the focus frame's selected window's
       frame.  It differs from so_focus_frame when we're using a global
       minibuffer.  */
    struct frame *x_highlight_frame;

    /* The frame waiting to be auto-raised in so_read_socket.  */
    struct frame *so_pending_autoraise_frame;

    /* The frame where the mouse was last time we reported a mouse event.  */
    struct frame *last_mouse_frame;

    /* The frame where the mouse was last time we reported a mouse motion.  */
    struct frame *last_mouse_motion_frame;

    /* The frame where the mouse was last time we reported a mouse position.  */
    struct frame *last_mouse_glyph_frame;

    /* Position where the mouse was last time we reported a motion.
       This is a position on last_mouse_motion_frame.  */
    int last_mouse_motion_x;
    int last_mouse_motion_y;

    /* Where the mouse was last time we reported a mouse position.
       This is a rectangle on last_mouse_glyph_frame.  */
    RECT last_mouse_glyph;

    /* The scroll bar in which the last motion event occurred.  */
    struct scroll_bar *last_mouse_scroll_bar;

    /* Mouse position on the scroll bar above.
       FIXME: shouldn't it be a member of struct scroll_bar?  */
    int last_mouse_scroll_bar_pos;

    /* Time of last mouse movement.  */
    Time last_mouse_movement_time;

    /* Value returned by last call of ShowCursor.  */
    int cursor_display_counter;
};

extern struct so_display_info *x_display_list;
extern struct so_display_info one_so_display_info;

/* Return the X output data for frame F.  */
#define FRAME_X_OUTPUT(f) ((f)->output_data.so)

/* Return the window associated with the frame F.  */
#define FRAME_SO_WINDOW(f) ((f)->output_data.so->window_desc)
#define FRAME_X_WINDOW(f) FRAME_SO_WINDOW (f)

#define FRAME_FONT(f) ((f)->output_data.so->font)
#define FRAME_FONTSET(f) ((f)->output_data.so->fontset)
#define FRAME_BASELINE_OFFSET(f) ((f)->output_data.so->baseline_offset)

#define FRAME_DISPLAY_INFO(f) ((void)(f), (&one_so_display_info))

/* This is the `Display *' which frame F is on.  */
#define FRAME_X_DISPLAY(f) (0)
#define FRAME_X_DRAWABLE(f) (0)
#define FRAME_X_SCREEN(f) (0)
#define DefaultDepthOfScreen(s) (0)

extern void x_delete_display (struct so_display_info *dpyinfo);
extern void x_clear_under_internal_border (struct frame *f);
extern void x_query_color (struct frame *, XColor *);
extern GC
x_create_gc (struct frame *f,
	     unsigned long mask,
	     XGCValues *xgcv);
extern void
x_free_gc (struct frame *f, GC gc);

extern unsigned long
so_get_pixel(XImagePtr ximage, int x, int y);
extern void
so_free_pixmap(XImagePtr ximage);

#define GET_PIXEL(ximg, x, y) so_get_pixel(ximg, x, y)
#define NO_PIXMAP 0

#define PIX_MASK_RETAIN	0
#define PIX_MASK_DRAW	1

#define GCGraphicsExposures 0

extern int so_defined_color (struct frame *f, const char *color,
                             XColor *color_def, bool alloc_p);
#define x_defined_color so_defined_color

extern
Pixmap XCreatePixmapFromBitmapData(Display *display,
                                   void * d,
                                   char *data, unsigned int width,
                                   unsigned int height,
                                   unsigned long fg,
                                   unsigned long bg, unsigned int depth);
#endif // __SERVICE_ONLY_TERM_H__
