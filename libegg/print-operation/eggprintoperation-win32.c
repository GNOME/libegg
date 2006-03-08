/* EGG - The GIMP Toolkit
 * eggprintoperation-win32.c: Print Operation Details for Win32
 * Copyright (C) 2006, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "eggprintoperation-private.h"
#include "eggprint-win32.h"
#include <cairo-win32.h>
#include <glib.h>
#include <stdlib.h>

#define _(x) (x)

#define MAX_PAGE_RANGES 20

typedef struct {
  HDC hdc;
  HGLOBAL devmode;
} EggPrintOperationWin32;

/* Base64 utils (straight from camel-mime-utils.c) */
static char *base64_alphabet =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * performs an 'encode step', only encodes blocks of 3 characters to the
 * output at a time, saves left-over state in state and save (initialise to
 * 0 on first invocation).
 */
static int
base64_encode_step (const guchar  *in, 
		    int            len, 
		    gboolean       break_lines, 
		    guchar        *out, 
		    int           *state, 
		    int           *save)
{
	register guchar *outptr;
	register const guchar *inptr;

	if (len <= 0)
		return 0;

	inptr = in;
	outptr = out;

	if (len + ((char *) save) [0] > 2) {
		const guchar *inend = in+len-2;
		register int c1, c2, c3;
		register int already;

		already = *state;

		switch (((char *) save) [0]) {
		case 1:	c1 = ((unsigned char *) save) [1]; goto skip1;
		case 2:	c1 = ((unsigned char *) save) [1];
			c2 = ((unsigned char *) save) [2]; goto skip2;
		}
		
		/* 
		 * yes, we jump into the loop, no i'm not going to change it, 
		 * it's beautiful! 
		 */
		while (inptr < inend) {
			c1 = *inptr++;
		skip1:
			c2 = *inptr++;
		skip2:
			c3 = *inptr++;
			*outptr++ = base64_alphabet [ c1 >> 2 ];
			*outptr++ = base64_alphabet [ c2 >> 4 | 
						      ((c1&0x3) << 4) ];
			*outptr++ = base64_alphabet [ ((c2 &0x0f) << 2) | 
						      (c3 >> 6) ];
			*outptr++ = base64_alphabet [ c3 & 0x3f ];
			/* this is a bit ugly ... */
			if (break_lines && (++already)>=19) {
				*outptr++='\n';
				already = 0;
			}
		}

		((char *)save)[0] = 0;
		len = 2-(inptr-inend);
		*state = already;
	}

	if (len>0) {
		register char *saveout;

		/* points to the slot for the next char to save */
		saveout = & (((char *)save)[1]) + ((char *)save)[0];

		/* len can only be 0 1 or 2 */
		switch(len) {
		case 2:	*saveout++ = *inptr++;
		case 1:	*saveout++ = *inptr++;
		}
		((char *)save)[0]+=len;
	}

	return outptr-out;
}

/* 
 * call this when finished encoding everything, to
 * flush off the last little bit 
 */
static int
base64_encode_close (const guchar  *in, 
		     int            inlen, 
		     gboolean       break_lines, 
		     guchar        *out, 
		     int           *state, 
		     int           *save)
{
	int c1, c2;
	unsigned char *outptr = out;

	if (inlen > 0)
		outptr += base64_encode_step (in, 
					      inlen, 
					      break_lines, 
					      outptr, 
					      state, 
					      save);

	c1 = ((unsigned char *) save) [1];
	c2 = ((unsigned char *) save) [2];
	
	switch (((char *) save) [0]) {
	case 2:
		outptr [2] = base64_alphabet[ ( (c2 &0x0f) << 2 ) ];
		g_assert (outptr [2] != 0);
		goto skip;
	case 1:
		outptr[2] = '=';
	skip:
		outptr [0] = base64_alphabet [ c1 >> 2 ];
		outptr [1] = base64_alphabet [ c2 >> 4 | ( (c1&0x3) << 4 )];
		outptr [3] = '=';
		outptr += 4;
		break;
	}
	if (break_lines)
		*outptr++ = '\n';

	*save = 0;
	*state = 0;

	return outptr-out;
}

/**
 * base64_encode:
 * @text: the binary data to encode.
 * @len: the length of @text.
 *
 * Encode a sequence of binary data into it's Base-64 stringified
 * representation.
 *
 * Return value: The Base-64 encoded string representing @text.
 */
static char *
base64_encode (const char *text, int len)
{
        unsigned char *out;
        int state = 0, outlen;
        unsigned int save = 0;
        
        out = g_malloc (len * 4 / 3 + 5);
        outlen = base64_encode_close (text, 
				      len, 
				      FALSE,
				      out, 
				      &state, 
				      &save);
        out[outlen] = '\0';
        return (char *) out;
}

