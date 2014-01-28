#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <io.h>

#include <libexif/exif-data.h>

#include <vector>
#include <array>
#include <algorithm>
#include <locale>
#include <iomanip>
#include <sstream>
#include <ctime>

#include "getopt.h"

const char * fileNamePattern = "%04d-%02d-%02d";

std::tm xlate_datetime(const std::string& s)
{
    std::tm t = {};
    std::istringstream ss(s);
    ss >> std::get_time(&t, "%Y:%m:%d %H:%M:%S");
    return t;
}


std::string format_path_tm(const std::tm & tm)
{
    std::array<char, 256> buffer;
    sprintf_s(buffer.data(), buffer.size(), fileNamePattern, 
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return buffer.data();
}

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

static std::string get_tag_value(ExifData *d, ExifIfd ifd, ExifTag tag) 
{
    std::string s_temp;

    ExifEntry *entry = exif_content_get_entry(d->ifd[ifd],tag);
    if (entry) {
        std::array<char, 1024> buffer;

        /* Get the contents of the tag in human-readable form */
        exif_entry_get_value(entry, buffer.data(), buffer.max_size());

        trim_spaces(buffer.data());
        size_t len = strlen(buffer.data());
        s_temp = buffer.data();
    }
    return s_temp;
}

/* Show the tag name and contents if the tag exists */
static void show_tag(ExifData *d, ExifIfd ifd, ExifTag tag)
{
    std::string&& buffer = get_tag_value(d, ifd, tag);

    if (!buffer.empty()) {
        printf("%s: %s\n", exif_tag_get_name_in_ifd(tag, ifd), buffer.data());
    }
}

static char noexifSubdir[] = "_notimeinfo_";

static const char * g_opts = "h?f:dg";
static char * fileName;
static bool dumpMode;
static bool genMode;

static void move_file(const char * fileName, const char* transName)
{
    printf("mkdir %s > NUL && move %s %s\\ \n", transName, fileName, transName);
}

static void move_nodate_file(const char *fileName, const char *defaultDir = noexifSubdir)
{
    // don't certain yet what to do with no exif date files
    printf("echo no time info for file %s \n", fileName);
}

static void not_existing_file(const char * fileName)
{
    printf("echo not existing file %s \n", fileName);
}

void process_datestamp_data (const char * fileName, ExifData *data)
{
#if 0
    show_tag(data, EXIF_IFD_0, EXIF_TAG_DATE_TIME_DIGITIZED);
    show_tag(data, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    show_tag(data, EXIF_IFD_0, EXIF_TAG_DATE_TIME_ORIGINAL);
    //	show_tag(data, EXIF_IFD_GPS, (ExifTag)EXIF_TAG_GPS_DATE_STAMP);
    //	show_tag(data, EXIF_IFD_GPS, (ExifTag)EXIF_TAG_GPS_TIME_STAMP);
#endif
    std::string&& tag_string = get_tag_value(data, EXIF_IFD_0, EXIF_TAG_DATE_TIME_DIGITIZED);
    if (tag_string.empty())
        tag_string = get_tag_value(data, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    if (tag_string.empty())
        tag_string = get_tag_value(data, EXIF_IFD_0, EXIF_TAG_DATE_TIME_ORIGINAL);
    assert(!tag_string.empty());

    std::tm&& xlate_tm = xlate_datetime(tag_string);
    move_file(fileName, format_path_tm(xlate_tm).c_str());
}

int main(int argc, char **argv)
{
    ExifData *ed;

    if (argc < 2) {
        printf("Usage: %s image.jpg\n", argv[0]);
        printf("Displays tags potentially relating to ownership "
            "of the image.\n");
        return 1;
    }

    int opt;
    while (-1 != (opt = getopt(argc, argv, g_opts))) {
        switch (opt) {
        case '?':
        case 'h':
            // help;
            return 0;

        case 'd':
            dumpMode = true;
            continue;

        case 'g':
            genMode = true;
            continue;

        case 'f':
            fileName = optarg;
            continue;
        }
        break;
    }

    do 
    {
        if (-1 == _access(fileName, 0)) {
            not_existing_file(fileName);
            goto next_file;
        }
        /* Load an ExifData object from an EXIF file */
        if (!(ed = exif_data_new_from_file(fileName))) {
            //printf("File not readable or no EXIF data in file %s\n", fileName);
            move_nodate_file(fileName);
            goto next_file;
        } 

        if (dumpMode) {
            exif_data_dump(ed);
        } else if (genMode) {
            process_datestamp_data(fileName, ed);
        }
        /* Free the EXIF data */
        exif_data_unref(ed);


next_file:
        fileName = argv[optind++]; // process next free argument in the command-line (wildcard expansion most-probably)

    } while (optind <= argc);
    return 0;
}
