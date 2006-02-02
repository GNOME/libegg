/* EggRegex -- regular expression API wrapper around PCRE.
 * Copyright (C) 1999 Scott Wimer
 * Copyright (C) 2004 Matthias Clasen
 *
 * This is basically an ease of user wrapper around the functionality of
 * PCRE.
 *
 * With this library, we are, hopefully, drastically reducing the code
 * complexity necessary by making use of a more complex and detailed
 * data structure to store the regex info.  I am hoping to have a regex
 * interface that is almost as easy to use as Perl's.  <fingers crossed>
 *
 * Author: Scott Wimer <scottw@cgibuilder.com>
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * This library is free software, you can distribute it or modify it
 * under the following terms:
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
#ifndef __EGGREGEX_H__
#define __EGGREGEX_H__

#include <glib/gtypes.h>
#include <glib/gquark.h>
#include <glib/gerror.h>
#include <glib/gstring.h>

G_BEGIN_DECLS

typedef enum
{
  EGG_REGEX_ERROR_COMPILE,
  EGG_REGEX_ERROR_OPTIMIZE,
  EGG_REGEX_ERROR_REPLACE
} EggRegexError;

#define EGG_REGEX_ERROR egg_regex_error_quark ()

GQuark egg_regex_error_quark (void);

typedef enum
{
  EGG_REGEX_CASELESS          = 1 << 0,
  EGG_REGEX_MULTILINE         = 1 << 1,
  EGG_REGEX_DOTALL            = 1 << 2,
  EGG_REGEX_EXTENDED          = 1 << 3,
  EGG_REGEX_ANCHORED          = 1 << 4,
  EGG_REGEX_DOLLAR_ENDONLY    = 1 << 5,
  EGG_REGEX_UNGREEDY          = 1 << 9,
  EGG_REGEX_NO_AUTO_CAPTURE   = 1 << 12
} EggRegexCompileFlags;

typedef enum
{
  EGG_REGEX_MATCH_ANCHORED    = 1 << 4,
  EGG_REGEX_MATCH_NOTBOL      = 1 << 7,
  EGG_REGEX_MATCH_NOTEOL      = 1 << 8,
  EGG_REGEX_MATCH_NOTEMPTY    = 1 << 10
} EggRegexMatchFlags;

typedef struct _EggRegex  EggRegex;

typedef gboolean (*EggRegexEvalCallback) (EggRegex*, const gchar*, GString*, gpointer);

/* Really quick outline of features... functions are preceded by 'egg_regex_'
 *   new         - compile a pattern and put it in a egg_regex structure
 *   free        - free up the memory used by the egg_regex structure
 *   clear       - clear out the structure to match against a new string
 *   optimize    - study the pattern to make matching more efficient
 *   match       - try matching a pattern in the string
 *   match_next  - try matching pattern again in the string
 *   fetch       - fetch a particular matching sub pattern
 *   fetch_all   - get all of the matching sub patterns
 *   split       - split the string on a regex
 *   split_next  - for using split as an iterator of sorts
 *   replace     - replace occurances of a pattern with some text
 */

EggRegex  *egg_regex_new          (const gchar           *pattern,
				   EggRegexCompileFlags   compile_options,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
void       egg_regex_optimize     (EggRegex              *regex,
				   GError               **error);
void       egg_regex_free         (EggRegex              *regex);
void       egg_regex_clear        (EggRegex              *regex);
gint       egg_regex_match        (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   EggRegexMatchFlags     match_options);
gint       egg_regex_match_next   (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   EggRegexMatchFlags     match_options);
gchar     *egg_regex_fetch        (EggRegex              *regex,
				   const gchar           *string,
				   gint                   match_num);
void       egg_regex_fetch_pos    (EggRegex              *regex,
				   const gchar           *string,
				   gint                   match_num,
				   gint                  *start_pos,
				   gint                  *end_pos);
gchar     *egg_regex_fetch_named  (EggRegex              *regex,
				   const gchar           *string,
				   const gchar           *name);
gchar    **egg_regex_fetch_all    (EggRegex              *regex,
				   const gchar           *string);
gchar    **egg_regex_split        (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   EggRegexMatchFlags     match_options,
				   gint                   max_pieces);
gchar     *egg_regex_split_next   (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   EggRegexMatchFlags     match_options);
gchar     *egg_regex_replace      (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   const gchar           *replacement,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gchar     *egg_regex_replace_eval (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   EggRegexEvalCallback   eval,
				   gpointer               user_data,
				   EggRegexMatchFlags     match_options);



G_END_DECLS


#endif  /*  __EGGREGEX_H__ */
