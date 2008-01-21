/*
 * watconvert.cpp
 *
 * wmake uses '&' as a line continuation however objs.inc
 * uses '\' so this executable will read in objs.inc, convert
 * the file, and save as objs.mif for use by wmake
 *
 */

#include <stdio.h>
#include <stdlib.h>

void main( )
{
    int  i;

    char buffer[30];

    FILE *input;
    FILE *output;

    input = fopen("objs.inc", "r");
    if(input == NULL)
    {
        printf("objs.inc file not found\n");
        exit(1);
    }

    output = fopen("objs.mif", "w");
    if(input == NULL)
    {
        printf("objs.mif not writable\n");
        exit(1);
    }

    while(fgets(buffer, 30, input) != NULL)
    {
        for(i=0; i<30; i++) if(buffer[i] == '\\') buffer[i] = '&';
        fputs(buffer, output);
    }

    fclose(input);
    fclose(output);    
}
