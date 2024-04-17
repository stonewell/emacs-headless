#include <config.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>

#include "lisp.h"
#include "headless_term.h"
#include "keyboard.h"
#include "blockinput.h"
#include "buffer.h"
#include "window.h"
#include "textconv.h"
#include "coding.h"
#include "pdumper.h"

#include "headless_service.h"

struct headless_display_info *x_display_list;

char *
get_keysym_name (int keysym)
{
  static char buffer[64]={0};

  return buffer;
}

unsigned long
headless_get_pixel (struct headless_image *ximg, int x, int y)
{
  return 0;
}

void
headless_put_pixel (struct headless_image *ximg, int x, int y,
		   unsigned long pixel)
{
}

void
syms_of_headlessterm (void)
{
  DEFVAR_BOOL ("x-use-underline-position-properties",
	       x_use_underline_position_properties,
     doc: /* SKIP: real doc in xterm.c.  */);
  x_use_underline_position_properties = 1;

  DEFVAR_BOOL ("x-underline-at-descent-line",
	       x_underline_at_descent_line,
     doc: /* SKIP: real doc in xterm.c.  */);
  x_underline_at_descent_line = 0;

  DEFVAR_LISP ("x-toolkit-scroll-bars", Vx_toolkit_scroll_bars,
     doc: /* SKIP: real doc in xterm.c.  */);
  Vx_toolkit_scroll_bars = Qt;

  DEFSYM (Qshift, "shift");
  DEFSYM (Qcontrol, "control");
  DEFSYM (Qoption, "option");
  DEFSYM (Qcommand, "command");

  DEFSYM (Qdata_directory, "data-directory");

  DEFSYM (Qx_use_underline_position_properties,
	  "x-use-underline-position-properties");

  DEFSYM (Qx_underline_at_descent_line, "x-underline-at-descent-line");

  Fprovide (Qheadless, Qnil);
}

static void
headless_cursor_to (struct frame *f, int vpos, int hpos);
static void
headless_raw_cursor_to (struct frame *f, int row, int col);
static void
headless_clear_to_end (struct frame *f);
static void
headless_clear_frame (struct frame *f);
static void
headless_clear_end_of_line (struct frame *f, int first_unused_hpos);
static void
headless_ins_del_lines (struct frame *f, int vpos, int n);
static void
headless_insert_glyphs (struct frame *f, struct glyph *start, int len);
static void
headless_write_glyphs (struct frame *f, struct glyph *string, int len);
static void
headless_delete_glyphs (struct frame *f, int n);
static void
headless_ring_bell (struct frame *f);
static void
headless_reset_terminal_modes (struct terminal *terminal);
static void
headless_set_terminal_modes (struct terminal *terminal);
static void
headless_update_end (struct frame *f);
static Lisp_Object
headless_menu_show (struct frame *f, int x, int y, int menuflags,
                        Lisp_Object title, const char **error_name);
static void
headless_set_terminal_window (struct frame *f, int size);
static bool
headless_defined_color (struct frame *f, const char *color_name,
                            Emacs_Color *color_def, bool alloc, bool _makeIndex);
static int
headless_read_avail_input (struct terminal *terminal,
                               struct input_event *hold_quit);
static void
headless_delete_frame (struct frame *f);
static void
headless_delete_terminal (struct terminal *terminal);

