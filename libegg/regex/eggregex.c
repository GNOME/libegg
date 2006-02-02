/* EggRegex -- regular expression API wrapper around PCRE.
 * Copyright (C) 1999, 2000 Scott Wimer
 * Copyright (C) 2004 Matthias Clasen <mclasen@redhat.com>
 *
 * This is basically an ease of user wrapper around the functionality of
 * PCRE.
 *
 * With this library, we are, hopefully, drastically reducing the code
 * complexity necessary by making use of a more complex and detailed
 * data structure to store the regex info.  I am hoping to have a regex
 * interface that is almost as easy to use as Perl's.  <fingers crossed>
 *
 * Author: Scott Wimer <scottw@cylant.com>
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * This library is free software, you can distribute it or modify it
 * under any of the following terms:
 *  1) The GNU General Public License (GPL)
 *  2) The GNU Library General Public License (LGPL)
 *  3) The Perl Artistic license (Artistic)
 *  4) The BSD license (BSD)
 *
 * In short, you can use this library in any code you desire, so long as
 * the Copyright notice above remains intact.  If you do make changes to
 * it, I would appreciate that you let me know so I can improve this 
 * library for everybody, but I'm not gonna force you to.
 * 
 * Please note that this library is just a wrapper around Philip Hazel's
 * PCRE library.  Please see the file 'LICENSE' in your PCRE distribution.
 * And, if you live in England, please send him a pint of good beer, his
 * library is great.
 *
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "eggregex.h"
#include <glib.h>
#include "pcre/pcre.h"

/* FIXME when this is in glib */
#define _(s) s

struct _EggRegex
{
  gchar *pattern;       /* the pattern */
  pcre *regex;		/* compiled form of the pattern */
  pcre_extra *extra;	/* data stored when egg_regex_optimize() is used */
  gint matches;		/* number of matching sub patterns */
  gint pos;		/* position in the string where last match left off */
  gint *offsets;	/* array of offsets paired 0,1 ; 2,3 ; 3,4 etc */
  gint n_offsets;	/* number of offsets */
  EggRegexCompileFlags compile_opts;	/* options used at compile time on the pattern */
  EggRegexMatchFlags match_opts;	/* options used at match time on the regex */
  gint string_len;	/* length of the string last used against */
  GSList *delims;	/* delimiter sub strings from split next */
};

GQuark
egg_regex_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("g-regex-error-quark");

  return error_quark;
}

/** 
 * egg_regex_new:
 * @pattern: the regular expression
 * @compile_options: compile options for the regular expression
 * @match_options: match options for the regular expression
 * @error: return location for a #GError
 * 
 * Compiles the regular expression to an internal form, and does the initial
 * setup of the #EggRegex structure.  
 * 
 * Returns: a #EggRegex structure
 */
EggRegex *
egg_regex_new (const gchar         *pattern, 
 	     EggRegexCompileFlags   compile_options,
	     EggRegexMatchFlags     match_options,
	     GError             **error)
{
  EggRegex *regex = g_new0 (EggRegex, 1);
  const gchar *errmsg;
  gint erroffset;
  gint capture_count;
  
  /* preset the parts of gregex that need to be set, regardless of the
   * type of match that will be checked */
  regex->pattern = g_strdup (pattern);
  regex->extra = NULL;
  regex->pos = 0;
  regex->string_len = -1;	/* not set yet */

  /* set the options */
  regex->compile_opts = compile_options | PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
  regex->match_opts = match_options | PCRE_NO_UTF8_CHECK;

  /* compile the pattern */
  regex->regex = _pcre_compile (pattern, regex->compile_opts,
				 &errmsg, &erroffset, NULL);

  /* if the compilation failed, set the error member and return 
   * immediately */
  if (regex->regex == NULL)
    {
      GError *tmp_error = g_error_new (EGG_REGEX_ERROR, 
				       EGG_REGEX_ERROR_COMPILE,
				       _("Error while compiling regular "
					 "expression %s at char %d: %s"),
				       pattern, erroffset, errmsg);
      g_propagate_error (error, tmp_error);

      return regex;
    }

  /* otherwise, find out how many sub patterns exist in this pattern,
   * and setup the offsets array and n_offsets accordingly */
  _pcre_fullinfo (regex->regex, regex->extra, 
		  PCRE_INFO_CAPTURECOUNT, &capture_count);
  regex->n_offsets = (capture_count + 1) * 3;
  regex->offsets = g_new0 (gint, regex->n_offsets);

  return regex;
}


