/* Headless window system selection support.
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

#include "lisp.h"
#include "blockinput.h"
#include "coding.h"
#include "headlessterm.h"
#include "keyboard.h"

#include <stdlib.h>

/* The frame that is currently the source of a drag-and-drop
   operation, or NULL if none is in progress.  The reason for this
   variable is to prevent it from being deleted, which really breaks
   the nested event loop inside be_drag_message.  */
struct frame *headless_dnd_frame;
