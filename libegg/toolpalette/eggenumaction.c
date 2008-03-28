/* EggEnumAction -- An action that creates combo boxes for enums
 * Copyright (C) 2008  Openismus GmbH
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *      Mathias Hasselmann
 */

#include "eggenumaction.h"
#include <gtk/gtk.h>

#define P_(msgid) (msgid)

enum
{
  PROP_NONE,
  PROP_ENUM_TYPE,
};

struct _EggEnumActionPrivate
{
  GType         enum_type;
  GEnumClass   *enum_class;
  GSList       *bindings;
  GSList       *callbacks;
  GtkListStore *model;

  EggEnumActionFilterFunc filter_func;
  GDestroyNotify          filter_destroy;
  gpointer                filter_data;
};

static GQuark egg_enum_action_child_quark;
static GQuark egg_enum_action_value_quark;

G_DEFINE_TYPE (EggEnumAction, egg_enum_action, GTK_TYPE_ACTION);

static void
egg_enum_action_init (EggEnumAction *action)
{
  action->priv = G_TYPE_INSTANCE_GET_PRIVATE (action,
                                              EGG_TYPE_ENUM_ACTION,
                                              EggEnumActionPrivate);
}

static void
egg_enum_action_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  EggEnumAction *action = EGG_ENUM_ACTION (object);
  GType enum_type;

  switch (prop_id)
    {
      case PROP_ENUM_TYPE:
        enum_type = g_value_get_gtype (value);

        if (enum_type == action->priv->enum_type)
          break;

        if (action->priv->enum_class)
          {
            g_type_class_unref (action->priv->enum_class);
            action->priv->enum_class = NULL;
          }

        action->priv->enum_type = enum_type;
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_enum_action_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  EggEnumAction *action = EGG_ENUM_ACTION (object);

  switch (prop_id)
    {
      case PROP_ENUM_TYPE:
        g_value_set_gtype (value, action->priv->enum_type);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_enum_action_dispose (GObject *object)
{
  EggEnumAction *action = EGG_ENUM_ACTION (object);

  if (action->priv->enum_class)
    {
      g_type_class_unref (action->priv->enum_class);
      action->priv->enum_class = NULL;
    }

  if (action->priv->model)
    {
      g_type_class_unref (action->priv->model);
      action->priv->model = NULL;
    }

  while (action->priv->bindings)
    {
      g_param_spec_unref (action->priv->bindings->data);
      g_object_unref (action->priv->bindings->next->data);

      action->priv->bindings->data = g_slist_delete_link (action->priv->bindings->data,
                                                          action->priv->bindings->data);
      action->priv->bindings->data = g_slist_delete_link (action->priv->bindings->data,
                                                          action->priv->bindings->data);
    }

  if (action->priv->callbacks)
    {
      g_slist_free (action->priv->callbacks);
      action->priv->callbacks = NULL;
    }

  if (action->priv->filter_destroy)
    {
      action->priv->filter_destroy (action->priv->filter_data);
      action->priv->filter_destroy = NULL;
    }

  action->priv->filter_func = NULL;
  action->priv->filter_data = NULL;

  G_OBJECT_CLASS (egg_enum_action_parent_class)->dispose (object);
}

static GtkTreeModel*
egg_enum_action_get_model (EggEnumAction *action)
{
  if (!action->priv->enum_class)
    action->priv->enum_class = g_type_class_ref (action->priv->enum_type);

  if (!action->priv->model)
    {
      GtkTreeIter iter;
      guint i;

      action->priv->model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);

      for (i = 0; i < action->priv->enum_class->n_values; ++i)
        {
          GEnumValue *enum_value = &action->priv->enum_class->values[i];

          if (action->priv->filter_func &&
             !action->priv->filter_func (enum_value, action->priv->filter_data))
            continue;

          gtk_list_store_append (action->priv->model, &iter);
          gtk_list_store_set (action->priv->model, &iter,
                              0, enum_value->value_nick,
                              1, enum_value, -1);
        }
    }

  return GTK_TREE_MODEL (action->priv->model);
}

static gboolean
egg_enum_action_get_iter (EggEnumAction *action,
                          GtkTreeIter   *iter,
                          GObject       *object,
                          GParamSpec    *property)
{
  GtkTreeModel *model = egg_enum_action_get_model (action);
  GEnumValue *enum_value;
  gint current_value;

  if (!gtk_tree_model_get_iter_first (model, iter))
    return FALSE;

  if (!G_IS_OBJECT (object) || !G_IS_PARAM_SPEC_ENUM (property))
    return TRUE;

  g_object_get (object, property->name, &current_value, NULL);

  do
    {
      gtk_tree_model_get (model, iter, 1, &enum_value, -1);

      if (enum_value->value == current_value)
        return TRUE;
    }
  while (gtk_tree_model_iter_next (model, iter));

  return FALSE;
}

static gboolean
egg_enum_action_get_active_iter (EggEnumAction *action,
                                 GtkTreeIter   *iter)
{
  GParamSpec *property = NULL;
  GObject *object = NULL;

  if (action->priv->bindings)
    {
      property = action->priv->bindings->data;
      object = action->priv->bindings->next->data;
    }

  return egg_enum_action_get_iter (action, iter, object, property);
}

static void
egg_enum_action_set_value (EggEnumAction *action,
                           GEnumValue    *enum_value)
{
  GSList *iter;

  for (iter = action->priv->bindings; iter; iter = iter->next->next)
    {
      GParamSpec *property = iter->data;
      GObject *object = iter->next->data;

      g_object_set (object, property->name, enum_value->value, NULL);
    }

  for (iter = action->priv->callbacks; iter; iter = iter->next->next)
    {
      EggEnumActionCallback callback = iter->next->data;
      gpointer user_data = iter->data;

      callback (enum_value, user_data);
    }
}

static void
egg_enum_action_combo_changed (GtkComboBox *combo,
                               gpointer     data)
{
  EggEnumAction *action = EGG_ENUM_ACTION (data);
  GEnumValue *enum_value;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_combo_box_get_active_iter (combo, &iter))
    {
      model = egg_enum_action_get_model (action);
      gtk_tree_model_get (model, &iter, 1, &enum_value, -1);
      egg_enum_action_set_value (action, enum_value);
    }
}

