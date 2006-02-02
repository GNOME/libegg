#include "eggtreeviewstate.h"
#include <string.h>
#include <stdlib.h>

#define _(x) (x)

typedef enum
{
  STATE_START,
  STATE_TREEVIEW_STATE,
  STATE_TREEVIEW,
  STATE_COLUMN,
  STATE_CELL,
} ParseState;

typedef struct
{
  gchar *name;
  gint column;
} CellAttribute;

typedef struct
{
  GSList *states;
  GtkTreeView *view;

  GtkTreeViewColumn *column;
  
  GtkCellRenderer *cell;
  gboolean pack_start;
  gboolean expand;
  GSList *cell_attributes;
} ParseInfo;

static void
set_error (GError             **err,
           GMarkupParseContext *context,
           int                  error_domain,
           int                  error_code,
           const char          *format,
           ...)
{
  int line, ch;
  va_list args;
  char *str;
  
  g_markup_parse_context_get_position (context, &line, &ch);

  va_start (args, format);
  str = g_strdup_vprintf (format, args);
  va_end (args);

  g_set_error (err, error_domain, error_code,
               _("Line %d character %d: %s"),
               line, ch, str);

  g_free (str);
}

static ParseState
peek_state (ParseInfo *info)
{
  g_return_val_if_fail (info->states != NULL, STATE_START);

  return GPOINTER_TO_INT (info->states->data);
}

static void
push_state (ParseInfo  *info,
            ParseState  state)
{
  info->states = g_slist_prepend (info->states, GINT_TO_POINTER (state));
}

static void
pop_state (ParseInfo *info)
{
  g_return_if_fail (info->states != NULL);
  
  info->states = g_slist_remove (info->states, info->states->data);
}

static void
parse_info_init (ParseInfo *info)
{
  info->states = g_slist_prepend (NULL, GINT_TO_POINTER (STATE_START));
  info->cell_attributes = NULL;
  
}

#define MAX_REASONABLE 4096
static gboolean
parse_integer (const char          *str,
	       GValue              *value,
	       GMarkupParseContext *context,
	       GError             **error)
{
  char *end;
  long l;

  end = NULL;
  
  l = strtol (str, &end, 10);

  if (end == NULL || end == str)
    {
      set_error (error, context, G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 _("Could not parse \"%s\" as an integer"),
                 str);
      return FALSE;
    }

  if (*end != '\0')
    {
      set_error (error, context, G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 _("Did not understand trailing characters \"%s\" in string \"%s\""),
                 end, str);
      return FALSE;
    }

  if (l < 0)
    {
      set_error (error, context, G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 _("Integer %ld must be positive"), l);
      return FALSE;
    }

  if (l > MAX_REASONABLE)
    {
      set_error (error, context, G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 _("Integer %ld is too large, current max is %d"),
                 l, MAX_REASONABLE);
      return FALSE;
    }

  g_value_init (value, G_TYPE_INT);
  g_value_set_int (value, (int)l);

  return TRUE;
}

static gboolean
parse_string (const char          *str,
	      GValue              *value,
	      GMarkupParseContext *context,
	      GError             **error)
{
  if (!str)
    return FALSE;

  g_value_init (value, G_TYPE_STRING);
  g_value_set_string (value, str);

  return TRUE;
}

static gboolean
parse_boolean (const char          *str,
               GValue              *value,
               GMarkupParseContext *context,
               GError             **error)
{
  if (strcmp ("true", str) == 0)
    {
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, TRUE);
    }
  else if (strcmp ("false", str) == 0)
    {
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, FALSE);
    }
  else
    {
      set_error (error, context, G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 _("Boolean values must be \"true\" or \"false\" not \"%s\""),
                 str);
      return FALSE;
    }
  
  return TRUE;
}

static gboolean
parse_enum (const gchar          *str,
	    GValue               *value,
	    GParamSpecEnum       *pspec,
	    GMarkupParseContext  *context,
	    GError              **error)
{
  GEnumValue *enum_value;

  enum_value = g_enum_get_value_by_nick (pspec->enum_class, str);

  if (!enum_value)
    {
      set_error (error, context, G_MARKUP_ERROR,
                 G_MARKUP_ERROR_PARSE,
                 _("The value \"%s\" is not part of the enum \"%s\""),
                 str,
		 G_ENUM_CLASS_TYPE_NAME (pspec->enum_class));
      return FALSE;
    }

  g_value_init (value, G_ENUM_CLASS_TYPE (pspec->enum_class));
  g_value_set_enum (value, enum_value->value);
  
  return TRUE;
      
}