/**
 * egg_regex_free:
 * @regex: a #EggRegex structure from egg_regex_new()
 *
 * Frees all the memory associated with the regex structure.
 */
void
egg_regex_free (EggRegex *regex)
{
  g_free (regex->pattern);
  g_slist_free (regex->delims);
  g_free (regex->offsets);
  if (regex->regex != NULL)
    g_free (regex->regex);
  if (regex->extra != NULL)
    g_free (regex->extra);
  g_free (regex);
}


/**
 * egg_regex_clear:
 * @regex: a #EggRegex structure
 *
 * Clears out the members of @regex that are holding information about the
 * last set of matches for this pattern.  egg_regex_clear() needs to be
 * called between uses of egg_regex_match() or egg_regex_match_next() against
 * new target strings. 
 */
void
egg_regex_clear (EggRegex *regex)
{
  regex->matches = -1;
  regex->string_len = -1;
  regex->pos = 0;

  /* if the pattern was used with egg_regex_split_next(), it may have
   * delimiter offsets stored.  Free up those guys as well. */
  if (regex->delims != NULL)
    g_slist_free (regex->delims);
}

/**
 * egg_regex_optimize:
 * @regex: a #EggRegex structure
 * @error: return location for a #GError
 *
 * If the pattern will be used many times, then it may be worth the
 * effort to optimize it to improve the speed of matches.
 */
void
egg_regex_optimize (EggRegex  *regex,
		  GError **error)
{
  const gchar *errmsg;

  regex->extra = _pcre_study (regex->regex, 0, &errmsg);

  if (errmsg)
    {
      GError *tmp_error = g_error_new (EGG_REGEX_ERROR,
				       EGG_REGEX_ERROR_OPTIMIZE, 
				       _("Error while optimizing "
					 "regular expression %s: %s"),
				       regex->pattern,
				       errmsg);
      g_propagate_error (error, tmp_error);
    }
}

/**
 * egg_regex_match:
 * @regex: a #EggRegex structure from egg_regex_new()
 * @string: the string to scan for matches
 * @string_len: the length of @string, or -1 to use strlen()
 * @match_options:  match options
 *
 * Scans for a match in string for the pattern in @regex. The starting index
 * of the match goes into the pos member of the @regex struct. The indexes
 * of the full match, and all matches get stored off in the offsets array.
 *
 * The @match_options are combined with the match options specified when the 
 * @regex structure was created, letting you have more flexibility in reusing
 * #EggRegex structures.
 *
 * Returns:  Number of matched substrings + 1, or 1 if the pattern has no
 *           substrings in it.  Returns #GREGEX_NOMATCH if the pattern
 *           did not match.
 */
gint 
egg_regex_match (EggRegex          *regex, 
	       const gchar     *string, 
	       gssize           string_len,
	       EggRegexMatchFlags match_options)
{
  if (string_len < 0)
    string_len = strlen (string);

  regex->string_len = string_len;

  /* perform the match */
  regex->matches = _pcre_exec (regex->regex, regex->extra, 
			       string, regex->string_len, 0,
			       regex->match_opts | match_options,
			       regex->offsets, regex->n_offsets);

  /* if the regex matched, set regex->pos to the character past the 
   * end of the match.
   */
  if (regex->matches > 0)
    regex->pos = regex->offsets[1];

  return regex->matches;	/* return what pcre_exec() returned */
}


