// 例1
#include <string.h>
#include <stdio.h>
int main()
{  
    char s[] = "tmp.txt yyy.txt";  
    char *delim = " ";  
    char *p;  
    printf("%s hhh", strtok(s, delim));  
 
    while((p = strtok(NULL, delim)))    
        printf("%s ", p);   
 
    printf("\n");
}
