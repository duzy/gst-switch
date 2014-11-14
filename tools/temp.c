#include <gst/gst.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <string.h>
#include <stdio.h>


struct _FormatAlias {
  const const gchar** shortcuts;
  const gchar* format;
};
typedef struct _FormatAlias FormatAlias;

#define ADD_FORMAT_ALIASES(format, ...) \
  { (const gchar*[]){ __VA_ARGS__ , NULL }, format }

static FormatAlias format_aliases[] = {
  ADD_FORMAT_ALIASES("300x200@25", "debug"),

  // Computer resolutions. These can be at pretty much any frame rate (common
  // ones being 60Hz, 75Hz and 80Hz) so the frame rate must still be specified.
  // For example VGA@75.
  ADD_FORMAT_ALIASES( "640x480",  "VGA"),
  ADD_FORMAT_ALIASES( "800x600", "SVGA"),
  ADD_FORMAT_ALIASES("1024x768",  "XGA"),

  // Older Analog TV formats from DV. These formats have only single frame
  // rate but is interlaced. After deinterlacing, the frame rates are halved.
  // Both PAL and NSTC also have non-square pixels, which need to be converted
  // to square pixels.
  // See http://help.adobe.com/en_US/PremierePro/4.0/WS03BF7479-8C7B-4522-8C75-210AD102524Ea.html#WS082E922A-A5D7-4de1-9A0E-1EFC401EA6D1

  // DV 4:3 PAL, 50 *fields* per second, 720x576 frame size, pixel ratio 59/54 (1.09).
  // Therefore 788x576@25
  ADD_FORMAT_ALIASES("788x576@25", "pal", "pal-4:3", "pal-dv"),
  // DV 16:9 PAL, 50 *fields* per second, 720x576 frame size, pixel ratio 118/81 (1.46).
  // Therefore 1050x576@25
  ADD_FORMAT_ALIASES("1050x576@25", "pal-16:9", "pal-dvd"),

  // Technically NTSC has a frame rate of 60/1.001 and *either* 486 or 480
  // horizontal lines, but the dvdec plugin in gstreamer only supports 60Hz and
  // 480 horizontal lines.

  // DV 4:3 NTSC, 60 *fields* per second, 720x480 frame size, pixel ratio 10/11 (0.91).
  // Therefore 720x534@30
  ADD_FORMAT_ALIASES("720x534@25", "ntsc", "ntsc-4:3", "ntsc-dv"),
  // DV 16:9 NTSC, 60 *fields* per second, 720x480 frame size, pixel ratio 40/33 (1.21).
  // Therefore 864x480@30
  ADD_FORMAT_ALIASES("864x480@25", "ntsc", "ntsc-16:9", "ntsc-dvd"),

  // None of the HD formats also don't have default frame rates, so the rate
  // must be specified.

  // In the following resolutions the 'p' is used in place of the '@', IE
  // 720p60.
  // Digital TV resolutions.
  ADD_FORMAT_ALIASES( "1280x720@",  "720p"),
  ADD_FORMAT_ALIASES("1920x1080@", "1080p"),
  // Super high resolutions.
  ADD_FORMAT_ALIASES("4096x2160@", "2160p"), // DCI 4k
  ADD_FORMAT_ALIASES("7680x4320@", "4320p"),

  // Other super high resolution aliases.
  ADD_FORMAT_ALIASES("2048x1080", "2k"),
  ADD_FORMAT_ALIASES("4096x2160", "4k"),
  ADD_FORMAT_ALIASES("7680x4320", "8k"),

  // Terminator
  NULL,
};

// Requirements for video;
static const gchar* requirements = 
    "video/x-raw,"
    // Required by intervideo(sink|src).
    "format=(string)I420,"
    // Square pixels, required for sanity.
    "pixel-aspect-ratio=(fraction)1/1,"
    // Smallest resolution is 300x200 (required for PIP to work).
    // Largest is 8k (arbitrarily chosen).
    "width=(int)[300,7680],"
    "height=(int)[200,4320],"
    // Pretty much any frame rate is supported, restricting to 1000fps to catch
    // user error.
    "framerate=(fraction)[0/1,1000/1]";

#define TMP_BUF_SIZE 255

