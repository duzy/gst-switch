
#include "tools/gstrecorder.c"
#include <fcntl.h>

#define BASEDIR  "/tmp/unittests/"

// fake options struct, not used in these tests
GstSwitchServerOpts opts;
gboolean verbose = FALSE;



static char *
filepath(char *buf, size_t len, char const *fn)
{
  snprintf(buf, len, BASEDIR "%d/%s", getpid(), fn);
  return buf;
}


static gboolean
exists(const char *filepath, gboolean dir)
{
  struct stat s;

  if (stat(filepath, &s) == 0) {
  	if (!dir || S_ISDIR(s.st_mode))
  		return TRUE;
  }
  return FALSE;
}

static gboolean
touch(const char *filepath)
{
  int fd = open(filepath, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fd != -1) {
  	close(fd);
  	return TRUE;
  }
  return FALSE;
}


static int
create_new_directory(const char *name)
{
  char buf[256];

  gst_recorder_mkdirs(filepath(buf, sizeof buf, name));
  return exists(buf, TRUE);
}


static gboolean
create_capture_file(const char *name)
{
  char buf[256];
  char const *capfilename = gst_recorder_new_filename(filepath(buf, sizeof buf, name));
  return touch(capfilename);
}

static void
test_mkdirs()
{
  g_assert_cmpint (create_new_directory("testdir1"), !=, 0);
  g_assert_cmpint (create_new_directory("testdir2"), !=, 0);
  g_assert_cmpint (create_new_directory("testdir1/testdir3"), !=, 0);
  g_assert_cmpint (create_new_directory("testdir3/testdir1"), !=, 0);
}

static void
test_new_filename()
{
  g_assert_true(create_capture_file("%Y%m%d_%T"));
  g_assert_true(create_capture_file("%Y%m/%d_%T"));
  g_assert_true(create_capture_file("%Y/%m/%d/%T"));
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);
  gst_init (&argc, &argv);
  g_test_set_nonfatal_assertions ();
  g_test_add_func ("/gstswitch/options/mkdirs", test_mkdirs);
  g_test_add_func ("/gstswitch/options/filename", test_new_filename);
  return g_test_run ();
}

