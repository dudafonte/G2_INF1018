/* Lívia Ramos Vallim            2300000 3WA */
/* Maria Eduarda Fonte de Macedo 2320450 3WA */

#include "cria_func.h"
#include <string.h>

void cria_func(void* f, DescParam params[], int n, unsigned char codigo[]) 
{
    int i = 0;
    int j;
    int param_count = 0;
    int offsets[] = {-8, -16, -24};
    unsigned char salva_hi[] = {0x7D, 0x75, 0x55};

    /*prólogo*/
    codigo[i++] = 0x55;
    codigo[i++] = 0x48;
    codigo[i++] = 0x89;
    codigo[i++] = 0xE5;
    codigo[i++] = 0x48;
    codigo[i++] = 0x83;
    codigo[i++] = 0xEC;
    codigo[i++] = 0x20;

    /* 1 - salva PARAMs recebidos na pilha */
    for (j=0; j<n; j++)
    {
        if (params[j].orig_val == PARAM)
        {
            codigo[i++] = 0x48;
            codigo[i++] = 0x89;
            codigo[i++] = salva_hi[param_count];
            codigo[i++] = (unsigned char) offsets[param_count];
            param_count++;
        }
    }

    /* cahma f */
    codigo[i++] = 0x48;
    codigo[i++] = 0xB8;
    *((void**)(codigo+i)) = f;
    i += 8;
    codigo[i++] = 0xFF;
    codigo[i++] = 0xD0;

    /*epílogo*/
    codigo[i++] = 0xC9;
    codigo[i++] = 0xC3;
}