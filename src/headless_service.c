#include "headless_service.h"

typedef void
(*fp_remote_cursor_to) (int terminal_id, int vpos, int hpos);
typedef void
(*fp_remote_raw_cursor_to) (int terminal_id, int row, int col);
typedef void
(*fp_remote_clear_to_end) (int terminal_id);
typedef void
(*fp_remote_clear_frame) (int terminal_id);
typedef void
(*fp_remote_clear_end_of_line) (int terminal_id, int first_unused_hpos);
typedef void
(*fp_remote_ins_del_lines) (int terminal_id, int vpos, int n);
typedef void
(*fp_remote_insert_glyphs) (int terminal_id, const wchar_t *start, int len);
typedef void
(*fp_remote_write_glyphs) (int terminal_id, const wchar_t *string, int len);
typedef void
(*fp_remote_delete_glyphs) (int terminal_id, int n);
typedef void
(*fp_remote_ring_bell) (int terminal_id);
typedef void
(*fp_remote_reset_terminal_modes) (int terminal_id);
typedef void
(*fp_remote_set_terminal_modes) (int terminal_id);
typedef void
(*fp_remote_update_end) (int terminal_id);
typedef void
(*fp_remote_menu_show) (int terminal_id, int x, int y, int menuflags,
                        const char * title, const char **error_name);
typedef void
(*fp_remote_set_terminal_window) (int terminal_id, int size);
typedef int
(*fp_remote_defined_color) (int terminal_id, const char *color_name,
                            const char *color_def, int alloc, int _makeIndex);
typedef int
(*fp_remote_read_avail_input) (int terminal_id);
typedef void
(*fp_remote_delete_frame) (int terminal_id);
typedef void
(*fp_remote_delete_terminal) (int terminal_id);

struct remote_lib
{
  int loaded;

  fp_remote_cursor_to cursor_to;
  fp_remote_raw_cursor_to raw_cursor_to;
  fp_remote_clear_to_end clear_to_end;
  fp_remote_clear_frame clear_frame;
  fp_remote_clear_end_of_line clear_end_of_line;
  fp_remote_ins_del_lines ins_del_lines;
  fp_remote_insert_glyphs insert_glyphs;
  fp_remote_write_glyphs write_glyphs;
  fp_remote_delete_glyphs delete_glyphs;
  fp_remote_ring_bell ring_bell;
  fp_remote_reset_terminal_modes reset_terminal_modes;
  fp_remote_set_terminal_modes set_terminal_modes;
  fp_remote_update_end update_end;
  fp_remote_menu_show menu_show;
  fp_remote_set_terminal_window set_terminal_window;
  fp_remote_defined_color defined_color;
  fp_remote_read_avail_input read_avail_input;
  fp_remote_delete_frame delete_frame;
  fp_remote_delete_terminal delete_terminal;
};

static struct remote_lib g_remote_lib = {0};

int headless_start_server(void)
{
  return 1;
}

int headless_stop_server(void)
{
  return 1;
}

void
remote_cursor_to (int terminal_id, int vpos, int hpos)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.cursor_to(terminal_id, vpos, hpos);
}

void
remote_raw_cursor_to (int terminal_id, int row, int col)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.raw_cursor_to(terminal_id, row, col);
}

void
remote_clear_to_end (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.clear_to_end(terminal_id);
}

void
remote_clear_frame (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.clear_frame(terminal_id);
}

void
remote_clear_end_of_line (int terminal_id, int first_unused_hpos)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.clear_end_of_line(terminal_id, first_unused_hpos);
}

void
remote_ins_del_lines (int terminal_id, int vpos, int n)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.ins_del_lines(terminal_id, vpos, n);
}

void
remote_insert_glyphs (int terminal_id, const wchar_t *start, int len)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.insert_glyphs(terminal_id, start, len);
}

void
remote_write_glyphs (int terminal_id, const wchar_t *string, int len)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.write_glyphs(terminal_id, string, len);
}

void
remote_delete_glyphs (int terminal_id, int n)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.delete_glyphs(terminal_id, n);
}

void
remote_ring_bell (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.ring_bell(terminal_id);
}

void
remote_reset_terminal_modes (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.reset_terminal_modes(terminal_id);
}

void
remote_set_terminal_modes (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.set_terminal_modes(terminal_id);
}

void
remote_update_end (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.update_end(terminal_id);
}

void
remote_menu_show (int terminal_id, int x, int y, int menuflags,
                  const char * title, const char **error_name)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.menu_show(terminal_id, x, y, menuflags, title, error_name);
}

void
remote_set_terminal_window (int terminal_id, int size)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.set_terminal_window(terminal_id, size);
}

int
remote_defined_color (int terminal_id, const char *color_name,
                      const char *color_def, int alloc, int _makeIndex)
{
  if (!g_remote_lib.loaded) return 0;

  return g_remote_lib.defined_color(terminal_id, color_name, color_def, alloc, _makeIndex);
}

int
remote_read_avail_input (int terminal_id)
{
  if (!g_remote_lib.loaded) return 0;

  return g_remote_lib.read_avail_input(terminal_id);
}

void
remote_delete_frame (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.delete_frame(terminal_id);
}

void
remote_delete_terminal (int terminal_id)
{
  if (!g_remote_lib.loaded) return;

  g_remote_lib.delete_terminal(terminal_id);
}
