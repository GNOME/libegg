/* GTK - The GIMP Toolkit
 * eggrecentfilter.h - Filter object for recently used resources
 * Copyright (C) 2005, Emmanuele Bassi
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

#include "config.h"

#include <string.h>
#include <gtk/gtkobject.h>

#ifdef G_OS_UNIX
#define XDG_PREFIX _gtk_xdg
#include "xdgmime/xdgmime.h"
#endif

#include "eggrecentfilter.h"

typedef struct _EggRecentFilterClass EggRecentFilterClass;
typedef struct _FilterRule FilterRule;

typedef enum {
  FILTER_RULE_URI,
  FILTER_RULE_DISPLAY_NAME,
  FILTER_RULE_MIME_TYPE,
  FILTER_RULE_APPLICATION,
  FILTER_RULE_AGE,
  FILTER_RULE_GROUP,
  FILTER_RULE_CUSTOM,
} FilterRuleType;

struct _EggRecentFilter
{
  GtkObject parent_instance;
  
  gchar *name;
  GSList *rules;
  
  EggRecentFilterFlags needed;
};

struct _EggRecentFilterClass
{
  GtkObjectClass parent_class;
};

struct _FilterRule
{
  FilterRuleType type;
  EggRecentFilterFlags needed;
  
  union {
    gchar *uri;
    gchar *pattern;
    gchar *mime_type;
    gchar *application;
    gchar *group;
    gint age;
    struct {
      EggRecentFilterFunc func;
      gpointer data;
      GDestroyNotify data_destroy;
    } custom;
  } u;
};

G_DEFINE_TYPE (EggRecentFilter, egg_recent_filter, GTK_TYPE_OBJECT);


/* this should go away, and be replaced by including "gtkprivate.h" */
#ifndef _gtk_fnmatch
gboolean _gtk_fnmatch (const gchar *pattern,
		       const gchar *string,
		       gboolean     no_leading_period);
#endif


static void
filter_rule_free (FilterRule *rule)
{
  switch (rule->type)
    {
    case FILTER_RULE_MIME_TYPE:
      g_free (rule->u.mime_type);
      break;
    case FILTER_RULE_URI:
      g_free (rule->u.uri);
      break;
    case FILTER_RULE_DISPLAY_NAME:
      g_free (rule->u.pattern);
      break;
    case FILTER_RULE_AGE:
      break;
    case FILTER_RULE_APPLICATION:
      g_free (rule->u.application);
      break;
    case FILTER_RULE_GROUP:
      g_free (rule->u.group);
      break;
    case FILTER_RULE_CUSTOM:
      if (rule->u.custom.data_destroy)
        rule->u.custom.data_destroy (rule->u.custom.data);
      break;
    default:
      g_assert_not_reached ();
      break;
    }
  
  g_free (rule);
}

static void
egg_recent_filter_finalize (GObject *object)
{
  EggRecentFilter *filter = EGG_RECENT_FILTER (object);
  
  if (filter->name)
    g_free (filter->name);
  
  if (filter->rules)
    {
      g_slist_foreach (filter->rules,
      		       (GFunc) filter_rule_free,
      		       NULL);
      g_slist_free (filter->rules);
    }
  
  G_OBJECT_CLASS (egg_recent_filter_parent_class)->finalize (object);
}

static void
egg_recent_filter_class_init (EggRecentFilterClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  gobject_class->finalize = egg_recent_filter_finalize;
}

static void
egg_recent_filter_init (EggRecentFilter *filter)
{

}

/*
 * Public API
 */
 
/**
 * egg_recent_filter_new:
 *
 * Creates a new #EggRecentFilter with no rules added to it.
 * Such filter does not accept any recently used resources, so is not
 * particularly useful until you add rules with
 * egg_recent_filter_add_pattern(), egg_recent_filter_add_mime_type(),
 * egg_recent_filter_add_application(), egg_recent_filter_add_age().
 * To create a filter that accepts any recently used resource, use:
 *
 * <informalexample><programlisting>
 * EggRecentFilter *filter = egg_recent_filter_new (<!-- -->);
 * egg_recent_filter_add_pattern (filter, "*");
 * </programlisting></informalexample>
 *
 * Return value: a new #EggRecentFilter
 *
 * Since: 2.10
 */
EggRecentFilter *
egg_recent_filter_new (void)
{
  return g_object_new (EGG_TYPE_RECENT_FILTER, NULL);
}

/**
 * egg_recent_filter_set_name:
 * @filter: a #EggRecentFilter
 * @name: then human readable name of @filter
 *
 * Sets the human-readable name of the filter; this is the string
 * that will be displayed in the recently used resources selector
 * user interface if there is a selectable list of filters.
 *
 * Since: 2.10
 */
