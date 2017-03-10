/*
 Bailey Kane
 February 3, 2017
 
 HW04 - Sort Strings in C
 
 Sorts user-input strings up to max_in characters.
 
 Usage:
    ./sort-strings {number of strings to be input}
    {string 1}
    {string 2}
    etc...
*/

// Includes for standard I/O, string processing, and general functions.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Wrapper around strcmp to convert type properly
int compare_strings(const void *a, const void *b) {
    return strcmp(a, b);
}

int main(int argc, char *argv[]) {
    // Counter for string input for loop.
    int i;
    // Maximum input size.
    int max_in = 80;
    // Number of strings to be input, from CLI.
    // Use atoi() to convert ASCII input to an integer.
    int num_input = atoi(argv[1]);
    // Buffer to store each individual string. Transferred later to strings[].
    // Memory allocated, freed later.
    char* buffer;
    buffer = (char*) malloc(max_in + 1);
    // Array to store strings
    char strings[num_input][max_in];
    
    // loop to receive user input on strings for num_input strings.
    for (i = 0; i < num_input; i++) {
        fgets(buffer, max_in, stdin);
        
        strcpy(strings[i], buffer);
    }
    
    // Free memory allocated to buffer.
    free(buffer);
    
    // Use quicksort to sort strings. 
    qsort(strings, num_input, max_in * sizeof(char), compare_strings);
    
    // Print sorted strings.
    for (i = 0; i < num_input; i++) {
        printf("%s", strings[i]);
    }
    
    // Return with success status.
    return(0);
}