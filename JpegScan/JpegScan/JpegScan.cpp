/*
 * libexif example program to display the contents of a number of specific
 * EXIF and MakerNote tags. The tags selected are those that may aid in
 * identification of the photographer who took the image.
 *
 * Placed into the public domain by Dan Fandrich
 */

#include <stdio.h>
#include <string.h>
#include <libexif/exif-data.h>
#include "getopt.h"

/* Remove spaces on the right of the string */
static void trim_spaces(char *buf)
{
    char *s = buf-1;
    for (; *buf; ++buf) {
        if (*buf != ' ')
            s = buf;
    }
    *++s = 0; /* nul terminate the string on the first of the final spaces */
}

/* Show the tag name and contents if the tag exists */
static void show_tag(ExifData *d, ExifIfd ifd, ExifTag tag)
{
    /* See if this tag exists */
    ExifEntry *entry = exif_content_get_entry(d->ifd[ifd],tag);
    if (entry) {
        char buf[1024];

        /* Get the contents of the tag in human-readable form */
        exif_entry_get_value(entry, buf, sizeof(buf));

        /* Don't bother printing it if it's entirely blank */
        trim_spaces(buf);
        if (*buf) {
            printf("%s: %s\n", exif_tag_get_name_in_ifd(tag,ifd), buf);
        }
    }
}

/* Show the given MakerNote tag if it exists */
static void show_mnote_tag(ExifData *d, unsigned tag)
{
    ExifMnoteData *mn = exif_data_get_mnote_data(d);
    if (mn) {
        int num = exif_mnote_data_count(mn);
        int i;

        /* Loop through all MakerNote tags, searching for the desired one */
        for (i=0; i < num; ++i) {
            char buf[1024];
            if (exif_mnote_data_get_id(mn, i) == tag) {
                if (exif_mnote_data_get_value(mn, i, buf, sizeof(buf))) {
                    /* Don't bother printing it if it's entirely blank */
                    trim_spaces(buf);
                    if (*buf) {
                        printf("%s: %s\n", exif_mnote_data_get_title(mn, i),
                            buf);
                    }
                }
            }
        }
    }
}

static const char * g_opts = "h?f:d";
static char * fileName;
static bool dumpMode;

int main(int argc, char **argv)
{
    ExifData *ed;
    //ExifEntry *entry;

    if (argc < 2) {
        printf("Usage: %s image.jpg\n", argv[0]);
        printf("Displays tags potentially relating to ownership "
                "of the image.\n");
        return 1;
    }

	while (1) {
		switch (int opt = getopt(argc, argv, g_opts)) {
		case '?':
		case 'h':
			// help;
			return 0;

		case 'd':
			dumpMode = true;
			continue;

		case 'f':
			fileName = optarg;
			continue;
		}
		break;
	}
    /* Load an ExifData object from an EXIF file */
    if (!(ed = exif_data_new_from_file(fileName))) {
        printf("File not readable or no EXIF data in file %s\n", argv[1]);
        return 2;
    }

#if 0
	/* Show all the tags that might contain information about the
     * photographer
     */
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_ARTIST);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_XP_AUTHOR);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_COPYRIGHT);

    /* These are much less likely to be useful */
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_USER_COMMENT);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_IMAGE_DESCRIPTION);
    show_tag(ed, EXIF_IFD_1, EXIF_TAG_IMAGE_DESCRIPTION);

#else
	exif_data_dump(ed);
#endif

    /* Free the EXIF data */
    exif_data_unref(ed);

    return 0;
}
