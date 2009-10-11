/*
 * watconvert.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 * wmake uses '&' as a line continuation however objs.inc
 * uses '\' so this executable will read in objs.inc, convert
 * the file, and save as objs.mif for use by wmake
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
    int  i;

    char buffer[30];

    FILE *input;
    FILE *output;

    input = fopen("objs.inc", "r");
    if (input == NULL) {
        printf("objs.inc file not found\n");
        return EXIT_FAILURE;
    }

    output = fopen("objs.mif", "w");
    if (output == NULL) {
        fclose(input);
        printf("objs.mif not writable\n");
        return EXIT_FAILURE;
    }

    while (fgets(buffer, 30, input) != NULL) {
        for (i = 0; i < 30; i++) if (buffer[i] == '\\') buffer[i] = '&';
        fputs(buffer, output);
    }

    fclose(input);
    fclose(output);
    return EXIT_SUCCESS;
}
