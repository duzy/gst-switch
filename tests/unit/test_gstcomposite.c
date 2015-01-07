#include <glib.h>

#include "tools/gstcomposite.h"

static void
mode_to_string (void)
{
  g_assert_cmpstr (gst_composite_mode_to_string (COMPOSE_MODE_NONE), ==,
      "COMPOSE_MODE_NONE");
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_set_nonfatal_assertions ();
  g_test_add_func ("/gstswitch/server/composite/mode_to_string",
      mode_to_string);
  return g_test_run ();
}
