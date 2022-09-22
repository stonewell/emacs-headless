/* Headless window system support
   Copyright (C) 2021-2022 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.  */
#include <config.h>

#include <math.h>

#include "lisp.h"
#include "frame.h"
#include "blockinput.h"
#include "termchar.h"
#include "font.h"
#include "keyboard.h"
#include "buffer.h"
#include "dispextern.h"

#include "headlessgui.h"
#include "headlessterm.h"

unsigned long
headless_get_pixel (headless img, int x, int y)
{
  (void)img;
  (void)x;
  (void)y;

  return 0;
}

void
headless_draw_cross_on_pixmap (Emacs_Pixmap pixmap,
			 int x, int y, unsigned int width, unsigned int height,
			 unsigned long color)
{
  (void)pixmap;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)color;
}
