
#define A_URI	"file:///usr/share/backgrounds/scalable/tiger.svg"

#include "eggmd5.h"

int main (int argc, char *argv[])
{
  EggMd5Digest *digest1, *digest2;
  gchar *md5_str;

  g_message ("URI = `" A_URI "'.");
  md5_str = egg_str_get_md5_str (A_URI);
  g_message ("md5_str of URI = `%s'.", md5_str);

  digest1 = egg_md5_str_to_digest (md5_str);
  digest2 = egg_str_get_md5_digest (A_URI);
  g_message ("Comparison of digest of md5_str to digest of URI: %s.",
	     (egg_md5_digest_equal (digest1, digest2) ? "TRUE" : "FALSE"));
  g_free (digest1);
  g_free (digest2);
  g_free (md5_str);

  return 0;
}