static gboolean
parse_value (const gchar          *string,
	     GValue               *value,
	     GParamSpec           *pspec,
	     GMarkupParseContext  *context,
	     GError              **error)
{
  if (G_IS_PARAM_SPEC_BOOLEAN (pspec))
    return parse_boolean (string, value, context, error);
  else if (G_IS_PARAM_SPEC_INT (pspec))
    return parse_integer (string, value, context, error);
  else if (G_IS_PARAM_SPEC_STRING (pspec))
    return parse_string (string, value, context, error);
  else if (G_IS_PARAM_SPEC_ENUM (pspec))
    return parse_enum (string, value, G_PARAM_SPEC_ENUM (pspec), context, error);
  
  set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
	     _("The type \"%s\" can't be parsed from a string"),
	     g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec)));
  return FALSE;
}

static gboolean
parse_object_property (GObject             *object,
		       const gchar         *attribute_name,
		       const gchar         *attribute_value,
		       GMarkupParseContext *context,
		       GError             **error)
{
  GParamSpec *pspec;
  GValue value = { 0 };
  
  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), attribute_name);

  if (!pspec)
    {
      set_error (error, context, G_MARKUP_ERROR,
		 G_MARKUP_ERROR_PARSE,
		 _("The property \"%s\" does not exist"),
		 attribute_name);
      return FALSE;
    }
  
  if (!parse_value (attribute_value, &value, pspec, context, error))
    return FALSE;
  
  g_object_set_property (object, attribute_name, &value);
  g_value_unset (&value);

  return TRUE;
}

static gboolean
parse_object_properties (GObject             *object,
			 const gchar        **attribute_names,
			 const gchar        **attribute_values,
			 GMarkupParseContext *context,
			 GError             **error)
{
  gint i;
  
  for (i = 0; attribute_names[i] != NULL; i++)
    {
      if (!parse_object_property (object, attribute_names[i], attribute_values[i], context, error))
	return FALSE;
    }

  return TRUE;
}

static void
parse_cell_element (GMarkupParseContext *context,
		    const gchar        **attribute_names,
		    const gchar        **attribute_values,
		    ParseInfo           *info,
		    GError             **error)
{
  int i;
  GtkCellRenderer *cell = NULL;

  info->pack_start = TRUE;
  info->expand = TRUE;
  
  /* We first need to traverse the attributes looking for a type */
  for (i = 0; attribute_names[i]; i++)
    {
      if (strcmp (attribute_names[i], "type") == 0)
	{
	  GType type;
	  
	  if (cell != NULL)
	    {
	      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
			 _("The type attribute can only be specified once."));
	      return;
	    }
	  
	  type = g_type_from_name (attribute_values[i]);
	  
	  if (type == G_TYPE_INVALID)
	    {
	      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
			 _("The type \"%s\" is not a valid type."),
			 attribute_values[i]);
	      return;
	    }
	  
	  if (!g_type_is_a (type, GTK_TYPE_CELL_RENDERER))
	    {
	      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
			 _("The type \"%s\" is not a cell renderer type."),
			 g_type_name (type));
	      return;
	    }

	  cell = g_object_new (type, NULL);
	}
    }

  if (!cell)
    {
      set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
		 _("No type attribute specified."));
      return;
    }
  
  for (i = 0; attribute_names[i]; i++)
    {
      if (strcmp (attribute_names[i], "type") == 0)
	continue;
      else if (strcmp (attribute_names[i], "pack_start") == 0)
	{
	  GValue value = { 0 };

	  if (!parse_boolean (attribute_values[i], &value, context, error))
	    return;

	  info->pack_start = g_value_get_boolean (&value);
	}
      else if (strcmp (attribute_names[i], "expand") == 0)
	{
	  GValue value = { 0 };

	  if (!parse_boolean (attribute_values[i], &value, context, error))
	    return;

	  info->expand = g_value_get_boolean (&value);
	}
      else
	{
	  if (strstr (attribute_values[i], "model:") == attribute_values[i])
	    {
	      CellAttribute *attr;
	      GValue value = { 0 };

	      if (!parse_integer (attribute_values[i] + strlen ("model:"), &value, context, error))
		return;
	      
	      attr = g_new (CellAttribute, 1);
	      attr->name = g_strdup (attribute_names[i]);
	      attr->column = g_value_get_int (&value);
	      g_value_unset (&value);

	      info->cell_attributes = g_slist_prepend (info->cell_attributes, attr);
	    }
	  else
	    {
	      if (!parse_object_property (G_OBJECT (cell),
					  attribute_names[i],
					  attribute_values[i],
					  context,
					  error))
		return;
	    }
	}
    }

  push_state (info, STATE_CELL);
  info->cell = cell;
}

