#include "utils.h"

gchar *
utils_get_diary_folder ()
{
  char *home = getenv("HOME");
  return g_strdup_printf("%s/Documents/Diary", home);
}