/**
 * egg_regex_match_next:
 * @regex: a #EggRegex structure 
 * @string: the string to scan for matches
 * @string_len: the length of @string, or -1 to use strlen()
 * @match_options: the match options
 *
 * Scans for the next match in @string of the pattern in @regex.  The starting 
 * index of the match goes into the pos member of the @regex struct.  The 
 * indexes of the full match, and all matches get stored off in the offsets 
 * array.  The match options are ored with the match options set when
 * the @regex was created.
 *
 * You have to call egg_regex_clear() to reuse the same pattern on a new string.
 * This is especially true for use with egg_regex_match_next().
 *
 * Returns:  Number of matched substrings + 1, or 1 if the pattern has no
 *           substrings in it.  Returns #GREGEX_NOMATCH if the pattern
 *           did not match.
 */
gint 
egg_regex_match_next (EggRegex          *regex, 
		    const gchar     *string, 
		    gssize           string_len,
		    EggRegexMatchFlags match_options)
{
  /* if this regex hasn't been used on this string before, then we
   * need to calculate the length of the string, and set pos to the
   * start of it.  
   * Knowing if this regex has been used on this string is a bit of 
   * a challenge.  For now, we require the user to call egg_regex_clear()
   * in between usages on a new string.  Not perfect, but not such a
   * bad solution either.
   */
  if (regex->string_len == -1)
    {
      if (string_len < 0)
	string_len = strlen (string);
      
      regex->string_len = string_len;
    }

  /* perform the match */
  regex->matches = _pcre_exec (regex->regex, regex->extra,
			       string + regex->pos, 
			       regex->string_len - regex->pos,
			       0, regex->match_opts | match_options,
			       regex->offsets, regex->n_offsets);

  /* if the regex matched, adjust the offsets array to take into account
   * the fact that the string they're out of is shorter than the string
   * that the caller passed us, by regex->pos to be exact.
   * Then, update regex->pos to take into account the new starting point.
   */
  if (regex->matches > 0)
    {
      gint i, pieces;
      pieces = (regex->matches * 2) - 1;

      for (i = 0; i <= pieces; i++)
	regex->offsets[i] += regex->pos;

      regex->pos = regex->offsets[1];
    }

  return regex->matches;
}


/**
 * egg_regex_fetch:
 * @regex: #EggRegex structure used in last match
 * @string: the string on which the last match was made
 * @match_num: number of the sub expression
 *
 * Retrieves the text matching the @match_num<!-- -->'th capturing parentheses.
 * 0 is the full text of the match, 1 is the first paren set, 2 the second,
 * and so on.
 *
 * Returns: The matched substring.  You have to free it yourself.
 */
gchar *
egg_regex_fetch (EggRegex      *regex, 
	       const gchar *string,
	       gint         match_num)
{
  gchar *match;

  /* make sure the sub expression number they're requesting is less than
   * the total number of sub expressions that were matched. */
  if (match_num >= regex->matches)
    return NULL;

  _pcre_get_substring (string, regex->offsets, regex->matches, 
		       match_num, (const char **)&match);

  return match;
}

/**
 * egg_regex_fetch_pos:
 * @regex: #EggRegex structure used in last match
 * @string: the string on which the last match was made
 * @match_num: number of the sub expression
 * @start_pos: pointer to location where to store the start position
 * @end_pos: pointer to location where to store the end position
 *
 * Retrieves the position of the @match_num<!-- -->'th capturing parentheses.
 * 0 is the full text of the match, 1 is the first paren set, 2 the second,
 * and so on.
 */
void
egg_regex_fetch_pos (EggRegex      *regex, 
		     const gchar *string,
		     gint         match_num,
		     gint        *start_pos,
		     gint        *end_pos)
{
  /* make sure the sub expression number they're requesting is less than
   * the total number of sub expressions that were matched. */
  if (match_num >= regex->matches)
    return;

  if (start_pos)
    *start_pos = regex->offsets[2 * match_num];

  if (end_pos)
    *end_pos = regex->offsets[2 * match_num + 1];
}

