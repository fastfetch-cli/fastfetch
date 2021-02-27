#include "fastfetch.h"

#include <dirent.h>

static uint32_t get_num_dirs(const char* dirname) {
    uint32_t num_dirs = 0;
    DIR * dirp;
    struct dirent *entry;

    dirp = opendir(dirname);
    if(dirp == NULL)
        return 0;

    while((entry = readdir(dirp)) != NULL) {
        if(entry->d_type == DT_DIR)
            ++num_dirs;
    }
   
    num_dirs -= 2; // accounting for . and ..

    closedir(dirp);

    return num_dirs;
}


static void printPacmanPackages()
{
    uint32_t nums = get_num_dirs("/var/lib/pacman/local");
    if(nums > 0)
        printf("%i (pacman) ", nums);
}

void ffPrintPackages(FFstate* state)
{
    ffPrintLogoAndKey(state, "Packages");
    printPacmanPackages();
    putchar('\n');
}