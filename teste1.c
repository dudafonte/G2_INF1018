/* teste2.c - testes de stress para cria_func
 *
 * Compilar com:
 *   gcc -Wall -Wa,--execstack -o teste2 cria_func.c teste2.c
 *
 * Foco: valores negativos, INT_MIN/INT_MAX, ponteiros "longe" (heap),
 *       retorno de ponteiro, chamadas a funcoes de biblioteca dentro
 *       da funcao-alvo, e estabilidade em muitas chamadas.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "cria_func.h"

static int total = 0;
static int passou = 0;

#define CHECK(desc, got, exp)                                       \
    do {                                                            \
        total++;                                                    \
        long _g = (long)(got);                                      \
        long _e = (long)(exp);                                      \
        if (_g == _e) {                                             \
            passou++;                                               \
            printf("  [OK]   %s  (obtido=%ld)\n", desc, _g);        \
        } else {                                                    \
            printf("  [FALHA] %s  (obtido=%ld, esperado=%ld)\n",    \
                   desc, _g, _e);                                   \
        }                                                           \
    } while (0)


/* ---------- funcoes-alvo ---------- */

int identidade(int x) { return x; }

int ret_neg(int x) { return -x; }    /* inverte o sinal */

int mult(int x, int y) { return x * y; }

int soma3(int a, int b, int c) { return a + b + c; }

int deref(int *p) { return *p; }

/* devolve o MESMO ponteiro recebido (testa preservacao do retorno em rax) */
int *id_ptr(int *p) { return p; }

/* chama snprintf internamente -> forca uma chamada de biblioteca "pesada",
   bom para estressar o alinhamento de pilha. Retorna nro de digitos. */
int conta_digitos(int x) {
    char buf[64];
    snprintf(buf, sizeof buf, "%d", x);
    return (int) strlen(buf);
}


/* ====================================================================== */