void
egg_recent_filter_set_name (EggRecentFilter *filter,
			    const gchar     *name)
{
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  
  if (filter->name)
    g_free (filter->name);
  
  if (name)
    filter->name = g_strdup (name);
}

/**
 * egg_recent_filter_get_name:
 * @filter: a #EggRecentFilter
 *
 * Gets the human-readable name for the filter.
 * See egg_recent_filter_set_name().
 *
 * Return value: the name of the filter, or %NULL.  The returned string
 *   is owned by the filter object and should not be freed.
 *
 * Since: 2.10
 */
G_CONST_RETURN gchar *
egg_recent_filter_get_name (EggRecentFilter *filter)
{
  g_return_val_if_fail (EGG_IS_RECENT_FILTER (filter), NULL);
  
  return filter->name;
}

/**
 * egg_recent_filter_get_needed:
 * @filter: a #EggRecentFilter
 *
 * Gets the fields that need to be filled in for the structure
 * passed to egg_recent_filter_filter()
 * 
 * This function will not typically be used by applications; it
 * is intended principally for use in the implementation of
 * #EggRecentChooser.
 * 
 * Return value: bitfield of flags indicating needed fields when
 *   calling egg_recent_filter_filter()
 *
 * Since: 2.10
 */
EggRecentFilterFlags
egg_recent_filter_get_needed (EggRecentFilter *filter)
{
  return filter->needed;
}

static void
recent_filter_add_rule (EggRecentFilter *filter,
			FilterRule      *rule)
{
  filter->needed |= rule->needed;
  filter->rules = g_slist_append (filter->rules, rule);
}

/**
 * egg_recent_filter_add_mime_type:
 * @filter: a #EggRecentFilter
 * @mime_type: a MIME type
 *
 * Adds a rule that allows resources based on their registered MIME type.
 *
 * Since: 2.10
 */
void
egg_recent_filter_add_mime_type (EggRecentFilter *filter,
				 const gchar     *mime_type)
{
  FilterRule *rule;
  
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  g_return_if_fail (mime_type != NULL);
  
  rule = g_new0 (FilterRule, 1);
  rule->type = FILTER_RULE_MIME_TYPE;
  rule->needed = EGG_RECENT_FILTER_MIME_TYPE;
  rule->u.mime_type = g_strdup (mime_type);
  
  recent_filter_add_rule (filter, rule);
}

/**
 * egg_recent_filter_add_pattern:
 * @filter: a #EggRecentFilter
 * @pattern: a file pattern
 *
 * Adds a rule that allows resources based on a pattern matching their
 * display name.
 *
 * Since: 2.10
 */
void
egg_recent_filter_add_pattern (EggRecentFilter *filter,
			       const gchar     *pattern)
{
  FilterRule *rule;
  
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  g_return_if_fail (pattern != NULL);
  
  rule = g_new0 (FilterRule, 1);
  rule->type = FILTER_RULE_DISPLAY_NAME;
  rule->needed = EGG_RECENT_FILTER_DISPLAY_NAME;
  rule->u.pattern = g_strdup (pattern);
  
  recent_filter_add_rule (filter, rule);
}

/**
 * egg_recent_filter_add_application:
 * @filter: a #EggRecentFilter
 * @application: an application name
 *
 * Adds a rule that allows resources based on the name of the application
 * that has registered them.
 *
 * Since: 2.10
 */
void
egg_recent_filter_add_application (EggRecentFilter *filter,
				   const gchar     *application)
{
  FilterRule *rule;
  
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  g_return_if_fail (application != NULL);
  
  rule = g_new0 (FilterRule, 1);
  rule->type = FILTER_RULE_APPLICATION;
  rule->needed = EGG_RECENT_FILTER_APPLICATION;
  rule->u.application = g_strdup (application);
  
  recent_filter_add_rule (filter, rule);
}

/**
 * egg_recent_filter_add_group:
 * @filter: a #EggRecentFilter
 * @group: a group name
 *
 * Adds a rule that allows resources based on the name of the group
 * to which they belong
 *
 * Since: 2.10
 */
void
egg_recent_filter_add_group (EggRecentFilter *filter,
			     const gchar     *group)
{
  FilterRule *rule;
  
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  g_return_if_fail (group != NULL);
  
  rule = g_new0 (FilterRule, 1);
  rule->type = FILTER_RULE_GROUP;
  rule->needed = EGG_RECENT_FILTER_GROUP;
  rule->u.group = g_strdup (group);
  
  recent_filter_add_rule (filter, rule);
}

/**
 * egg_recent_filter_add_mime_type:
 * @filter: a #EggRecentFilter
 * @days: number of days
 *
 * Adds a rule that allows resources based on their age - that is, the number
 * of days elapsed since they were last modified.
 *
 * Since: 2.10
 */
