#include "headless_term.h"

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

  if (terminal->headless_terminal->cursor_to_hook) {
    terminal->headless_terminal->cursor_to_hook(f, vpos, hpos);
  }
}

void headless_raw_cursor_to (struct frame *f, int row, int col)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->raw_cursor_to_hook) {
    terminal->headless_terminal->raw_cursor_to_hook(f, row, col);
  }
}

void headless_clear_to_end (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->clear_to_end_hook) {
    terminal->headless_terminal->clear_to_end_hook(f);
  }
}

void headless_clear_frame (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->clear_frame_hook) {
    terminal->headless_terminal->clear_frame_hook(f);
  }
}

void headless_clear_end_of_line (struct frame *f, int first_unused_hpos)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->clear_end_of_line_hook) {
    terminal->headless_terminal->clear_end_of_line_hook(f, first_unused_hpos);
  }
}

void headless_ins_del_lines (struct frame *f, int vpos, int n)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->ins_del_lines_hook) {
    terminal->headless_terminal->ins_del_lines_hook(f, vpos, n);
  }
}

void headless_insert_glyphs (struct frame *f, struct glyph *start, int len)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->insert_glyphs_hook) {
    terminal->headless_terminal->insert_glyphs_hook(f, start, len);
  }
}

void headless_write_glyphs (struct frame *f, struct glyph *string, int len)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->write_glyphs_hook) {
    terminal->headless_terminal->write_glyphs_hook(f, string, len);
  }
}

void headless_delete_glyphs (struct frame *f, int n)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->delete_glyphs_hook) {
    terminal->headless_terminal->delete_glyphs_hook(f, n);
  }
}

void headless_ring_bell (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->ring_bell_hook) {
    terminal->headless_terminal->ring_bell_hook(f);
  }
}

void headless_reset_terminal_modes (struct terminal *terminal)
{
  if (terminal->headless_terminal->reset_terminal_modes_hook) {
    terminal->headless_terminal->reset_terminal_modes_hook(terminal);
  }
}

void headless_set_terminal_modes (struct terminal *terminal)
{
  if (terminal->headless_terminal->set_terminal_modes_hook) {
    terminal->headless_terminal->set_terminal_modes_hook(terminal);
  }
}

void headless_update_end (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->update_end_hook) {
    terminal->headless_terminal->update_end_hook(f);
  }
}

Lisp_Object
headless_menu_show (struct frame *f, int x, int y, int menuflags,
                        Lisp_Object title, const char **error_name)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->menu_show_hook) {
    return terminal->headless_terminal->menu_show_hook(f, x, y, menuflags, title, error_name);
  }

  return Qnil;
}

void headless_set_terminal_window (struct frame *f, int size)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->set_terminal_window_hook) {
    terminal->headless_terminal->set_terminal_window_hook(f, size);
  }
}

bool headless_defined_color (struct frame *f, const char *color_name,
                            Emacs_Color *color_def, bool alloc, bool _makeIndex)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->defined_color_hook) {
    return terminal->headless_terminal->defined_color_hook(f, color_name, color_def, alloc,_makeIndex);
  }

  return false;
}

int headless_read_avail_input (struct terminal *terminal,
                               struct input_event *hold_quit)
{
  if (terminal->headless_terminal->read_socket_hook) {
    return terminal->headless_terminal->read_socket_hook(terminal, hold_quit);
  }
  return 0;
}

void headless_delete_frame (struct frame *f)
{
  struct terminal * terminal = FRAME_TERMINAL(f);

  if (terminal->headless_terminal->delete_frame_hook) {
    terminal->headless_terminal->delete_frame_hook(f);
  }
}

void headless_delete_terminal (struct terminal *terminal)
{
  if (terminal->headless_terminal->delete_terminal_hook) {
    terminal->headless_terminal->delete_terminal_hook(terminal);
  }
}
