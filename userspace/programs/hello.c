#include "stdio.h"
#include "unistd.h"
#include <stddef.h>

int main(void)
{
    char name[100];
    read(0, name, 100);
    printf("Seu nome: %s", name);
}