static void
egg_enum_action_toolbar_reconfigured (GtkToolItem *item,
                                      gpointer     data)
{
  gboolean important = gtk_tool_item_get_is_important (item);
  GtkToolbarStyle style = gtk_tool_item_get_toolbar_style (item);
  EggEnumAction *action = EGG_ENUM_ACTION (data);
  gchar *text, *tmp;

  GtkWidget *align, *box = NULL;
  GtkWidget *combo, *label;
  GtkCellRenderer *cell;
  GtkTreeIter iter;

  align = gtk_bin_get_child (GTK_BIN (item));
  box = gtk_bin_get_child (GTK_BIN (align));

  if (box)
    gtk_container_remove (GTK_CONTAINER (align), box);

  g_object_get (action, "label", &text, NULL);
  box = NULL;

  combo = gtk_combo_box_new_with_model (egg_enum_action_get_model (action));
  g_signal_connect (combo, "changed", G_CALLBACK (egg_enum_action_combo_changed), action);

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell, "text", 0, NULL);
  g_object_set_qdata (G_OBJECT (item), egg_enum_action_child_quark, combo);

  if (egg_enum_action_get_active_iter (action, &iter))
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);

  if (GTK_TOOLBAR_BOTH == style)
    {
      label = gtk_label_new (text);

      box = gtk_vbox_new (FALSE, 0);
      gtk_box_pack_end (GTK_BOX (box), label, FALSE, TRUE, 0);
    }
  else if (GTK_TOOLBAR_ICONS != style || important)
    {
      tmp = g_strconcat (text, ":", NULL);
      label = gtk_label_new (tmp);
      g_free (tmp);

      gtk_misc_set_padding (GTK_MISC (label), 3, 0);

      box = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);
    }

  if (box)
    {
      gtk_box_pack_start (GTK_BOX (box), combo, TRUE, TRUE, 0);
      gtk_container_add (GTK_CONTAINER (align), box);
    }
  else
    gtk_container_add (GTK_CONTAINER (align), combo);

  gtk_widget_show_all (align);

  g_free (text);
}