static void
save_hooks(struct terminal *terminal)
{
  terminal->headless_terminal->cursor_to_hook = terminal->cursor_to_hook;
  terminal->headless_terminal->raw_cursor_to_hook = terminal->raw_cursor_to_hook;
  terminal->headless_terminal->clear_to_end_hook = terminal->clear_to_end_hook;
  terminal->headless_terminal->clear_frame_hook = terminal->clear_frame_hook;
  terminal->headless_terminal->clear_end_of_line_hook = terminal->clear_end_of_line_hook;
  terminal->headless_terminal->ins_del_lines_hook = terminal->ins_del_lines_hook;
  terminal->headless_terminal->insert_glyphs_hook = terminal->insert_glyphs_hook;
  terminal->headless_terminal->write_glyphs_hook = terminal->write_glyphs_hook;
  terminal->headless_terminal->delete_glyphs_hook = terminal->delete_glyphs_hook;
  terminal->headless_terminal->ring_bell_hook = terminal->ring_bell_hook;
  terminal->headless_terminal->reset_terminal_modes_hook = terminal->reset_terminal_modes_hook;
  terminal->headless_terminal->set_terminal_modes_hook = terminal->set_terminal_modes_hook;
  terminal->headless_terminal->update_end_hook = terminal->update_end_hook;
  terminal->headless_terminal->menu_show_hook = terminal->menu_show_hook;
  terminal->headless_terminal->set_terminal_window_hook = terminal->set_terminal_window_hook;
  terminal->headless_terminal->defined_color_hook = terminal->defined_color_hook;
  terminal->headless_terminal->read_socket_hook = terminal->read_socket_hook;
  terminal->headless_terminal->delete_frame_hook = terminal->delete_frame_hook;
  terminal->headless_terminal->delete_terminal_hook = terminal->delete_terminal_hook;
}

void headless_set_hooks (struct terminal *terminal)
{
  save_hooks(terminal);

  terminal->cursor_to_hook = &headless_cursor_to;
  terminal->raw_cursor_to_hook = &headless_raw_cursor_to;
  terminal->clear_to_end_hook = &headless_clear_to_end;
  terminal->clear_frame_hook = &headless_clear_frame;
  terminal->clear_end_of_line_hook = &headless_clear_end_of_line;
  terminal->ins_del_lines_hook = &headless_ins_del_lines;
  terminal->insert_glyphs_hook = &headless_insert_glyphs;
  terminal->write_glyphs_hook = &headless_write_glyphs;
  terminal->delete_glyphs_hook = &headless_delete_glyphs;
  terminal->ring_bell_hook = &headless_ring_bell;
  terminal->reset_terminal_modes_hook = &headless_reset_terminal_modes;
  terminal->set_terminal_modes_hook = &headless_set_terminal_modes;
  terminal->update_end_hook = &headless_update_end;
  terminal->menu_show_hook = &headless_menu_show;
  terminal->set_terminal_window_hook = &headless_set_terminal_window;
  terminal->defined_color_hook = &headless_defined_color; /* xfaces.c */
  terminal->read_socket_hook = &headless_read_avail_input; /* keyboard.c */
  terminal->delete_frame_hook = &headless_delete_frame;
  terminal->delete_terminal_hook = &headless_delete_terminal;
}

void headless_cursor_to (struct frame *f, int vpos, int hpos)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_cursor_to(terminal->id, vpos, hpos);

  if (terminal->headless_terminal->cursor_to_hook) {
    terminal->headless_terminal->cursor_to_hook(f, vpos, hpos);
  }
}

void headless_raw_cursor_to (struct frame *f, int row, int col)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_raw_cursor_to(terminal->id, row, col);

  if (terminal->headless_terminal->raw_cursor_to_hook) {
    terminal->headless_terminal->raw_cursor_to_hook(f, row, col);
  }
}

void headless_clear_to_end (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_clear_to_end(terminal->id);

  if (terminal->headless_terminal->clear_to_end_hook) {
    terminal->headless_terminal->clear_to_end_hook(f);
  }
}

void headless_clear_frame (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_clear_frame(terminal->id);

  if (terminal->headless_terminal->clear_frame_hook) {
    terminal->headless_terminal->clear_frame_hook(f);
  }
}

void headless_clear_end_of_line (struct frame *f, int first_unused_hpos)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_clear_end_of_line(terminal->id, first_unused_hpos);

  if (terminal->headless_terminal->clear_end_of_line_hook) {
    terminal->headless_terminal->clear_end_of_line_hook(f, first_unused_hpos);
  }
}

