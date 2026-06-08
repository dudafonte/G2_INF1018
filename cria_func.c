/* Lívia Ramos Vallim            2320338 3WA */
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
    /* 2 - carrega valores nos registradores para chamar f */
    int param_slot = 0;
    unsigned char fix_op[] = {0xBF, 0xBE, 0xBA};
    for (j = 0; j < n; j++) {
        if (params[j].orig_val == PARAM) {
            codigo[i++] = 0x48;
            codigo[i++] = 0x8B;
            codigo[i++] = salva_hi[j];          // destino: rdi/rsi/rdx baseado em j
            codigo[i++] = (unsigned char) offsets[param_slot]; // fonte: slot salvo
            param_slot++;
        } else if (params[j].orig_val == FIX) {

        if (params[j].tipo_val == INT_PAR) {
            /* mov $val32, %eXX  — 5 bytes, sem REX */
            codigo[i++] = fix_op[j];
            *((int*)(codigo+i)) = params[j].valor.v_int;
            i += 4;

        } else {
            /* movabs $ptr64, %rXX  — 10 bytes, com REX.W */
            codigo[i++] = 0x48;
            codigo[i++] = fix_op[j];
            *((void**)(codigo+i)) = params[j].valor.v_ptr;
            i += 8;
        }    
      }  else if (params[j].orig_val == IND) {
           unsigned char ind_op_int[] = {0x38, 0x30, 0x10}; // ModRM: (%rax)→edi/esi/edx
           unsigned char ind_op_ptr[] = {0x38, 0x30, 0x10}; // mesmo ModRM, mas com REX

          /* movabs $addr, %rax */
          codigo[i++] = 0x48;
          codigo[i++] = 0xB8;
          *((void**)(codigo+i)) = params[j].valor.v_ptr;
         i += 8;

          if (params[j].tipo_val == INT_PAR) {
                 /* mov (%rax), %eXX */
                codigo[i++] = 0x8B;
                codigo[i++] = ind_op_int[j];
            } else {
                /* mov (%rax), %rXX */
                codigo[i++] = 0x48;
                codigo[i++] = 0x8B;
                codigo[i++] = ind_op_ptr[j];
    }
}
    }   

    /* chama f */
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