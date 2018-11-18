#ifndef __SERVICE_ONLY_GUI_H__
#define __SERVICE_ONLY_GUI_H__

typedef unsigned int COLORREF;

typedef struct _XGCValues
{
  COLORREF foreground;
  COLORREF background;
  struct font *font;
} XGCValues;

#define GCForeground 0x01
#define GCBackground 0x02
#define GCFont 0x03

typedef void * HANDLE;
typedef HANDLE Pixmap;
typedef HANDLE Bitmap;

typedef char * XrmDatabase;

typedef XGCValues * GC;
typedef COLORREF Color;
typedef HANDLE Window;
typedef HANDLE Display;
typedef HANDLE Cursor;

#define No_Cursor (0)

#define XChar2b wchar_t

/* Dealing with bits of wchar_t as if they were an XChar2b.  */
#define STORE_XCHAR2B(chp, byte1, byte2) \
  ((*(chp)) = ((XChar2b)((((byte1) & 0x00ff) << 8) | ((byte2) & 0x00ff))))

#define XCHAR2B_BYTE1(chp) \
  (((*(chp)) & 0xff00) >> 8)

#define XCHAR2B_BYTE2(chp) \
  ((*(chp)) & 0x00ff)


typedef struct _BITMAPINFO {
} BITMAPINFO;

typedef struct _XImage
{
  unsigned char * data;
  BITMAPINFO info;
  /* Optional RGBQUAD array for palette follows (see BITMAPINFO docs).  */
} XImage;

#define FACE_DEFAULT (~0)

extern HANDLE hinst;

/* Bit Gravity */

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
#define PAspect		(1L << 7) /* program specified min and max aspect ratios */
#define PBaseSize	(1L << 8) /* program specified base for incrementing */
#define PWinGravity	(1L << 9) /* program specified window gravity */

typedef struct {
    int x, y;
    unsigned width, height;
} XRectangle;

typedef struct {
    int left, top;
    int right, bottom;
} RECT;

#define NativeRectangle RECT

#define CONVERT_TO_XRECT(xr,nr)			\
  ((xr).x = (nr).left,				\
   (xr).y = (nr).top,				\
   (xr).width = ((nr).right - (nr).left),	\
   (xr).height = ((nr).bottom - (nr).top))

#define CONVERT_FROM_XRECT(xr,nr)		\
  ((nr).left = (xr).x,				\
   (nr).top = (xr).y,				\
   (nr).right = ((xr).x + (xr).width),		\
   (nr).bottom = ((xr).y + (xr).height))

#define STORE_NATIVE_RECT(nr,x,y,width,height)	\
  ((nr).left = (x),				\
   (nr).top = (y),				\
   (nr).right = ((nr).left + (width)),		\
   (nr).bottom = ((nr).top + (height)))

extern struct font_driver sofont_driver;
extern void syms_of_sofont (void);

#endif //__SERVICE_ONLY_GUI_H__