void
egg_recent_filter_add_age (EggRecentFilter *filter,
			   gint             days)
{
  FilterRule *rule;
  
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  
  rule = g_new0 (FilterRule, 1);
  rule->type = FILTER_RULE_AGE;
  rule->needed = EGG_RECENT_FILTER_AGE;
  rule->u.age = days;
  
  recent_filter_add_rule (filter, rule);
}

/**
 * egg_recent_filter_add_custom:
 * @filter: a #EggRecentFilter
 * @needed: bitfield of flags indicating the information that the custom
 *          filter function needs.
 * @func: callback function; if the function returns %TRUE, then
 *   the file will be displayed.
 * @data: data to pass to @func
 * @data_destroy: function to call to free @data when it is no longer needed.
 * 
 * Adds a rule to a filter that allows resources based on a custom callback
 * function. The bitfield @needed which is passed in provides information
 * about what sorts of information that the filter function needs;
 * this allows GTK+ to avoid retrieving expensive information when
 * it isn't needed by the filter.
 * 
 * Since: 2.10
 **/
void
egg_file_filter_add_custom (EggRecentFilter      *filter,
			    EggRecentFilterFlags  needed,
			    EggRecentFilterFunc   func,
			    gpointer              data,
			    GDestroyNotify        data_destroy)
{
  FilterRule *rule;
  
  g_return_if_fail (EGG_IS_RECENT_FILTER (filter));
  g_return_if_fail (func != NULL);

  rule = g_new (FilterRule, 1);
  rule->type = FILTER_RULE_CUSTOM;
  rule->needed = needed;
  rule->u.custom.func = func;
  rule->u.custom.data = data;
  rule->u.custom.data_destroy = data_destroy;

  recent_filter_add_rule (filter, rule);
}


/**
 * egg_recent_filter_filter:
 * @filter: a #EggRecentFilter
 * @filter_info: a #EggRecentFilterInfo structure containing information
 *   about a recently used resource
 *
 * Tests whether a file should be displayed according to @filter.
 * The #EggRecentFilterInfo structure @filter_info should include
 * the fields returned from egg_recent_filter_get_needed().
 *
 * This function will not typically be used by applications; it
 * is intended principally for use in the implementation of
 * #EggRecentChooser.
 * 
 * Return value: %TRUE if the file should be displayed
 */
gboolean
egg_recent_filter_filter (EggRecentFilter           *filter,
			  const EggRecentFilterInfo *filter_info)
{
  GSList *l;
  
  g_return_val_if_fail (EGG_IS_RECENT_FILTER (filter), FALSE);
  g_return_val_if_fail (filter_info != NULL, FALSE);
  
  for (l = filter->rules; l != NULL; l = l->next)
    {
      FilterRule *rule = (FilterRule *) l->data;

      if ((filter_info->contains & rule->needed) != rule->needed)
        continue;

      switch (rule->type)
        {
        case FILTER_RULE_MIME_TYPE:
          if ((filter_info->mime_type != NULL)
#ifdef G_OS_UNIX
              && (xdg_mime_mime_type_subclass (filter_info->mime_type, rule->u.mime_type)))
#else
	      && (strcmp (filter_info->mime_type, rule->u.mime_type) == 0))
#endif
            return TRUE;
          break;
        case FILTER_RULE_APPLICATION:
          if (filter_info->applications)
            {
              const gchar *app;
              gint i = 0;
              
              app = filter_info->applications[i];
              while (app)
                {
                  if (strcmp (app, rule->u.application) == 0)
                    return TRUE;
                  
                  app = filter_info->applications[++i];
                }
            }
          break;
	case FILTER_RULE_GROUP:
	  if (filter_info->groups)
            {
              const gchar *group;
	      gint i = 0;

	      group = filter_info->groups[i];
	      while (group)
		{
		  if (strcmp (group, rule->u.group))
		    return TRUE;
		  
		  group = filter_info->groups[++i];
		}
	    }
	  break;
        case FILTER_RULE_URI:
          if ((filter_info->uri != NULL) &&
              _gtk_fnmatch (rule->u.uri, filter_info->uri, FALSE))
            return TRUE;
          break;
        case FILTER_RULE_DISPLAY_NAME:
          if ((filter_info->display_name != NULL) &&
              _gtk_fnmatch (rule->u.pattern, filter_info->display_name, FALSE))
            return TRUE;
          break;
        case FILTER_RULE_AGE:
          if ((filter_info->age != -1) &&
              (filter_info->age < rule->u.age))
            return TRUE;
          break;
        case FILTER_RULE_CUSTOM:
          if (rule->u.custom.func (filter_info, rule->u.custom.data))
            return TRUE;
          break;
        }
    }
  
  return FALSE;
}
