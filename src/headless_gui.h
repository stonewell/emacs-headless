/* Headless window system support.
   Copyright (C) 2023-2024 Free Software Foundation, Inc.

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

#ifndef _HEADLESS_GUI_H_
#define _HEADLESS_GUI_H_

struct headless_char_struct
{
  int rbearing;
  int lbearing;
  int width;
  int ascent;
  int descent;
};

typedef struct headless_char_struct XCharStruct;

typedef unsigned short headless_handle;

typedef headless_handle headless_pixmap, Emacs_Pixmap;
typedef headless_handle headless_window, Emacs_Window;
typedef headless_handle headless_gcontext, GContext;
typedef headless_handle headless_drawable, Drawable;
typedef headless_handle headless_cursor, Emacs_Cursor;

typedef unsigned int headless_time;

struct headless_rectangle
{
  int x, y;
  unsigned width, height;
};

struct headless_point
{
  int x, y;
};

#define NativeRectangle			Emacs_Rectangle

#define ForgetGravity		0
#define NorthWestGravity	1
#define NorthGravity		2
#define NorthEastGravity	3
#define WestGravity		4
#define CenterGravity		5
#define EastGravity		6
#define SouthWestGravity	7
#define SouthGravity		8
#define SouthEastGravity	9
#define StaticGravity		10

#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020

#define USPosition	(1L << 0) /* user specified x, y */
#define USSize		(1L << 1) /* user specified width, height */
#define PPosition	(1L << 2) /* program specified position */
#define PSize		(1L << 3) /* program specified size */
#define PMinSize	(1L << 4) /* program specified minimum size */
#define PMaxSize	(1L << 5) /* program specified maximum size */
#define PResizeInc	(1L << 6) /* program specified resize increments */
#define PAspect		(1L << 7) /* program specified min, max aspect ratios */
#define PBaseSize	(1L << 8) /* program specified base for incrementing */
#define PWinGravity	(1L << 9) /* program specified window gravity */

#endif /* _HEADLESS_GUI_H_ */