static unsigned char camel_mime_base64_rank[256] = {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
	 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
	255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

/**
 * base64_decode_step: decode a chunk of base64 encoded data
 * @in: input stream
 * @len: max length of data to decode
 * @out: output stream
 * @state: holds the number of bits that are stored in @save
 * @save: leftover bits that have not yet been decoded
 *
 * Decodes a chunk of base64 encoded data
 **/
static int
base64_decode_step (const guchar  *in, 
		    int            len, 
		    guchar        *out, 
		    int           *state, 
		    guint         *save)
{
	register const guchar *inptr;
	register guchar *outptr;
	const guchar *inend;
	guchar c;
	register unsigned int v;
	int i;

	inend = in+len;
	outptr = out;

	/* convert 4 base64 bytes to 3 normal bytes */
	v=*save;
	i=*state;
	inptr = in;
	while (inptr < inend) {
		c = camel_mime_base64_rank [*inptr++];
		if (c != 0xff) {
			v = (v<<6) | c;
			i++;
			if (i==4) {
				*outptr++ = v>>16;
				*outptr++ = v>>8;
				*outptr++ = v;
				i=0;
			}
		}
	}

	*save = v;
	*state = i;

	/* quick scan back for '=' on the end somewhere */
	/* fortunately we can drop 1 output char for each trailing = (upto 2) */
	i=2;
	while (inptr > in && i) {
		inptr--;
		if (camel_mime_base64_rank [*inptr] != 0xff) {
			if (*inptr == '=')
				outptr--;
			i--;
		}
	}

	/* if i!= 0 then there is a truncation error! */
	return outptr - out;
}

static char *
base64_decode (const char   *text,
	       int          *out_len)
{
	char *ret;
	int inlen, state = 0, save = 0;

	inlen = strlen (text);
	ret = g_malloc0 (inlen);

	*out_len = base64_decode_step (text, inlen, ret, &state, &save);

	return ret; 
}

static EggPageOrientation
orientation_from_win32 (short orientation)
{
  if (orientation == DMORIENT_LANDSCAPE)
    return EGG_PAGE_ORIENTATION_LANDSCAPE;
  return EGG_PAGE_ORIENTATION_PORTRAIT;
}

static short
orientation_to_win32 (EggPageOrientation orientation)
{
  if (orientation == EGG_PAGE_ORIENTATION_LANDSCAPE)
    return DMORIENT_LANDSCAPE;
  return DMORIENT_PORTRAIT;
}

static const char *
page_size_from_win32 (short size)
{
  switch (size)
    {
    case DMPAPER_LETTER_TRANSVERSE:
    case DMPAPER_LETTER:
    case DMPAPER_LETTERSMALL:
      return "na_letter";
    case DMPAPER_TABLOID:
    case DMPAPER_LEDGER:
      return "na_ledger";
    case DMPAPER_LEGAL:
      return "na_legal";
    case DMPAPER_STATEMENT:
      return "na_invoice";
    case DMPAPER_EXECUTIVE:
      return "na_executive";
    case DMPAPER_A3:
    case DMPAPER_A3_TRANSVERSE:
      return "iso_a3";
    case DMPAPER_A4:
    case DMPAPER_A4SMALL:
    case DMPAPER_A4_TRANSVERSE:
      return "iso_a4";
    case DMPAPER_A5:
    case DMPAPER_A5_TRANSVERSE:
      return "iso_a5";
    case DMPAPER_B4:
      return "iso_b4";
    case DMPAPER_B5:
    case DMPAPER_B5_TRANSVERSE:
      return "iso_b5";
    case DMPAPER_QUARTO:
      return "na_quarto";
    case DMPAPER_10X14:
      return "na_10x14";
    case DMPAPER_11X17:
      return "na_ledger";
    case DMPAPER_NOTE:
      return "na_letter";
    case DMPAPER_ENV_9:
      return "na_number-9";
    case DMPAPER_ENV_10:
      return "na_number-10";
    case DMPAPER_ENV_11:
      return "na_number-11";
    case DMPAPER_ENV_12:
      return "na_number-12";
    case DMPAPER_ENV_14:
      return "na_number-14";
    case DMPAPER_CSHEET:
      return "na_c";
    case DMPAPER_DSHEET:
      return "na_d";
    case DMPAPER_ESHEET:
      return "na_e";
    case DMPAPER_ENV_DL:
      return "iso_dl";
    case DMPAPER_ENV_C5:
      return "iso_c5";
    case DMPAPER_ENV_C3:
      return "iso_c3";
    case DMPAPER_ENV_C4:
      return "iso_c4";
    case DMPAPER_ENV_C6:
      return "iso_c6";
    case DMPAPER_ENV_C65:
      return "iso_c6c5";
    case DMPAPER_ENV_B4:
      return "iso_b4";
    case DMPAPER_ENV_B5:
      return "iso_b5";
    case DMPAPER_ENV_B6:
      return "iso_b6";
    case DMPAPER_ENV_ITALY:
      return "om_italian";
    case DMPAPER_ENV_MONARCH:
      return "na_monarch";
    case DMPAPER_ENV_PERSONAL:
      return "na_personal";
    case DMPAPER_FANFOLD_US:
      return "na_fanfold-us";
    case DMPAPER_FANFOLD_STD_GERMAN:
      return "na_fanfold-eur";
    case DMPAPER_FANFOLD_LGL_GERMAN:
      return "na_foolscap";
    case DMPAPER_ISO_B4:
      return "iso_b4";
    case DMPAPER_JAPANESE_POSTCARD:
      return "jpn_hagaki";
    case DMPAPER_9X11:
      return "na_9x11";
    case DMPAPER_10X11:
      return "na_10x11";
    case DMPAPER_ENV_INVITE:
      return "om_invite";
    case DMPAPER_LETTER_EXTRA:
    case DMPAPER_LETTER_EXTRA_TRANSVERSE:
      return "na_letter-extra";
    case DMPAPER_LEGAL_EXTRA:
      return "na_legal-extra";
    case DMPAPER_TABLOID_EXTRA:
      return "na_arch";
    case DMPAPER_A4_EXTRA:
      return "iso_a4-extra";
    case DMPAPER_B_PLUS:
      return "na_b-plus";
    case DMPAPER_LETTER_PLUS:
      return "na_letter-plus";
    case DMPAPER_A3_EXTRA:
    case DMPAPER_A3_EXTRA_TRANSVERSE:
      return "iso_a3-extra";
    case DMPAPER_A5_EXTRA:
      return "iso_a5-extra";
    case DMPAPER_B5_EXTRA:
      return "iso_b5-extra";
    case DMPAPER_A2:
      return "iso_a2";
      
    default:
      /* Dunno what these are: */
    case DMPAPER_A4_PLUS:
    case DMPAPER_A_PLUS:
    case DMPAPER_FOLIO:
    case DMPAPER_15X11:
      return "iso_a4";
    }
}

static short
paper_size_to_win32 (EggPaperSize *paper_size)
{
  const char *format;

  if (egg_paper_size_is_custom (paper_size))
    return 0;
  
  format = egg_paper_size_get_name (paper_size);

  if (strcmp (format, "na_letter") == 0)
    return DMPAPER_LETTER;
  if (strcmp (format, "na_ledger") == 0)
    return DMPAPER_LEDGER;
  if (strcmp (format, "na_legal") == 0)
    return DMPAPER_LEGAL;
  if (strcmp (format, "na_invoice") == 0)
    return DMPAPER_STATEMENT;
  if (strcmp (format, "na_executive") == 0)
    return DMPAPER_EXECUTIVE;
  if (strcmp (format, "iso_a2") == 0)
    return DMPAPER_A2;
  if (strcmp (format, "iso_a3") == 0)
    return DMPAPER_A3;
  if (strcmp (format, "iso_a4") == 0)
    return DMPAPER_A4;
  if (strcmp (format, "iso_a5") == 0)
    return DMPAPER_A5;
  if (strcmp (format, "iso_b4") == 0)
    return DMPAPER_B4;
  if (strcmp (format, "iso_b5") == 0)
    return DMPAPER_B5;
  if (strcmp (format, "na_quarto") == 0)
    return DMPAPER_QUARTO;
  if (strcmp (format, "na_10x14") == 0)
    return DMPAPER_10X14;
  if (strcmp (format, "na_number-9") == 0)
    return DMPAPER_ENV_9;
  if (strcmp (format, "na_number-10") == 0)
    return DMPAPER_ENV_10;
  if (strcmp (format, "na_number-11") == 0)
    return DMPAPER_ENV_11;
  if (strcmp (format, "na_number-12") == 0)
    return DMPAPER_ENV_12;
  if (strcmp (format, "na_number-14") == 0)
    return DMPAPER_ENV_14;
  if (strcmp (format, "na_c") == 0)
    return DMPAPER_CSHEET;
  if (strcmp (format, "na_d") == 0)
    return DMPAPER_DSHEET;
  if (strcmp (format, "na_e") == 0)
    return DMPAPER_ESHEET;
  if (strcmp (format, "iso_dl") == 0)
    return DMPAPER_ENV_DL;
  if (strcmp (format, "iso_c3") == 0)
    return DMPAPER_ENV_C3;
  if (strcmp (format, "iso_c4") == 0)
    return DMPAPER_ENV_C4;
  if (strcmp (format, "iso_c5") == 0)
    return DMPAPER_ENV_C5;
  if (strcmp (format, "iso_c6") == 0)
    return DMPAPER_ENV_C6;
  if (strcmp (format, "iso_c5c6") == 0)
    return DMPAPER_ENV_C65;
  if (strcmp (format, "iso_b6") == 0)
    return DMPAPER_ENV_B6;
  if (strcmp (format, "om_italian") == 0)
    return DMPAPER_ENV_ITALY;
  if (strcmp (format, "na_monarch") == 0)
    return DMPAPER_ENV_MONARCH;
  if (strcmp (format, "na_personal") == 0)
    return DMPAPER_ENV_PERSONAL;
  if (strcmp (format, "na_fanfold-us") == 0)
    return DMPAPER_FANFOLD_US;
  if (strcmp (format, "na_fanfold-eur") == 0)
    return DMPAPER_FANFOLD_STD_GERMAN;
  if (strcmp (format, "na_foolscap") == 0)
    return DMPAPER_FANFOLD_LGL_GERMAN;
  if (strcmp (format, "jpn_hagaki") == 0)
    return DMPAPER_JAPANESE_POSTCARD;
  if (strcmp (format, "na_9x11") == 0)
    return DMPAPER_9X11;
  if (strcmp (format, "na_10x11") == 0)
    return DMPAPER_10X11;
  if (strcmp (format, "om_invite") == 0)
    return DMPAPER_ENV_INVITE;
  if (strcmp (format, "na_letter-extra") == 0)
    return DMPAPER_LETTER_EXTRA;
  if (strcmp (format, "na_legal-extra") == 0)
    return DMPAPER_LEGAL_EXTRA;
  if (strcmp (format, "na_arch") == 0)
    return DMPAPER_TABLOID_EXTRA;
  if (strcmp (format, "iso_a3-extra") == 0)
    return DMPAPER_A3_EXTRA;
  if (strcmp (format, "iso_a4-extra") == 0)
    return DMPAPER_A4_EXTRA;
  if (strcmp (format, "iso_a5-extra") == 0)
    return DMPAPER_A5_EXTRA;
  if (strcmp (format, "iso_b5-extra") == 0)
    return DMPAPER_B5_EXTRA;
  if (strcmp (format, "na_b-plus") == 0)
    return DMPAPER_B_PLUS;
  if (strcmp (format, "na_letter-plus") == 0)
    return DMPAPER_LETTER_PLUS;

  return 0;
}

void
win32_start_page (EggPrintOperation *op,
		  EggPrintContext *print_context,
		  EggPageSetup *page_setup)
{
  EggPrintOperationWin32 *op_win32 = op->priv->platform_data;
  LPDEVMODEW devmode;
  EggPaperSize *paper_size;
  
  devmode = GlobalLock (op_win32->devmode);
  
  devmode->dmFields |= DM_ORIENTATION;
  devmode->dmOrientation =
    orientation_to_win32 (egg_page_setup_get_orientation (page_setup));
  
  paper_size = egg_page_setup_get_paper_size (page_setup);
  devmode->dmFields |= DM_PAPERSIZE;
  devmode->dmFields &= ~(DM_PAPERWIDTH | DM_PAPERLENGTH);
  devmode->dmPaperSize = paper_size_to_win32 (paper_size);
  if (devmode->dmPaperSize == 0)
    {
      devmode->dmFields |= DM_PAPERWIDTH | DM_PAPERLENGTH;
      devmode->dmPaperWidth = egg_paper_size_get_width (paper_size, EGG_UNIT_MM) * 10.0;
      devmode->dmPaperLength = egg_paper_size_get_height (paper_size, EGG_UNIT_MM) * 10.0;
    }
  
  ResetDCW (op_win32->hdc, devmode);
  
  GlobalUnlock (op_win32->devmode);
  
  StartPage (op_win32->hdc);
}

static void
win32_end_page (EggPrintOperation *op,
		EggPrintContext *print_context)
{
  EggPrintOperationWin32 *op_win32 = op->priv->platform_data;
  EndPage (op_win32->hdc);
}

static void
win32_end_run (EggPrintOperation *op)
{
  EggPrintOperationWin32 *op_win32 = op->priv->platform_data;

  EndDoc (op_win32->hdc);
  GlobalFree(op_win32->devmode);

  cairo_surface_destroy (op->priv->surface);
  op->priv->surface = NULL;

  DeleteDC(op_win32->hdc);
  
  g_free (op_win32);
}

static HWND
get_parent_hwnd (GtkWidget *widget)
{
  gtk_widget_realize (widget);
  return gdk_win32_drawable_get_handle (widget->window);
}



static void
dialog_to_printer_settings (EggPrintOperation *op,
			    LPPRINTDLGEXW printdlgex)
{
  int i;
  LPDEVMODEW devmode;
  EggPrinterSettings *settings;

  settings = egg_printer_settings_new ();

  egg_printer_settings_set_print_pages (settings,
					EGG_PRINT_PAGES_ALL);
  if (printdlgex->Flags & PD_CURRENTPAGE)
    egg_printer_settings_set_print_pages (settings,
					  EGG_PRINT_PAGES_CURRENT);
  else if (printdlgex->Flags & PD_PAGENUMS)
    egg_printer_settings_set_print_pages (settings,
					  EGG_PRINT_PAGES_RANGES);

  if (printdlgex->nPageRanges > 0)
    {
      EggPageRange *ranges;
      ranges = g_new (EggPageRange, printdlgex->nPageRanges);

      for (i = 0; i < printdlgex->nPageRanges; i++)
	{
	  ranges[i].start = printdlgex->lpPageRanges[i].nFromPage - 1;
	  ranges[i].end = printdlgex->lpPageRanges[i].nToPage - 1;
	}

      egg_printer_settings_set_page_ranges (settings, ranges,
					    printdlgex->nPageRanges);
      g_free (ranges);
    }
  
  if (printdlgex->hDevNames != NULL) 
    {
      EggPrintWin32Devnames *devnames = egg_print_win32_devnames_from_win32 (printdlgex->hDevNames);
      egg_printer_settings_set_printer (settings,
					devnames->device);
      egg_print_win32_devnames_free (devnames);
    }

  if (printdlgex->hDevMode != NULL) 
    {
      devmode = GlobalLock (printdlgex->hDevMode);

      egg_printer_settings_set_int (settings, EGG_PRINTER_SETTINGS_WIN32_DRIVER_VERSION,
				    devmode->dmDriverVersion);
      if (devmode->dmDriverExtra != 0)
	{
	  char *extra = base64_encode (((char *)devmode) + sizeof (DEVMODEW),
				       devmode->dmDriverExtra);
	  egg_printer_settings_set (settings,
				    EGG_PRINTER_SETTINGS_WIN32_DRIVER_EXTRA,
				    extra);
	  g_free (extra);
	}
      
      if (devmode->dmFields & DM_ORIENTATION)
	egg_printer_settings_set_orientation (settings,
					      orientation_from_win32 (devmode->dmOrientation));


      if (devmode->dmFields & DM_PAPERSIZE)
	{
	  if (devmode->dmPaperSize != 0)
	    {
	      EggPaperSize *paper_size = egg_paper_size_new (page_size_from_win32 (devmode->dmPaperSize));
	      egg_printer_settings_set_paper_size (settings, paper_size);
	      egg_paper_size_free (paper_size);
	    }
	  else
	    {
	      EggPaperSize *paper_size = egg_paper_size_new_custom ("dialog",
								    devmode->dmPaperWidth * 10.0,
								    devmode->dmPaperLength * 10.0,
								    EGG_UNIT_MM);
	      egg_printer_settings_set_paper_size (settings, paper_size);
	      egg_paper_size_free (paper_size);
	    }
	}

      if (devmode->dmFields & DM_SCALE)
	egg_printer_settings_set_scale (settings,
					devmode->dmScale / 100.0);

      if (devmode->dmFields & DM_COPIES)
	egg_printer_settings_set_num_copies (settings,
					     devmode->dmCopies);
      
      if (devmode->dmFields & DM_DEFAULTSOURCE)
	{
	  char *source;
	  switch (devmode->dmDefaultSource)
	    {
	    default:
	    case DMBIN_AUTO:
	      source = "auto";
	      break;
	    case DMBIN_CASSETTE:
	      source = "cassette";
	      break;
	    case DMBIN_ENVELOPE:
	      source = "envelope";
	      break;
	    case DMBIN_ENVMANUAL:
	      source = "envelope-manual";
	      break;
	    case DMBIN_LOWER:
	      source = "lower";
	      break;
	    case DMBIN_MANUAL:
	      source = "manual";
	      break;
	    case DMBIN_MIDDLE:
	      source = "middle";
	      break;
	    case DMBIN_ONLYONE:
	      source = "only-one";
	      break;
	    case DMBIN_FORMSOURCE:
	      source = "form-source";
	      break;
	    case DMBIN_LARGECAPACITY:
	      source = "large-capacity";
	      break;
	    case DMBIN_LARGEFMT:
	      source = "large-format";
	      break;
	    case DMBIN_TRACTOR:
	      source = "tractor";
	      break;
	    case DMBIN_SMALLFMT:
	      source = "small-format";
	      break;
	    }
	  egg_printer_settings_set_default_source (settings, source);
	}

      if (devmode->dmFields & DM_PRINTQUALITY)
	{
	  EggPrintQuality quality;
	  switch (devmode->dmPrintQuality)
	    {
	    case DMRES_LOW:
	      quality = EGG_PRINT_QUALITY_LOW;
	      break;
	    case DMRES_MEDIUM:
	      quality = EGG_PRINT_QUALITY_NORMAL;
	      break;
	    default:
	    case DMRES_HIGH:
	      quality = EGG_PRINT_QUALITY_HIGH;
	      break;
	    case DMRES_DRAFT:
	      quality = EGG_PRINT_QUALITY_DRAFT;
	      break;
	    }
	  egg_printer_settings_set_quality (settings, quality);
	}
      
      if (devmode->dmFields & DM_COLOR)
	egg_printer_settings_set_use_color (settings, devmode->dmFields == DMCOLOR_COLOR);

      if (devmode->dmFields & DM_DUPLEX)
	{
	  EggPrintDuplex duplex;
	  switch (duplex)
	    {
	    default:
	    case DMDUP_SIMPLEX:
	      duplex = EGG_PRINT_DUPLEX_SIMPLEX;
	      break;
	    case DMDUP_HORIZONTAL:
	      duplex = EGG_PRINT_DUPLEX_HORIZONTAL;
	      break;
	    case DMDUP_VERTICAL:
	      duplex = EGG_PRINT_DUPLEX_VERTICAL;
	      break;
	    }
	  
	  egg_printer_settings_set_duplex (settings, duplex);
	}
      
      if (devmode->dmFields & DM_COLLATE)
	egg_printer_settings_set_collate (settings,
					  devmode->dmCollate == DMCOLLATE_TRUE);

      if (devmode->dmFields & DM_MEDIATYPE)
	{
	  char *media_type;
	  switch (devmode->dmMediaType)
	    {
	    default:
	    case DMMEDIA_STANDARD:
	      media_type = "stationery";
	      break;
	    case DMMEDIA_TRANSPARENCY:
	      media_type = "transparency";
	      break;
	    case DMMEDIA_GLOSSY:
	      media_type = "photographic-glossy";
	      break;
	    }
	  egg_printer_settings_set_media_type (settings, media_type);
	}

      if (devmode->dmFields & DM_DITHERTYPE)
	{
	  char *dither;
	  switch (devmode->dmDitherType)
	    {
	    default:
	    case DMDITHER_FINE:
	      dither = "fine";
	      break;
	    case DMDITHER_NONE:
	      dither = "none";
	      break;
	    case DMDITHER_COARSE:
	      dither = "coarse";
	      break;
	    case DMDITHER_LINEART:
	      dither = "lineart";
	      break;
	    case DMDITHER_GRAYSCALE:
	      dither = "grayscale";
	      break;
	    case DMDITHER_ERRORDIFFUSION:
	      dither = "error-diffusion";
	      break;
	    }
	  egg_printer_settings_set_dither (settings, dither);
	}
      	  
      GlobalUnlock (printdlgex->hDevMode);
    }
  
  egg_print_operation_set_printer_settings (op, settings);
}

static void
dialog_from_printer_settings (EggPrintOperation *op,
			     LPPRINTDLGEXW printdlgex)
{
  EggPrinterSettings *settings = op->priv->printer_settings;
  const char *printer;
  const char *extras_base64;
  const char *val;
  EggPaperSize *paper_size;
  char *extras;
  int extras_len;
  
  LPDEVMODEW devmode;

  if (settings == NULL)
    return;

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_PRINT_PAGES))
    {
      EggPrintPages print_pages = egg_printer_settings_get_print_pages (settings);

      switch (print_pages)
	{
	default:
	case EGG_PRINT_PAGES_ALL:
	  printdlgex->Flags |= PD_ALLPAGES;
	  break;
	case EGG_PRINT_PAGES_CURRENT:
	  printdlgex->Flags |= PD_CURRENTPAGE;
	  break;
	case EGG_PRINT_PAGES_RANGES:
	  printdlgex->Flags |= PD_PAGENUMS;
	  break;
	}
    }
  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_PAGE_RANGES))
    {
      EggPageRange *ranges;
      int num_ranges, i;

      ranges = egg_printer_settings_get_page_ranges (settings, &num_ranges);

      if (num_ranges > MAX_PAGE_RANGES)
	num_ranges = MAX_PAGE_RANGES;

      printdlgex->nPageRanges = num_ranges;
      for (i = 0; i < num_ranges; i++)
	{
	  printdlgex->lpPageRanges[i].nFromPage = ranges[i].start + 1;
	  printdlgex->lpPageRanges[i].nToPage = ranges[i].end + 1;
	}
    }
  
  printer = egg_printer_settings_get_printer (settings);
  if (printer)
    printdlgex->hDevNames = egg_print_win32_devnames_from_printer_name (printer);

  extras = NULL;
  extras_len = 0;
  extras_base64 = egg_printer_settings_get (settings, EGG_PRINTER_SETTINGS_WIN32_DRIVER_EXTRA);
  if (extras_base64)
    {
      extras = base64_decode (extras_base64, &extras_len);
    }
  
  
  printdlgex->hDevMode = GlobalAlloc (GMEM_MOVEABLE, 
				      sizeof (DEVMODEW) + extras_len);

  devmode = GlobalLock (printdlgex->hDevMode);

  memset (devmode, 0, sizeof (DEVMODEW));
  
  devmode->dmSpecVersion = DM_SPECVERSION;
  devmode->dmSize = sizeof (DEVMODEW);
  
  devmode->dmDriverExtra = 0;
  if (extras && extras_len > 0)
    {
      devmode->dmDriverExtra = extras_len;
      memcpy (((char *)devmode) + sizeof (DEVMODEW), extras, extras_len);
      g_free (extras);
    }
  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_WIN32_DRIVER_VERSION))
    devmode->dmDriverVersion = egg_printer_settings_get_int (settings, EGG_PRINTER_SETTINGS_WIN32_DRIVER_VERSION);

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_ORIENTATION))
    {
      devmode->dmFields |= DM_ORIENTATION;
      devmode->dmOrientation =
	orientation_to_win32 (egg_printer_settings_get_orientation (settings));
    }

  paper_size = egg_printer_settings_get_paper_size (settings);
  if (paper_size)
    {
      devmode->dmFields |= DM_PAPERSIZE;
      devmode->dmPaperSize = paper_size_to_win32 (paper_size);
      if (devmode->dmPaperSize == 0)
	{
	  devmode->dmFields |= DM_PAPERWIDTH | DM_PAPERLENGTH;
	  devmode->dmPaperWidth = egg_paper_size_get_width (paper_size, EGG_UNIT_MM) * 10.0;
	  devmode->dmPaperLength = egg_paper_size_get_height (paper_size, EGG_UNIT_MM) * 10.0;
	}
      egg_paper_size_free (paper_size);
    }

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_SCALE))
    {
      devmode->dmFields |= DM_SCALE;
      devmode->dmScale = egg_printer_settings_get_scale (settings) * 100;
    }
  
  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_NUM_COPIES))
    {
      devmode->dmFields |= DM_COPIES;
      devmode->dmCopies = egg_printer_settings_get_num_copies (settings);
    }
  
  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_DEFAULT_SOURCE))
    {
      devmode->dmFields |= DM_DEFAULTSOURCE;
      devmode->dmDefaultSource = DMBIN_AUTO;

      val = egg_printer_settings_get_default_source (settings);
      if (strcmp (val, "auto") == 0)
	devmode->dmDefaultSource = DMBIN_AUTO;
      if (strcmp (val, "cassette") == 0)
	devmode->dmDefaultSource = DMBIN_CASSETTE;
      if (strcmp (val, "envelope") == 0)
	devmode->dmDefaultSource = DMBIN_ENVELOPE;
      if (strcmp (val, "envelope-manual") == 0)
	devmode->dmDefaultSource = DMBIN_ENVMANUAL;
      if (strcmp (val, "lower") == 0)
	devmode->dmDefaultSource = DMBIN_LOWER;
      if (strcmp (val, "manual") == 0)
	devmode->dmDefaultSource = DMBIN_MANUAL;
      if (strcmp (val, "middle") == 0)
	devmode->dmDefaultSource = DMBIN_MIDDLE;
      if (strcmp (val, "only-one") == 0)
	devmode->dmDefaultSource = DMBIN_ONLYONE;
      if (strcmp (val, "form-source") == 0)
	devmode->dmDefaultSource = DMBIN_FORMSOURCE;
      if (strcmp (val, "large-capacity") == 0)
	devmode->dmDefaultSource = DMBIN_LARGECAPACITY;
      if (strcmp (val, "large-format") == 0)
	devmode->dmDefaultSource = DMBIN_LARGEFMT;
      if (strcmp (val, "tractor") == 0)
	devmode->dmDefaultSource = DMBIN_TRACTOR;
      if (strcmp (val, "small-format") == 0)
	devmode->dmDefaultSource = DMBIN_SMALLFMT;
    }

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_RESOLUTION))
    {
      devmode->dmFields |= DM_PRINTQUALITY;
      devmode->dmPrintQuality = egg_printer_settings_get_resolution (settings);
    } 
  else if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_QUALITY))
    {
      devmode->dmFields |= DM_PRINTQUALITY;
      switch (egg_printer_settings_get_quality (settings))
	{
	case EGG_PRINT_QUALITY_LOW:
	  devmode->dmPrintQuality = DMRES_LOW;
	  break;
	case EGG_PRINT_QUALITY_DRAFT:
	  devmode->dmPrintQuality = DMRES_DRAFT;
	  break;
	default:
	case EGG_PRINT_QUALITY_NORMAL:
	  devmode->dmPrintQuality = DMRES_MEDIUM;
	  break;
	case EGG_PRINT_QUALITY_HIGH:
	  devmode->dmPrintQuality = DMRES_HIGH;
	  break;
	}
    }

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_USE_COLOR))
    {
      devmode->dmFields |= DM_COLOR;
      if (egg_printer_settings_get_use_color (settings))
	devmode->dmColor = DMCOLOR_COLOR;
      else
	devmode->dmColor = DMCOLOR_MONOCHROME;
    }

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_DUPLEX))
    {
      devmode->dmFields |= DM_DUPLEX;
      switch (egg_printer_settings_get_duplex (settings))
	{
	default:
	case EGG_PRINT_DUPLEX_SIMPLEX:
	  devmode->dmDuplex = DMDUP_SIMPLEX;
	  break;
	case EGG_PRINT_DUPLEX_HORIZONTAL:
	  devmode->dmDuplex = DMDUP_HORIZONTAL;
	  break;
	case EGG_PRINT_DUPLEX_VERTICAL:
	  devmode->dmDuplex = DMDUP_VERTICAL;
	  break;
	}
    }

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_COLLATE))
    {
      devmode->dmFields |= DM_COLLATE;
      if (egg_printer_settings_get_collate (settings))
	devmode->dmCollate = DMCOLLATE_TRUE;
      else
	devmode->dmCollate = DMCOLLATE_FALSE;
    }

  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_MEDIA_TYPE))
    {
      devmode->dmFields |= DM_MEDIATYPE;
      devmode->dmMediaType = DMMEDIA_STANDARD;
      
      val = egg_printer_settings_get_media_type (settings);
      if (strcmp (val, "transparency") == 0)
	devmode->dmMediaType = DMMEDIA_TRANSPARENCY;
      if (strcmp (val, "photographic-glossy") == 0)
	devmode->dmMediaType = DMMEDIA_GLOSSY;
    }
 
  if (egg_printer_settings_has_key (settings, EGG_PRINTER_SETTINGS_DITHER))
    {
      devmode->dmFields |= DM_DITHERTYPE;
      devmode->dmDitherType = DMDITHER_FINE;
      
      val = egg_printer_settings_get_dither (settings);
      if (strcmp (val, "none") == 0)
	devmode->dmDitherType = DMDITHER_NONE;
      if (strcmp (val, "coarse") == 0)
	devmode->dmDitherType = DMDITHER_COARSE;
      if (strcmp (val, "fine") == 0)
	devmode->dmDitherType = DMDITHER_FINE;
      if (strcmp (val, "lineart") == 0)
	devmode->dmDitherType = DMDITHER_LINEART;
      if (strcmp (val, "grayscale") == 0)
	devmode->dmDitherType = DMDITHER_GRAYSCALE;
      if (strcmp (val, "error-diffusion") == 0)
	devmode->dmDitherType = DMDITHER_ERRORDIFFUSION;
    }
  
  GlobalUnlock (printdlgex->hDevMode);

}