void headless_ins_del_lines (struct frame *f, int vpos, int n)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_ins_del_lines(terminal->id, vpos, n);

  if (terminal->headless_terminal->ins_del_lines_hook) {
    terminal->headless_terminal->ins_del_lines_hook(f, vpos, n);
  }
}

void headless_insert_glyphs (struct frame *f, struct glyph *start, int len)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_insert_glyphs(terminal->id, L"", len);

  if (terminal->headless_terminal->insert_glyphs_hook) {
    terminal->headless_terminal->insert_glyphs_hook(f, start, len);
  }
}

void headless_write_glyphs (struct frame *f, struct glyph *string, int len)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_write_glyphs(terminal->id, L"", len);

  if (terminal->headless_terminal->write_glyphs_hook) {
    terminal->headless_terminal->write_glyphs_hook(f, string, len);
  }
}

void headless_delete_glyphs (struct frame *f, int n)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_delete_glyphs(terminal->id, n);

  if (terminal->headless_terminal->delete_glyphs_hook) {
    terminal->headless_terminal->delete_glyphs_hook(f, n);
  }
}

void headless_ring_bell (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_ring_bell(terminal->id);

  if (terminal->headless_terminal->ring_bell_hook) {
    terminal->headless_terminal->ring_bell_hook(f);
  }
}

void headless_reset_terminal_modes (struct terminal *terminal)
{
  remote_reset_terminal_modes(terminal->id);

  if (terminal->headless_terminal->reset_terminal_modes_hook) {
    terminal->headless_terminal->reset_terminal_modes_hook(terminal);
  }
}

void headless_set_terminal_modes (struct terminal *terminal)
{
  remote_set_terminal_modes(terminal->id);

  if (terminal->headless_terminal->set_terminal_modes_hook) {
    terminal->headless_terminal->set_terminal_modes_hook(terminal);
  }
}

void headless_update_end (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_update_end(terminal->id);

  if (terminal->headless_terminal->update_end_hook) {
    terminal->headless_terminal->update_end_hook(f);
  }
}

Lisp_Object
headless_menu_show (struct frame *f, int x, int y, int menuflags,
                        Lisp_Object title, const char **error_name)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_menu_show(terminal->id, x, y, menuflags, "", error_name);

  if (terminal->headless_terminal->menu_show_hook) {
    return terminal->headless_terminal->menu_show_hook(f, x, y, menuflags, title, error_name);
  }

  return Qnil;
}

void headless_set_terminal_window (struct frame *f, int size)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_set_terminal_window(terminal->id, size);

  if (terminal->headless_terminal->set_terminal_window_hook) {
    terminal->headless_terminal->set_terminal_window_hook(f, size);
  }
}

bool headless_defined_color (struct frame *f, const char *color_name,
                            Emacs_Color *color_def, bool alloc, bool _makeIndex)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_defined_color(terminal->id, color_name, "", alloc, _makeIndex);

  if (terminal->headless_terminal->defined_color_hook) {
    return terminal->headless_terminal->defined_color_hook(f, color_name, color_def, alloc,_makeIndex);
  }

  return false;
}

int headless_read_avail_input (struct terminal *terminal,
                               struct input_event *hold_quit)
{
  remote_read_avail_input(terminal->id);

  if (terminal->headless_terminal->read_socket_hook) {
    return terminal->headless_terminal->read_socket_hook(terminal, hold_quit);
  }
  return 0;
}

void headless_delete_frame (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  remote_delete_frame(terminal->id);

  if (terminal->headless_terminal->delete_frame_hook) {
    terminal->headless_terminal->delete_frame_hook(f);
  }
}

void headless_delete_terminal (struct terminal *terminal)
{
  remote_delete_terminal(terminal->id);

  if (terminal->headless_terminal->delete_terminal_hook) {
    terminal->headless_terminal->delete_terminal_hook(terminal);
  }
}
