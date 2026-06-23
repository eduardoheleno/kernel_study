#include "unistd.h"
#include "stdio.h"
#include <stddef.h>

int main(void)
{
    char buffer[100];
    printf("Digite seu nome: \n");
    read(0, buffer, 100);
    printf("Seu nome é: \n");
    printf(buffer);
}