/**
 * egg_regex_fetch_named:
 * @regex: #EggRegex structure used in last match
 * @string: the string on which the last match was made
 * @name: name of the subexpression
 *
 * Retrieves the text matching the capturing parentheses named @name.
 *
 * Returns: The matched substring.  You have to free it yourself.
 */
gchar *
egg_regex_fetch_named (EggRegex      *regex, 
		     const gchar *string,
		     const gchar *name)
{
  gchar *match;

  _pcre_get_named_substring (regex->regex, 
			     string, regex->offsets, regex->matches, 
			     name, (const char **)&match);

  return match;
}

/**
 * egg_regex_fetch_all:
 * @regex: a #EggRegex structure
 * @string: the string on which the last match was made
 *
 * Bundles up pointers to each of the matching substrings from a match 
 * and stores then in an array of gchar pointers.
 *
 * Returns: a %NULL-terminated array of gchar * pointers. It must be freed using
 * g_strfreev(). If the memory can't be allocated, returns %NULL.
 */
gchar **
egg_regex_fetch_all (EggRegex      *regex,
		   const gchar *string)
{
  gchar **listptr = NULL; /* the list pcre_get_substring_list() will fill */
  gchar **result;

  if (regex->matches < 0)
    return NULL;
  
  _pcre_get_substring_list (string, regex->offsets, 
			    regex->matches, (const char ***)&listptr);

  if (listptr)
    {
      /* PCRE returns a single block of memory which
       * isn't suitable for g_strfreev().
       */
      result = g_strdupv (listptr);
      g_free (listptr);
    }
  else 
    result = NULL;

  return result;
}


/**
 * egg_regex_split:
 * @regex:  a #EggRegex structure
 * @string:  the string to split with the pattern
 * @string_len: the length of @string, or -1 to use strlen()
 * @match_options:  match time option flags
 * @max_pieces:  maximum number of pieces to split the string into, 
 *    or 0 for no limit
 *
 * Breaks the string on the pattern, and returns an array of the pieces.  
 *
 * Returns: a %NULL-terminated gchar ** array. Free it using g_strfreev().
 **/
gchar **
egg_regex_split (EggRegex           *regex, 
	       const gchar      *string, 
	       gssize            string_len,
	       EggRegexMatchFlags  match_options,
	       gint              max_pieces)
{
  gchar **string_list;		/* The array of char **s worked on */
  gint pos;
  gint match_ret;
  gint pieces;
  gint start_pos;
  gchar *piece;
  GList *list, *last;

  start_pos = 0;
  pieces = 0;
  list = NULL;
  while (TRUE)
    {
      match_ret = egg_regex_match_next (regex, string, string_len, match_options);
      if ((match_ret > 0) && ((max_pieces == 0) || (pieces < max_pieces)))
	{
	  piece = g_strndup (string + start_pos, regex->offsets[0] - start_pos);
	  list = g_list_prepend (list, piece);

	  /* if there were substrings, these need to get added to the
	   * list as well */
	  if (match_ret > 1)
	    {
	      int i;
	      for (i = 1; i < match_ret; i++)
		list = g_list_prepend (list, egg_regex_fetch (regex, string, i));
	    }

	  start_pos = regex->pos;	/* move start_pos to end of match */
	  pieces++;
	}
      else	 /* if there was no match, copy to end of string, and break */
	{
	  piece = g_strndup (string + start_pos, regex->string_len - start_pos);
	  list = g_list_prepend (list, piece);
	  break;
	}
    }

  string_list = (gchar **) g_malloc (sizeof (gchar *) * (g_list_length (list) + 1));
  pos = 0;
  for (last = g_list_last (list); last; last = last->prev)
    string_list[pos++] = last->data;
  string_list[pos] = 0;

  g_list_free (list);
  return string_list;
}