// Parse a format string into it's bits.
//  [Resolution X]x[Resolution Y]@[Frame Rate]
//
// Both x and y resolution must be whole integers, frame rate can be floating
// value.
gboolean parse_short_format(const gchar *format, GstCaps *caps) {
  g_assert(gst_caps_is_writable(caps));

  gsize format_len = strlen(format);
  gchar format_buf[TMP_BUF_SIZE];
  memset(format_buf, '\0', TMP_BUF_SIZE);  

  // Convert an alias into a full format string; 
  //  VGA@60 -> 640x480@60
  //  PAL    -> 720x576@25
  //  720p25 -> 1280x720@25
  for (gsize i = 0; format_aliases[i].format != NULL; i++) {
    const gchar* alias_format = format_aliases[i].format;
    gsize alias_format_len = strlen(alias_format);

    const gchar** shortcuts = format_aliases[i].shortcuts;
    for (gsize j = 0; shortcuts[j] != NULL; j++) {
      const gchar* shortcut = shortcuts[j];
      gsize shortcut_len = strlen(shortcut);

      if (g_ascii_strncasecmp(shortcuts[j], format, shortcut_len) == 0) {
        printf("Found alias %p '%s' ('%s') in format '%s'\n", shortcut, shortcut, alias_format, format);
        strncpy(format_buf, alias_format, alias_format_len);
        strncpy(format_buf+alias_format_len, format+shortcut_len, format_len-shortcut_len);
        goto parse_short_format_found_alias;
      }
    }
  }

  // No alias found, just use the given value
  printf("No alias found!\n");
  strncpy(format_buf, format, format_len);

parse_short_format_found_alias:
  printf("Current format string: '%s'\n", format_buf);

  gint format_width = 0;
  gint format_height = 0;
  gdouble format_rate = 0;
  gint format_rate_num = 0;
  gint format_rate_den = 0;
  gint r = sscanf(format_buf, "%dx%d@%lf/%d", &format_width, &format_height, &format_rate, &format_rate_den);
  if (r == 3) {
    gst_util_double_to_fraction(format_rate, &format_rate_num, &format_rate_den);
  } else if (r == 4) {
    format_rate_num = gst_gdouble_to_guint64(format_rate);
    // Check the double was really an int
    if (gst_guint64_to_gdouble(format_rate_num) != format_rate) {
      return FALSE;
    }
  } else {
    // Wasn't able to parse the format format.
    printf("r: %d\n", r);
    return FALSE;
  }

  gst_caps_set_simple(caps,
    "framerate", GST_TYPE_FRACTION, format_rate_num, format_rate_den,
    "width", G_TYPE_INT, format_width,
    "height", G_TYPE_INT, format_height,
    NULL);

  return TRUE;
} 

// Smallest resolution is 300x200 (required for PIP to work) and largest is
// 8k (arbitrarily chosen).
static int parse_format (const gchar *format)
{
  int r = -1;
  GstCaps *require_caps = gst_caps_from_string(requirements);
  GstCaps *incoming_caps = NULL;

  gsize format_end = strlen(format);
  if (format_end >= TMP_BUF_SIZE) {
    // Invalid
    return -1;
  }

  incoming_caps = gst_caps_new_simple ("video/x-raw", 
    "format", G_TYPE_STRING, "I420",
    "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
    NULL);
  g_assert(incoming_caps);

  // Is the format already in gstreamer caps format, or in the simple
  // abbreviation format.
  if (strstr(format, "video/x-raw") == NULL) {
    if(!parse_short_format(format, incoming_caps))
      goto parse_format_error;
  } else {
    GstCaps *parsed_caps = gst_caps_from_string(format);
    if (parsed_caps == NULL) {
      printf("parse failed!?\n");
      goto parse_format_error;
    }
    printf("  parsed-caps: %s\n", gst_caps_to_string(parsed_caps));

    // Check the incoming caps are fully specified (IE only one format, don't
    // have any ranges, etc).
    if (!gst_caps_is_fixed(parsed_caps)) {
      printf("Caps were not fixed!\n");
      gst_caps_unref(parsed_caps); 
      goto parse_format_error;
    }

    GstCaps *merged_caps = gst_caps_intersect(incoming_caps, parsed_caps);
    printf("  merged-caps: %s\n", gst_caps_to_string(merged_caps));
    incoming_caps = merged_caps;
//    GstCaps *merged_caps = gst_caps_intersect(incoming_caps, parsed_caps); 
  }

  //GST_LOG ("caps are %" GST_PTR_FORMAT, caps);
  printf(" caps-to-test: %s\n", gst_caps_to_string(incoming_caps));
  printf("required-caps: %s\n", gst_caps_to_string(require_caps));

  GstCaps *final_caps = gst_caps_intersect(require_caps, incoming_caps);
  printf("   final-caps: %s\n", gst_caps_to_string(final_caps));
  if (!gst_caps_is_empty(final_caps) && gst_caps_is_fixed(final_caps)) {
    r = 0;
  }

parse_format_error:
  if (require_caps != NULL)
    gst_caps_unref(require_caps);
  if (incoming_caps != NULL)
    gst_caps_unref(incoming_caps);

  printf("\n\n\n");
  return r;
}

int main(int argc, char *argv[]) {
  gst_init (&argc, &argv);

  g_assert_cmpint(parse_format("debug"), ==, 0);
  g_assert_cmpint(parse_format("pal"), ==, 0);
  g_assert_cmpint(parse_format("720p60"), ==, 0);
  g_assert_cmpint(parse_format("1024x768@60"), ==, 0);
  g_assert_cmpint(parse_format("VGA@60"), ==, 0);
  g_assert_cmpint(parse_format("4k@60"), ==, 0);
  g_assert_cmpint(parse_format("video/x-raw,height=400,width=500,framerate=25/1"), ==, 0);

  g_assert_cmpint(parse_format("video/x-raw,height=[400,800],width=500,framerate=25/1"), ==, -1);
  g_assert_cmpint(parse_format("720p@75"), ==, -1);
  g_assert_cmpint(parse_format("sadfasf"), ==, -1);
  g_assert_cmpint(parse_format("video/x-raw,height=10,width=500,framerate=25/1"), ==, -1);
  g_assert_cmpint(parse_format("video/x-raw,height=400,width=10,framerate=25/1"), ==, -1);
  g_assert_cmpint(parse_format("video/x-raw,height=400,width=10,framerate=1001/1"), ==, -1);
  g_assert_cmpint(parse_format("pal@75"), ==, -1);

}