static void
egg_enum_action_select_menu_item (EggEnumAction *action,
                                  GtkTreeIter   *iter,
                                  GtkWidget     *menu)
{
  GList *items = gtk_container_get_children (GTK_CONTAINER (menu));
  GtkTreeModel *model = egg_enum_action_get_model (action);
  GtkTreePath *path = gtk_tree_model_get_path (model, iter);
  gint item_index = gtk_tree_path_get_indices (path)[0];
  GtkWidget *child = g_list_nth_data (items, item_index);

  if (child)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (child), TRUE);

  gtk_tree_path_free (path);
  g_list_free (items);
}

static void
egg_enum_action_menu_item_toggled (GtkCheckMenuItem *item,
                                   gpointer          data)
{
  GEnumValue *enum_value;

  enum_value = g_object_get_qdata (G_OBJECT (item), egg_enum_action_value_quark);
  egg_enum_action_set_value (EGG_ENUM_ACTION (data), enum_value);
}

static GtkWidget*
egg_enum_action_create_menu_item (GtkAction *action)
{
  GtkTreeModel *model = egg_enum_action_get_model (EGG_ENUM_ACTION (action));
  GEnumValue *enum_value;
  GtkTreeIter iter;
  gchar *label;

  GtkWidget *item;
  GtkWidget *menu = gtk_menu_new ();
  GSList *group = NULL;

  if (gtk_tree_model_get_iter_first (model, &iter))
    do
      {
        gtk_tree_model_get (model, &iter, 0, &label, 1, &enum_value, -1);

        item = gtk_radio_menu_item_new_with_label (group, label);
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

        g_object_set_qdata (G_OBJECT (item),
                            egg_enum_action_value_quark,
                            enum_value);

        g_signal_connect (item, "toggled",
                          G_CALLBACK (egg_enum_action_menu_item_toggled),
                          action);
      }
    while (gtk_tree_model_iter_next (model, &iter));

  gtk_widget_show_all (menu);

  if (egg_enum_action_get_active_iter (EGG_ENUM_ACTION (action), &iter))
    egg_enum_action_select_menu_item (EGG_ENUM_ACTION (action), &iter, menu);

  item = GTK_ACTION_CLASS (egg_enum_action_parent_class)->create_menu_item (action);
  g_object_set_qdata (G_OBJECT (item), egg_enum_action_child_quark, menu);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);

  return item;
}

static GtkWidget*
egg_enum_action_create_tool_item (GtkAction *action)
{
  GtkToolItem *item = gtk_tool_item_new ();
  GtkWidget *align = gtk_alignment_new (0.5, 0.5, 1, 0);

  gtk_container_add (GTK_CONTAINER (item), align);

  g_signal_connect (item, "toolbar-reconfigured",
                    G_CALLBACK (egg_enum_action_toolbar_reconfigured),
                    action);

  return GTK_WIDGET (item);
}