/**
 * egg_regex_split_next:
 * @pattern:  gchar pointer to the pattern
 * @string:  the string to split on pattern
 * @string_len: the length of @string, or -1 to use strlen()
 * @match_options:  match time options for the regex
 *
 * egg_regex_split_next() breaks the string on pattern, and returns the  
 * pieces, one per call.  If the pattern contains capturing parentheses, 
 * then the text for each of the substrings will also be returned.
 * If the pattern does not match anywhere in the string, then the whole 
 * string is returned as the first piece.
 *
 * Returns:  a gchar * to the next piece of the string
 */
gchar *
egg_regex_split_next (EggRegex      *regex, 
		    const gchar *string, 
		    gssize       string_len, 
		    EggRegexMatchFlags match_options)
{
  gint start_pos = regex->pos;
  gchar *piece = NULL;
  gint match_ret;

  /* if there are delimiter substrings stored, return those one at a
   * time.  
   */
  if (regex->delims != NULL)
    {
      piece = regex->delims->data;
      regex->delims = g_slist_remove (regex->delims, piece);
      return piece;
    }

  /* otherwise...
   * use egg_regex_match_next() to find the next occurance of the pattern
   * in the string.  We use start_pos to keep track of where the stuff
   * up to the current match starts.  Copy that piece of the string off
   * and append it to the buffer using strncpy.  We have to NUL term the
   * piece we copied off before returning it.
   */
  match_ret = egg_regex_match_next (regex, string, string_len, match_options);
  if (match_ret > 0)
    {
      piece = g_strndup (string + start_pos, regex->offsets[0] - start_pos);

      /* if there were substrings, these need to get added to the
       * list of delims */
      if (match_ret > 1)
	{
	  gint i;
	  for (i = 1; i < match_ret; i++)
	    regex->delims = g_slist_append (regex->delims,
					     egg_regex_fetch (regex, string, i));
	}
    }
  else		/* if there was no match, copy to end of string */
    piece = g_strndup (string + start_pos, regex->string_len - start_pos);

  return piece;
}

static gboolean
copy_replacement (EggRegex      *regex,
		  const gchar *string,
		  GString     *result,
	          gpointer     data)
{
  g_string_append (result, (gchar *)data);

  return FALSE;
}

enum
{
  REPL_TYPE_STRING,
  REPL_TYPE_CHARACTER,
  REPL_TYPE_SYMBOLIC_REFERENCE,
  REPL_TYPE_NUMERIC_REFERENCE
}; 

typedef struct 
{
  gchar *text;   
  gint   type;   
  gint   num;
  gchar  c;
} InterpolationData;

static void
free_interpolation_data (InterpolationData *data)
{
  g_free (data->text);
  g_free (data);
}

