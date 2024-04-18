#include <config.h>

#include <signal.h>
#include <stdio.h>
#include <setjmp.h>

#include "lisp.h"
#include "keyboard.h"
#include "frame.h"
#include "blockinput.h"
#include "buffer.h"
#include "coding.h"	/* for ENCODE_SYSTEM */
#include "menu.h"
#include "pdumper.h"

#include "headless_term.h"

void
set_frame_menubar (struct frame *f, bool deep_p)
{
}
