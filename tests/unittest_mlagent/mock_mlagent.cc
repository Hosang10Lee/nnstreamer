/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    mlagent_mock.cc
 * @date    30 Nov 2023
 * @brief   A class that mocks the ML Agent instance
 * @author  Wook Song <wook16.song@samsung.com>
 * @see     http://github.com/nnstreamer/nnstreamer
 * @bug     No known bugs
 *
 */

#include <glib.h>
#include <json-glib/json-glib.h>
#include <nnstreamer_util.h>

#include <functional>
#include <iostream>
#include <memory>

#include "mock_mlagent.h"

static std::unique_ptr<MockMLAgent> uptr_mock;

/**
 * @brief Generate C-stringified JSON
 */
gchar *
MockModel::to_cstr_json ()
{
  using json_member_name_to_cb_t
      = std::pair<std::string, std::function<std::string ()>>;

  const std::vector<json_member_name_to_cb_t> json_mem_to_cb_map{
    json_member_name_to_cb_t (
        "path", [this] () -> std::string { return path (); }),
    json_member_name_to_cb_t (
        "description", [this] () -> std::string { return desc (); }),
    json_member_name_to_cb_t (
        "app_info", [this] () -> std::string { return app_info (); }),
    json_member_name_to_cb_t ("version",
        [this] () -> std::string { return std::to_string (version ()); }),
    json_member_name_to_cb_t ("active",
        [this] () -> std::string { return (is_activated () ? "T" : "F"); }),
  };

  g_autoptr (JsonBuilder) builder = json_builder_new ();
  g_autoptr (JsonGenerator) gen = NULL;

  json_builder_begin_object (builder);
  for (auto iter : json_mem_to_cb_map) {
    json_builder_set_member_name (builder, iter.first.c_str ());
    json_builder_add_string_value (builder, iter.second ().c_str ());
  }
  json_builder_end_object (builder);

  {
    g_autoptr (JsonNode) root = json_builder_get_root (builder);

    gen = json_generator_new ();
    json_generator_set_root (gen, root);
    json_generator_set_pretty (gen, TRUE);
  }

  return json_generator_to_data (gen, NULL);
}

/**
 * @brief Initialize the static unique_ptr of MockMLAgent
 */
void
ml_agent_mock_init ()
{
  uptr_mock = std::make_unique<MockMLAgent> ();
}

/**
 * @brief C-wrapper for the MockModel's method add_model.
 */
bool
ml_agent_mock_add_model (const gchar *name, const gchar *path, const gchar *app_info,
    const bool is_activated, const char *desc, const guint version)
{
  MockModel model{ name, path, app_info, is_activated, desc, version };

  return uptr_mock->add_model (model);
}

/**
 * @brief C-wrapper for the MockModel's method get_model.
 */
MockModel *
ml_agent_mock_model_get (const gchar *name, const guint version)
{
  return uptr_mock->get_model (name, version);
}

/**
 * @brief Pass the JSON c-string generated by the ML Agent mock class to the caller.
 */
gint
ml_agent_model_get (const gchar *name, const guint version, gchar **description, GError **err)
{
  MockModel *model_ptr = ml_agent_mock_model_get (name, version);

  g_return_val_if_fail (err == NULL || *err == NULL, -EINVAL);

  if (model_ptr == nullptr) {
    return -EINVAL;
  }

  *description = model_ptr->to_cstr_json ();

  return 0;
}