EggPrintOperationResult
egg_print_operation_platform_backend_run_dialog (EggPrintOperation *op,
						 GtkWindow *parent,
						 gboolean *do_print,
						 GError **error)
{
  HRESULT hResult;
  LPPRINTDLGEXW printdlgex = NULL;
  LPPRINTPAGERANGE page_ranges = NULL;
  HWND parentHWnd;
  GtkWidget *invisible = NULL;
  EggPrintOperationResult result;
  EggPrintOperationWin32 *op_win32;
  
  *do_print = FALSE;

  if (parent == NULL)
    {
      invisible = gtk_invisible_new ();
      parentHWnd = get_parent_hwnd (invisible);
    }
  else 
    parentHWnd = get_parent_hwnd (GTK_WIDGET (parent));

  printdlgex = (LPPRINTDLGEXW)GlobalAlloc (GPTR, sizeof (PRINTDLGEXW));
  if (!printdlgex)
    {
      result = EGG_PRINT_OPERATION_RESULT_ERROR;
      g_set_error (error,
		   EGG_PRINT_ERROR,
		   EGG_PRINT_ERROR_NOMEM,
		   _("Not enough free memory"));
      goto out;
    }      

  page_ranges = (LPPRINTPAGERANGE) GlobalAlloc (GPTR, 
						MAX_PAGE_RANGES * sizeof (PRINTPAGERANGE));
  if (!page_ranges) 
    {
      result = EGG_PRINT_OPERATION_RESULT_ERROR;
      g_set_error (error,
		   EGG_PRINT_ERROR,
		   EGG_PRINT_ERROR_NOMEM,
		   _("Not enough free memory"));
      goto out;
    }

  printdlgex->lStructSize = sizeof(PRINTDLGEX);
  printdlgex->hwndOwner = parentHWnd;
  printdlgex->hDevMode = NULL;
  printdlgex->hDevNames = NULL;
  printdlgex->hDC = NULL;
  printdlgex->Flags = PD_RETURNDC | PD_NOSELECTION;
  if (op->priv->current_page == -1)
    printdlgex->Flags |= PD_NOCURRENTPAGE;
  printdlgex->Flags2 = 0;
  printdlgex->ExclusionFlags = 0;
  printdlgex->nPageRanges = 0;
  printdlgex->nMaxPageRanges = MAX_PAGE_RANGES;
  printdlgex->lpPageRanges = page_ranges;
  printdlgex->nMinPage = 1;
  if (op->priv->nr_of_pages != -1)
    printdlgex->nMaxPage = op->priv->nr_of_pages;
  else
    printdlgex->nMaxPage = 10000;
  printdlgex->nCopies = 1;
  printdlgex->hInstance = 0;
  printdlgex->lpPrintTemplateName = NULL;
  printdlgex->lpCallback = NULL;
  printdlgex->nPropertyPages = 0;
  printdlgex->lphPropertyPages = NULL;
  printdlgex->nStartPage = START_PAGE_GENERAL;
  printdlgex->dwResultAction = 0;

  dialog_from_printer_settings (op, printdlgex);

  /* TODO: We should do this in a thread to avoid blocking the mainloop */
  hResult = PrintDlgExW(printdlgex);

  if (hResult != S_OK) 
    {
      result = EGG_PRINT_OPERATION_RESULT_ERROR;
      if (hResult == E_OUTOFMEMORY)
	g_set_error (error,
		     EGG_PRINT_ERROR,
		     EGG_PRINT_ERROR_NOMEM,
		     _("Not enough free memory"));
      else if (hResult == E_INVALIDARG)
	g_set_error (error,
		     EGG_PRINT_ERROR,
		     EGG_PRINT_ERROR_INTERNAL_ERROR,
		     _("Invalid argument to PrintDlgEx"));
      else if (hResult == E_POINTER)
	g_set_error (error,
		     EGG_PRINT_ERROR,
		     EGG_PRINT_ERROR_INTERNAL_ERROR,
		     _("Invalid pointer to PrintDlgEx"));
      else if (hResult == E_HANDLE)
	g_set_error (error,
		     EGG_PRINT_ERROR,
		     EGG_PRINT_ERROR_INTERNAL_ERROR,
		     _("Invalid handle to PrintDlgEx"));
      else /* E_FAIL */
	g_set_error (error,
		     EGG_PRINT_ERROR,
		     EGG_PRINT_ERROR_GENERAL,
		     _("Unspecified error"));
      goto out;
    }

  if (printdlgex->dwResultAction == PD_RESULT_PRINT ||
      printdlgex->dwResultAction == PD_RESULT_APPLY)
    {
      result = EGG_PRINT_OPERATION_RESULT_APPLY;
      dialog_to_printer_settings (op, printdlgex);
    }
  else
    result = EGG_PRINT_OPERATION_RESULT_CANCEL;
  
  if (printdlgex->dwResultAction == PD_RESULT_PRINT)
    {
      DOCINFOW docinfo;
      int job_id;

      *do_print = TRUE;

      op->priv->surface = cairo_win32_surface_create (printdlgex->hDC);
      op->priv->dpi_x = (double)GetDeviceCaps (printdlgex->hDC, LOGPIXELSX);
      op->priv->dpi_y = (double)GetDeviceCaps (printdlgex->hDC, LOGPIXELSY);

      memset( &docinfo, 0, sizeof (DOCINFOW));
      docinfo.cbSize = sizeof (DOCINFOW); 
      docinfo.lpszDocName = g_utf8_to_utf16 (op->priv->job_name, -1, NULL, NULL, NULL); 
      docinfo.lpszOutput = (LPCWSTR) NULL; 
      docinfo.lpszDatatype = (LPCWSTR) NULL; 
      docinfo.fwType = 0; 

      job_id = StartDocW(printdlgex->hDC, &docinfo); 
      g_free ((void *)docinfo.lpszDocName);
      if (job_id <= 0) 
	{ 
	  result = EGG_PRINT_OPERATION_RESULT_ERROR;
	  g_set_error (error,
		       EGG_PRINT_ERROR,
		       EGG_PRINT_ERROR_GENERAL,
		     _("Error from StartDoc"));
	  *do_print = FALSE;
	  cairo_surface_destroy (op->priv->surface);
	  op->priv->surface = NULL;
	  goto out; 
	} 

      op_win32 = g_new (EggPrintOperationWin32, 1);
      op->priv->platform_data = op_win32;
      op_win32->hdc = printdlgex->hDC;
      op_win32->devmode = printdlgex->hDevMode;
      
      op->priv->manual_num_copies = printdlgex->nCopies;
      op->priv->manual_collation = (printdlgex->Flags & PD_COLLATE) != 0;
    }

  op->priv->start_page = win32_start_page;
  op->priv->end_page = win32_end_page;
  op->priv->end_run = win32_end_run;
  
  out:
  if (!result && printdlgex && printdlgex->hDevMode != NULL) 
    GlobalFree(printdlgex->hDevMode); 

  if (printdlgex && printdlgex->hDevNames != NULL) 
    GlobalFree(printdlgex->hDevNames); 

  if (page_ranges)
    GlobalFree (page_ranges);

  if (printdlgex)
    GlobalFree (printdlgex);

  if (invisible)
    gtk_widget_destroy (invisible);

  return result;
}