static const gchar *
expand_escape (const gchar        *replacement,
	       const gchar        *p, 
	       InterpolationData  *data,
	       GError            **error)
{
  const gchar *q, *r;
  gint x, d, h, i;
  gchar *error_detail;
  gint base = 0;
  GError *tmp_error = NULL;

  p++;
  switch (*p)
    {
    case 't':
      p++;
      data->c = '\t';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'n':
      p++;
      data->c = '\n';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'v':
      p++;
      data->c = '\v';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'r':
      p++;
      data->c = '\r';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'f':
      p++;
      data->c = '\f';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'a':
      p++;
      data->c = '\a';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'b':
      p++;
      data->c = '\b';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case '\\':
      p++;
      data->c = '\\';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'x':
      p++;
      x = 0;
      if (*p == '{')
	{
	  p++;
	  do 
	    {
	      h = g_ascii_xdigit_value (*p);
	      if (h < 0)
		{
		  error_detail = _("hexadecimal digit or '}' expected");
		  goto error;
		}
	      x = x * 16 + h;
	      p++;
	    }
	  while (*p != '}');
	  p++;
	}
      else
	{
	  for (i = 0; i < 2; i++)
	    {
	      h = g_ascii_xdigit_value (*p);
	      if (h < 0)
		{
		  error_detail = _("hexadecimal digit expected");
		  goto error;
		}
	      x = x * 16 + h;
	      p++;
	    }
	}
      data->type = REPL_TYPE_STRING;
      data->text = g_new0 (gchar, 8);
      g_unichar_to_utf8 (x, data->text);
      break;
    case 'l':
    case 'u':
    case 'L':
    case 'U':
    case 'E':
    case 'Q':
    case 'G':
      error_detail = _("escape sequence not allowed");
      goto error;
    case 'g':
      p++;
      if (*p != '<')
	{
	  error_detail = _("missing '<' in symbolic reference");
	  goto error;
	}
      q = p + 1;
      do 
	{
	  p++;
	  if (!*p)
	    {
	      error_detail = _("unfinished symbolic reference");
	      goto error;
	    }
	}
      while (*p != '>');
      if (p - q == 0)
	{
	  error_detail = _("zero-length symbolic reference");
	  goto error;
	}
      if (g_ascii_isdigit (*q))
	{
	  x = 0;
	  do 
	    {
	      h = g_ascii_digit_value (*q);
	      if (h < 0)
		{
		  error_detail = _("digit expected");
		  p = q;
		  goto error;
		}
	      x = x * 10 + h;
	      q++;
	    }
	  while (q != p);
	  data->num = x;
	  data->type = REPL_TYPE_NUMERIC_REFERENCE;
	}
      else
	{
	  r = q;
	  do 
	    {
	      if (!g_ascii_isalnum (*r))
		{
		  error_detail = _("illegal symbolic reference");
		  p = r;
		  goto error;
		}
	      r++;
	    }
	  while (r != p);
	  data->text = g_strndup (q, p - q);
	  data->type = REPL_TYPE_SYMBOLIC_REFERENCE;
	}
      p++;
      break;
    case '0':
      base = 8;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      x = 0;
      d = 0;
      for (i = 0; i < 3; i++)
	{
	  h = g_ascii_digit_value (*p);
	  if (h < 0) 
	    break;
	  if (h > 7)
	    {
	      if (base == 8)
		break;
	      else 
		base = 10;
	    }
	  if (i == 2 && base == 10)
	    break;
	  x = x * 8 + h;
	  d = d * 10 + h;
	  p++;
	}
      if (base == 8 || i == 3)
	{
	  data->type = REPL_TYPE_STRING;
	  data->text = g_new0 (gchar, 8);
	  g_unichar_to_utf8 (x, data->text);
	}
      else
	{
	  data->type = REPL_TYPE_NUMERIC_REFERENCE;
	  data->num = d;
	}
      break;
    case 0:
      error_detail = _("stray final '\\'");
      goto error;
      break;
    default:
      data->type = REPL_TYPE_STRING;
      data->text = g_new0 (gchar, 8);
      g_unichar_to_utf8 (g_utf8_get_char (p), data->text);
      p = g_utf8_next_char (p);
    }

  return p;

 error:
  tmp_error = g_error_new (EGG_REGEX_ERROR, 
			   EGG_REGEX_ERROR_REPLACE,
			   _("Error while parsing replacement "
			     "text \"%s\" at char %d: %s"),
			   replacement, 
			   p - replacement,
			   error_detail);
  g_propagate_error (error, tmp_error);

  return NULL;
}

static GList *
split_replacement (const gchar  *replacement,
		   GError      **error)
{
  GList *list = NULL;
  InterpolationData *data;
  const gchar *p, *start;
  
  start = p = replacement; 
  while (*p)
    {
      if (*p == '\\')
	{
	  data = g_new0 (InterpolationData, 1);
	  start = p = expand_escape (replacement, p, data, error);
	  if (*error)
	    {
	      g_list_foreach (list, (GFunc)free_interpolation_data, NULL);
	      g_list_free (list);

	      return NULL;
	    }
	  list = g_list_prepend (list, data);
	}
      else
	{
	  p++;
	  if (*p == '\\' || *p == '\0')
	    {
	      if (p - start > 0)
		{
		  data = g_new0 (InterpolationData, 1);
		  data->text = g_strndup (start, p - start);
		  data->type = REPL_TYPE_STRING;
		  list = g_list_prepend (list, data);
		}
	    }
	}
    }

  return g_list_reverse (list);
}

