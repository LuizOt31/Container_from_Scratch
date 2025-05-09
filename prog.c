/*
Este programa é um teste tentando rodar ele dentro do container
*/
#include<stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    
    int aux = atoi(argv[1]);
    
    for (int i = 0; i < 50; i ++)
        printf("numero aux: %d\n", aux);

    return 0;
}