static void
parse_column_element (GMarkupParseContext *context,
		      const gchar        **attribute_names,
		      const gchar        **attribute_values,
		      ParseInfo           *info,
		      GError             **error)
{
  GtkTreeViewColumn *column;

  column = gtk_tree_view_column_new ();

  if (!parse_object_properties (G_OBJECT (column),
				attribute_names,
				attribute_values,
				context,
				error))
    return;

  push_state (info, STATE_COLUMN);
  
  info->column = column;
}

static void
parse_treeview_element (GMarkupParseContext *context,
			const gchar        **attribute_names,
			const gchar        **attribute_values,
			ParseInfo           *info,
			GError             **error)
{
  if (!parse_object_properties (G_OBJECT (info->view),
				attribute_names,
				attribute_values,
				context,
				error))
    return;
  
  push_state (info, STATE_TREEVIEW);
}

static void
start_element_handler (GMarkupParseContext *context,
		       const gchar         *element_name,
		       const gchar        **attribute_names,
		       const gchar        **attribute_values,
		       gpointer             user_data,
		       GError             **error)
{
  ParseInfo *info = user_data;

  switch (peek_state (info))
    {
    case STATE_START:
      if (strcmp (element_name, "treeview_state") == 0)
	{
	  push_state (info, STATE_TREEVIEW_STATE);
	}
      else
	set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   _("Outermost element in theme must be <treeview_state> not <%s>"),
                   element_name);
      break;
    case STATE_TREEVIEW_STATE:
      if (strcmp (element_name, "treeview") == 0)
	{
	  parse_treeview_element (context, attribute_names, attribute_values, info, error);
	}
      else
	set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   _("Element inside of <treeview_state> must be <treeview> not <%s>"),
                   element_name);
      break;
    case STATE_TREEVIEW:
      if (strcmp (element_name, "column") == 0)
	{
	  parse_column_element (context, attribute_names, attribute_values, info, error);
	}
      else
	set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   _("Element inside of <treeview> must be <column> not <%s>"),
                   element_name);

      break;
    case STATE_COLUMN:
      if (strcmp (element_name, "cell") == 0)
	{
	  parse_cell_element (context, attribute_names, attribute_values, info, error);
	}
      else
	set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   _("Element inside of <column> must be <cell> not <%s>"),
                   element_name);

      break;
    case STATE_CELL:
	set_error (error, context, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                   _("The <cell> element must not have any children."));
	break;
    }
}

static void
end_element_handler (GMarkupParseContext *context,
                     const gchar         *element_name,
                     gpointer             user_data,
                     GError             **error)
{
  ParseInfo *info = user_data;
  GSList *list;
  
  switch (peek_state (info))
    {
    case STATE_START:
      break;
    case STATE_TREEVIEW_STATE:
      pop_state (info);
      g_assert (peek_state (info) == STATE_START);
      break;
    case STATE_TREEVIEW:
      pop_state (info);
      g_assert (peek_state (info) == STATE_TREEVIEW_STATE);
      break;
    case STATE_COLUMN:
      g_assert (info->column);

      gtk_tree_view_append_column (info->view, info->column);
      
      pop_state (info);
      g_assert (peek_state (info) == STATE_TREEVIEW);
      break;
    case STATE_CELL:
      g_assert (info->cell);

      if (info->pack_start)
	gtk_tree_view_column_pack_start (info->column, info->cell,
					 info->expand);
      else
	gtk_tree_view_column_pack_end (info->column, info->cell,
				       info->expand);

      for (list = info->cell_attributes; list; list = list->next)
	{
	  CellAttribute *attr = list->data;

	  gtk_tree_view_column_add_attribute (info->column, info->cell,
					      attr->name,
					      attr->column);
	  g_free (attr->name);
	  g_free (attr);
	}
      
      g_slist_free (info->cell_attributes);
      info->cell_attributes = NULL;
      
      pop_state (info);
      g_assert (peek_state (info) == STATE_COLUMN);
    }
}


static GMarkupParser parser =
{
  start_element_handler,
  end_element_handler,
  NULL,
  NULL,
};

gboolean
egg_tree_view_state_apply_from_string (GtkTreeView *tree_view, const gchar *string, GError **err)
{
	GMarkupParseContext *context;
	GError *error = NULL;
	gboolean retval;
	ParseInfo info;

	parse_info_init (&info);
	info.view = tree_view;
	
	context = g_markup_parse_context_new (&parser, 0, &info, NULL);

	retval = g_markup_parse_context_parse (context, string, -1, &error);

	if (!retval)
	  {
	    if (err)
	      *err = error;
	  }
	
	return retval;
}

void
egg_tree_view_state_add_cell_renderer_type (GType type)
{
  /* Do nothing, this is just to have the types registered */
}

