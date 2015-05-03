/*
 * ImageList definitions
 *
 * Copyright 1998 Eric Kohl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __WINE_IMAGELIST_H
#define __WINE_IMAGELIST_H

#include <stdarg.h>

//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"

/* the ones with offsets at the end are the same as in Windows */

#define IMAGELIST_MAGIC 0x53414D58

/* Header used by gxImageList_Read() and gxImageList_Write() */
#if defined(_WIN32) || defined(_WINDOWS) 
#include "pshpack2.h"
#endif // defined(_WIN32) || defined(_WINDOWS) 
typedef struct _ILHEAD
{
    GXUSHORT  usMagic;
    GXUSHORT  usVersion;
    GXWORD  cCurImage;
    GXWORD  cMaxImage;
    GXWORD  cGrow;
    GXWORD  cx;
    GXWORD  cy;
    GXCOLORREF  bkcolor;
    GXWORD  flags;
    GXSHORT  ovls[4];
} ILHEAD;
#if defined(_WIN32) || defined(_WINDOWS) 
#include "poppack.h"
#endif // defined(_WIN32) || defined(_WINDOWS) 

#endif  /* __WINE_IMAGELIST_H */
