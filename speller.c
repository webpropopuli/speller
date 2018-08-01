// Implements a spell-checker

#include <ctype.h>
#include <stdio.h>
//!  #include <sys/resource.h> // does not exist in Windows distro

#define __USE_BSD
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <sys/time.h>
#ifndef timersub
#define timersub(a, b, result) \
        do { \
                (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
                (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
                if ((result)->tv_usec < 0) { \
                        --(result)->tv_sec; \
                        (result)->tv_usec += 1000000; \
                } \
        } while (0)
#endif // timersub

#ifndef timeradd
#define timeradd(_a, _b, _res) \
  do { \
    (_res)->tv_usec = (_a)->tv_usec + (_b)->tv_usec; \
    (_res)->tv_sec = (_a)->tv_sec + (_b)->tv_sec; \
    if ((_res)->tv_usec >= 1000000) \
      { \
        (_res)->tv_usec -= 1000000; \
        (_res)->tv_sec++; \
      } \
  } while (0)
#endif

#include <assert.h>
#include <math.h>

#include "dictionary.h"

// Default dictionary
#define DICTIONARY "dictionaries/large"


int main(int argc, char *argv[])
{
struct timeval start2;
struct timeval  end2;


    // Check for correct number of args
    if (argc != 2 && argc != 3)
    {
        printf("Usage: speller [dictionary] text\n");
        return 1;
    }

    // Benchmarks
    struct timeval time_load, time_size, time_check, time_unload;
    struct timeval time_check_total;
    timerclear(&time_check_total);

    // Determine dictionary to use
    char *dictionary = (argc == 3) ? argv[1] : DICTIONARY;

    // Load dictionary

    gettimeofday(&start2, NULL);
    bool loaded = load(dictionary);
    gettimeofday(&end2, NULL);
    timersub(&end2, &start2, &time_load);

    
    // Exit if dictionary not loaded
    if (!loaded)
    {
        printf("Could not load %s.\n", dictionary);
        return 1;
    }

    // Try to open text
    char *text = (argc == 3) ? argv[2] : argv[1];
    FILE *file = fopen(text, "r");
    if (file == NULL)
    {
        printf("Could not open %s.\n", text);
        unload();
        return 1;
    }

    // Prepare to report misspellings
    printf("\nMISSPELLED WORDS\n\n");

    // Prepare to spell-check
    int index = 0, misspellings = 0, words = 0;
    char word[LENGTH + 1];

    // Spell-check each word in text
    for (int c = fgetc(file); c != EOF; c = fgetc(file))
    {
        // Allow only alphabetical characters and apostrophes
        if (isalpha(c) || (c == '\'' && index > 0))
        {
            // Append character to word
            word[index] = c;
            index++;

            // Ignore alphabetical strings too long to be words
            if (index > LENGTH)
            {
                // Consume remainder of alphabetical string
                while ((c = fgetc(file)) != EOF && isalpha(c));

                // Prepare for new word
                index = 0;
            }
        }

        // Ignore words with numbers (like MS Word can)
        else if (isdigit(c))
        {
            // Consume remainder of alphanumeric string
            while ((c = fgetc(file)) != EOF && isalnum(c));

            // Prepare for new word
            index = 0;
        }

        // We must have found a whole word
        else if (index > 0)
        {
            // Terminate current word
            word[index] = '\0';

            // Update counter
            words++;

            // Check word's spelling
            gettimeofday(&start2, NULL);
            bool misspelled = !check(word);
            gettimeofday(&end2, NULL);
            timersub(&end2, &start2, &time_check);
            // Update benchmark total time
            timeradd(&time_check_total, &time_check, &time_check_total);
            

            // Print word if misspelled
            if (misspelled)
            {
                //printf("%s\n", word);
                misspellings++;
            }

            // Prepare for next word
            index = 0;
        }
    }

    // Check whether there was an error
    if (ferror(file))
    {
        fclose(file);
        printf("Error reading %s.\n", text);
        unload();
        return 1;
    }

    // Close text
    fclose(file);

    // Determine dictionary's size
    gettimeofday(&start2, NULL);
    unsigned int n = size();
    gettimeofday(&end2, NULL);
    timersub(&end2, &start2, &time_size);

    // Unload dictionary
    gettimeofday(&start2, NULL);
    bool unloaded = unload();
    gettimeofday(&end2, NULL);
    timersub(&end2, &start2, &time_unload);

    // Abort if dictionary not unloaded
    if (!unloaded)
    {
        printf("Could not unload %s.\n", dictionary);
        return 1;
    }

    // Report benchmarks
    printf("\nWORDS MISSPELLED:   %d\n", misspellings);
    printf("WORDS IN DICTIONARY:  %d\n", n);
    printf("WORDS IN TEXT:        %d\n", words);
    printf("TIME IN load:         %ld.%06ld s\n",  time_load.tv_sec, time_load.tv_usec);
    printf("TIME IN check:        %ld.%06ld s\n",  time_check_total.tv_sec, time_check_total.tv_usec);
    printf("TIME IN size:         %ld.%06ld s\n",  time_size.tv_sec, time_size.tv_usec);
    printf("TIME IN unload:       %ld.%06ld s\n",  time_unload.tv_sec, time_unload.tv_usec);
    
    timeradd(&time_check_total, &time_load, &time_check_total);
    timeradd(&time_check_total, &time_size, &time_check_total);
    timeradd(&time_check_total, &time_unload, &time_check_total);
    printf("TIME IN TOTAL:        %ld.%06ld s\n", time_check_total.tv_sec, time_check_total.tv_usec);

    // Success
    return 0;
}

