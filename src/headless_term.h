/* headless Terminal control module
   Copyright (C) 1985-1987, 1993-1995, 1998, 2000-2022 Free Software
   Foundation, Inc.

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

#ifndef __HEADLESS_TERM_H__
#define __HEADLESS_TERM_H__

#include <config.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>

#include "lisp.h"
#include "termchar.h"
#include "tparam.h"
#include "character.h"
#include "buffer.h"
#include "charset.h"
#include "coding.h"
#include "composite.h"
#include "keyboard.h"
#include "frame.h"
#include "disptab.h"
#include "termhooks.h"
#include "dispextern.h"
#include "window.h"
#include "keymap.h"
#include "blockinput.h"
#include "syssignal.h"
#include "sysstdio.h"

void headless_set_hooks (struct terminal *terminal);

#endif //__HEADLESS_TERM_H__