static void
egg_enum_action_class_init (EggEnumActionClass *cls)
{
  GObjectClass *oclass = G_OBJECT_CLASS (cls);
  GtkActionClass *aclass = GTK_ACTION_CLASS (cls);

  oclass->set_property     = egg_enum_action_set_property;
  oclass->get_property     = egg_enum_action_get_property;
  oclass->dispose          = egg_enum_action_dispose;

  aclass->create_menu_item = egg_enum_action_create_menu_item;
  aclass->create_tool_item = egg_enum_action_create_tool_item;

  g_object_class_install_property (oclass, PROP_ENUM_TYPE,
                                   g_param_spec_gtype ("enum-type",
                                                       P_("Enum Type"),
                                                       P_("Type of the enumeration"),
                                                       G_TYPE_ENUM,
                                                       G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                       G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_type_class_add_private (cls, sizeof (EggEnumActionPrivate));

  egg_enum_action_child_quark = g_quark_from_static_string ("egg-enum-action-child-quark");
  egg_enum_action_value_quark = g_quark_from_static_string ("egg-enum-action-value-quark");
}

GtkAction*
egg_enum_action_new (const gchar *name,
                     const gchar *label,
                     const gchar *tooltip,
                     GType        enum_type)
{
  g_return_val_if_fail (NULL != name, NULL);
  g_return_val_if_fail (g_type_is_a (enum_type, G_TYPE_ENUM), NULL);

  return g_object_new (EGG_TYPE_ENUM_ACTION, "name", name, "label", label,
                       "tooltip", tooltip, "enum-type", enum_type, NULL);
}

static void
egg_enum_action_notify (GObject    *object,
                        GParamSpec *property,
                        gpointer    data)
{
  EggEnumAction *action = EGG_ENUM_ACTION (data);
  GtkTreeIter active_iter;
  GtkWidget *child;
  GSList *proxies;

  if (egg_enum_action_get_iter (action, &active_iter, object, property))
    {
      proxies = gtk_action_get_proxies (GTK_ACTION (action));

      while (proxies)
        {
          child = g_object_get_qdata (proxies->data, egg_enum_action_child_quark);

          if (GTK_IS_COMBO_BOX (child))
            gtk_combo_box_set_active_iter (GTK_COMBO_BOX (child), &active_iter);
          else if (GTK_IS_MENU (child))
            egg_enum_action_select_menu_item (action, &active_iter, child);

          proxies = proxies->next;
        }
    }
}

void
egg_enum_action_bind (EggEnumAction *action,
                      GObject       *object,
                      const gchar   *property_name)
{
  gchar *signal_name;
  GParamSpec *property;

  g_return_if_fail (EGG_IS_ENUM_ACTION (action));
  g_return_if_fail (GTK_IS_OBJECT (object));
  g_return_if_fail (NULL != property_name);

  property = g_object_class_find_property (G_OBJECT_GET_CLASS (object),
                                           property_name);

  g_return_if_fail (NULL != property);
  g_return_if_fail (g_type_is_a (property->value_type, action->priv->enum_type));

  signal_name = g_strconcat ("notify::", property_name, NULL);

  g_signal_connect (object, signal_name,
                    G_CALLBACK (egg_enum_action_notify),
                    action);

  g_free (signal_name);

  action->priv->bindings = g_slist_prepend (action->priv->bindings, g_object_ref (object));
  action->priv->bindings = g_slist_prepend (action->priv->bindings, g_param_spec_ref (property));

  egg_enum_action_notify (object, property, action);
}

void
egg_enum_action_connect (EggEnumAction         *action,
                         EggEnumActionCallback  callback,
                         gpointer               data)
{
  g_return_if_fail (EGG_IS_ENUM_ACTION (action));
  g_return_if_fail (NULL != callback);

  action->priv->callbacks = g_slist_prepend (action->priv->callbacks, callback);
  action->priv->callbacks = g_slist_prepend (action->priv->callbacks, data);
}

void
egg_enum_action_set_filter (EggEnumAction           *action,
                            EggEnumActionFilterFunc  filter,
                            gpointer                 user_data,
                            GDestroyNotify           destroy_data)
{
  g_return_if_fail (EGG_IS_ENUM_ACTION (action));

  if (action->priv->filter_destroy)
    action->priv->filter_destroy (action->priv->filter_data);

  action->priv->filter_func = filter;
  action->priv->filter_data = user_data;
  action->priv->filter_destroy = destroy_data;
}
