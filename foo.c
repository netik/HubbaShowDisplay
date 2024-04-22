#include <string.h>
#include <stdio.h>

int  main () {
    char str[] = "WIFI";

    printf("%d", strncmp(str, "WIFI",4)); 
}