static gboolean
interpolate_replacement (EggRegex      *regex,
			 const gchar *string,
			 GString     *result,
			 gpointer     data)
{
  GList *list;
  InterpolationData *idata;
  gchar *match;

  for (list = data; list; list = list->next)
    {
      idata = list->data;
      switch (idata->type)
	{
	case REPL_TYPE_STRING:
	  g_string_append (result, idata->text);
	  break;
	case REPL_TYPE_CHARACTER:
	  g_string_append_c (result, idata->c);
	  break;
	case REPL_TYPE_NUMERIC_REFERENCE:
	  match = egg_regex_fetch (regex, string, idata->num);
	  if (match) 
	    {
	      g_string_append (result, match);
	      g_free (match);
	    }
	  break;
	case REPL_TYPE_SYMBOLIC_REFERENCE:
	  match = egg_regex_fetch_named (regex, string, idata->text);
	  if (match) 
	    {
	      g_string_append (result, match);
	      g_free (match);
	    }
	  break;
	}
    }

  return FALSE;  
}

/**
 * egg_regex_replace:
 * @regex:  a #EggRegex structure
 * @string:  the string to perform matches against
 * @string_len: the length of @string, or -1 to use strlen()
 * @replacement:  text to replace each match with
 * @match_options:  options for the match
 *
 * Replaces all occurances of the pattern in @regex with the 
 * replacement text. Backreferences of the form '\number' or '\g<number>' 
 * in the replacement text are interpolated by the number-th captured 
 * subexpression of the match, '\g<name>' refers to the captured subexpression
 * with the given name. '\0' refers to the complete match. To include a 
 * literal '\' in the replacement, write '\\'.
 *
 * Returns: a newly allocated string containing the replacements.
 */
gchar *
egg_regex_replace (EggRegex            *regex, 
		 const gchar       *string, 
		 gssize             string_len,
		 const gchar       *replacement,
		 EggRegexMatchFlags   match_options,
		 GError           **error)
{
  gchar *result;
  GList *list;

  list = split_replacement (replacement, error);
  result = egg_regex_replace_eval (regex, 
				 string, string_len,
				 interpolate_replacement,
				 (gpointer)list,
				 match_options);
  g_list_foreach (list, (GFunc)free_interpolation_data, NULL);
  g_list_free (list);
  
  return result;
}


/**
 * egg_regex_replace_eval:
 * @gregex:  a #EggRegex structure
 * @string:  string to perform matches against
 * @string_len: the length of @string, or -1 to use strlen()
 * @eval: a function to call for each match
 * @match_options:  Options for the match
 *
 * Replaces occurances of the pattern in regex with
 * the output of @eval for that occurance.
 *
 * Returns: a newly allocated string containing the replacements.
 */
gchar *
egg_regex_replace_eval (EggRegex             *regex, 
		      const gchar        *string,
		      gssize              string_len,
		      EggRegexEvalCallback  eval,
		      gpointer            user_data, 
		      EggRegexMatchFlags match_options)
{
  GString *result;
  gint str_pos = 0;
  gboolean done = FALSE;

  if (string_len < 0)
    string_len = strlen (string);

  /* clear out the regex for reuse, just in case */
  egg_regex_clear (regex);

  result = g_string_sized_new (string_len);

  /* run down the string making matches. */
  while (egg_regex_match_next (regex, string, string_len, match_options) > 0 && !done)
    {
      g_string_append_len (result, 
			   string + str_pos, 
			   regex->offsets[0] - str_pos);
      done = (*eval) (regex, string, result, user_data);
      str_pos = regex->offsets[1];
    }
  
  g_string_append_len (result, string + str_pos, string_len - str_pos);

  return g_string_free (result, FALSE);
}