int main(void) {
    unsigned char codigo[500];
    unsigned char codigo2[500];   /* segundo buffer, para teste de coexistencia */
    DescParam params[3];

    printf("==============================================\n");
    printf(" TESTES DE STRESS - cria_func\n");
    printf("==============================================\n");


    /* -------------------------------------------------------------- */
    /* 1) FIX com valores extremos de int                              */
    /* -------------------------------------------------------------- */
    printf("\n[1] identidade() com FIX em valores extremos:\n");
    {
        typedef int (*fp)(void);

        params[0].tipo_val = INT_PAR;
        params[0].orig_val = FIX;

        params[0].valor.v_int = INT_MAX;
        cria_func(identidade, params, 1, codigo);
        CHECK("FIX = INT_MAX", ((fp)codigo)(), INT_MAX);

        params[0].valor.v_int = INT_MIN;
        cria_func(identidade, params, 1, codigo);
        CHECK("FIX = INT_MIN", ((fp)codigo)(), INT_MIN);

        params[0].valor.v_int = -1;
        cria_func(identidade, params, 1, codigo);
        CHECK("FIX = -1", ((fp)codigo)(), -1);

        params[0].valor.v_int = -123456789;
        cria_func(identidade, params, 1, codigo);
        CHECK("FIX = -123456789", ((fp)codigo)(), -123456789);
    }


    /* -------------------------------------------------------------- */
    /* 2) PARAM com entradas negativas (e retorno negativo)           */
    /* -------------------------------------------------------------- */
    printf("\n[2] ret_neg(x) com PARAM negativo:\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = PARAM;
        cria_func(ret_neg, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("ret_neg(5)", f(5), -5);
        CHECK("ret_neg(-5)", f(-5), 5);
        CHECK("ret_neg(INT_MAX)", f(INT_MAX), -INT_MAX);
        CHECK("ret_neg(0)", f(0), 0);
    }


    /* -------------------------------------------------------------- */
    /* 3) mult(x, FIX) com fator negativo e valores grandes           */
    /* -------------------------------------------------------------- */
    printf("\n[3] mult(x, -7) com PARAM + FIX negativo:\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
        params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
        params[1].valor.v_int = -7;

        cria_func(mult, params, 2, codigo);
        fp f = (fp) codigo;

        CHECK("mult(3,-7)", f(3), -21);
        CHECK("mult(-4,-7)", f(-4), 28);
        CHECK("mult(1000,-7)", f(1000), -7000);
    }


    /* -------------------------------------------------------------- */
    /* 4) IND com variavel alternando INT_MIN / INT_MAX               */
    /* -------------------------------------------------------------- */
    printf("\n[4] identidade() com IND alternando extremos:\n");
    {
        typedef int (*fp)(void);
        int v;
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = IND;
        params[0].valor.v_ptr = &v;
        cria_func(identidade, params, 1, codigo);
        fp f = (fp) codigo;

        v = INT_MAX; CHECK("IND v=INT_MAX", f(), INT_MAX);
        v = INT_MIN; CHECK("IND v=INT_MIN", f(), INT_MIN);
        v = -1;      CHECK("IND v=-1",      f(), -1);
    }


    /* -------------------------------------------------------------- */
    /* 5) soma3 com mistura PARAM/FIX/IND e valores negativos         */
    /*    f(a) chama soma3(a, -1000, v)                                */
    /* -------------------------------------------------------------- */
    printf("\n[5] soma3(a, -1000, v) PARAM + FIX + IND:\n");
    {
        typedef int (*fp)(int);
        int v = 50;
        params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
        params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
        params[1].valor.v_int = -1000;
        params[2].tipo_val = INT_PAR; params[2].orig_val = IND;
        params[2].valor.v_ptr = &v;

        cria_func(soma3, params, 3, codigo);
        fp f = (fp) codigo;

        CHECK("soma3(100,-1000,50)", f(100), -850);
        v = -50;
        CHECK("soma3(100,-1000,-50)", f(100), -950);
    }


    /* -------------------------------------------------------------- */
    /* 6) ponteiro de retorno preservado (id_ptr)                     */
    /* -------------------------------------------------------------- */
    printf("\n[6] id_ptr(p) - retorno de ponteiro preservado:\n");
    {
        typedef int* (*fp)(int*);
        int x = 0;
        params[0].tipo_val = PTR_PAR;
        params[0].orig_val = PARAM;
        cria_func(id_ptr, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("id_ptr(&x) == &x", (long)f(&x), (long)&x);
    }


    /* -------------------------------------------------------------- */
    /* 7) ponteiro FIX para memoria alocada no heap (endereco "longe") */
    /* -------------------------------------------------------------- */
    printf("\n[7] deref() com PTR_PAR FIX apontando para heap:\n");
    {
        typedef int (*fp)(void);
        int *heap = malloc(sizeof(int));
        if (!heap) { printf("  malloc falhou\n"); }
        else {
            *heap = 0x7FABCDEF;
            params[0].tipo_val = PTR_PAR;
            params[0].orig_val = FIX;
            params[0].valor.v_ptr = heap;
            cria_func(deref, params, 1, codigo);
            fp f = (fp) codigo;

            CHECK("deref(heap) = 0x7FABCDEF", f(), 0x7FABCDEF);
            *heap = -2000000000;
            CHECK("deref(heap) = -2000000000", f(), -2000000000);
            free(heap);
        }
    }


    /* -------------------------------------------------------------- */
    /* 8) funcao-alvo que chama biblioteca (snprintf) - alinhamento   */
    /* -------------------------------------------------------------- */
    printf("\n[8] conta_digitos(x) - alvo chama snprintf internamente:\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = PARAM;
        cria_func(conta_digitos, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("digitos(0)", f(0), 1);
        CHECK("digitos(42)", f(42), 2);
        CHECK("digitos(1000000)", f(1000000), 7);
        CHECK("digitos(-99)", f(-99), 3);   /* '-' conta */
        CHECK("digitos(INT_MIN)", f(INT_MIN), 11); /* -2147483648 = 11 chars */
    }


    /* -------------------------------------------------------------- */
    /* 9) estabilidade: muitas chamadas seguidas                      */
    /* -------------------------------------------------------------- */
    printf("\n[9] estabilidade em 1.000.000 de chamadas:\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
        params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
        params[1].valor.v_int = 3;
        cria_func(mult, params, 2, codigo);
        fp f = (fp) codigo;

        long acc = 0;
        for (int k = 0; k < 1000000; k++) acc += f(k % 100);
        /* soma de (k%100)*3 por 1.000.000 iteracoes.
           cada bloco de 100: sum(0..99)=4950, *3 = 14850, x10000 blocos */
        CHECK("acumulado estavel", acc, 148500000L);
    }


    /* -------------------------------------------------------------- */
    /* 10) duas funcoes geradas coexistindo em buffers diferentes     */
    /* -------------------------------------------------------------- */
    printf("\n[10] duas funcoes geradas ao mesmo tempo:\n");
    {
        typedef int (*fp)(int);

        /* f1: mult(x, 2) */
        params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
        params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
        params[1].valor.v_int = 2;
        cria_func(mult, params, 2, codigo);
        fp f1 = (fp) codigo;

        /* f2: mult(x, 100) num buffer separado */
        params[1].valor.v_int = 100;
        cria_func(mult, params, 2, codigo2);
        fp f2 = (fp) codigo2;

        /* intercala chamadas - uma nao pode atrapalhar a outra */
        CHECK("f1(5)=10",   f1(5),   10);
        CHECK("f2(5)=500",  f2(5),   500);
        CHECK("f1(9)=18",   f1(9),   18);
        CHECK("f2(9)=900",  f2(9),   900);
    }


    /* ====================== resumo ====================== */
    printf("\n==============================================\n");
    printf(" RESULTADO: %d/%d testes passaram\n", passou, total);
    if (passou == total)
        printf(" TUDO OK :)\n");
    else
        printf(" HÁ FALHAS - %d teste(s) falharam\n", total - passou);
    printf("==============================================\n");

    return (passou == total) ? 0 : 1;
}