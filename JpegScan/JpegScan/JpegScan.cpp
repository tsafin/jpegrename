// JpegScan.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>

#include <libexif/exif-data.h>

int	main (int argc, char **argv)
{
	ExifData *d;
	unsigned int buf_size;
	unsigned char *buf;
	int r;

	if (argc <= 1) {
		fprintf (stderr, "You need to supply a filename!\n");
		return 1;
	}

	fprintf (stdout, "Loading '%s'...\n", argv[1]);
	d = exif_data_new_from_file (argv[1]);
	if (!d) {
		fprintf (stderr, "Could not load data from '%s'!\n", argv[1]);
		return 1;
	}
	fprintf (stdout, "Loaded '%s'.\n", argv[1]);

	fprintf (stdout, "######### Test 1 #########\n");
	//r = test_exif_data (d);
	//if (r) return r;

	exif_data_save_data (d, &buf, &buf_size);
	exif_data_unref (d);
	d = exif_data_new_from_data (buf, buf_size);
	free (buf);

	fprintf (stdout, "######### Test 2 #########\n");
	//r = test_exif_data (d);
	//if (r) return r;

	fprintf (stdout, "Test successful!\n");

	return 1;
}

