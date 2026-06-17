/* teste.c - testes para cria_func
 *
 * Compilar com:
 *   gcc -Wall -Wa,--execstack -o teste cria_func.c teste.c
 *
 * Rodar:
 *   ./teste
 */

#include <stdio.h>
#include <string.h>
#include "cria_func.h"

/* contadores globais de teste */
static int total = 0;
static int passou = 0;

/* macro simples de verificacao */
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


/* ---------- funcoes-alvo que serao chamadas pelo codigo gerado ---------- */

int mult(int x, int y) {
    return x * y;
}

int soma3(int a, int b, int c) {
    return a + b + c;
}

int identidade(int x) {
    return x;
}

/* recebe um ponteiro e retorna o int apontado */
int deref(int *p) {
    return *p;
}

/* conta o tamanho de uma string (igual strlen, mas nossa) */
int tam_str(char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}


/* ====================================================================== */

int main(void) {
    unsigned char codigo[500];
    DescParam params[3];

    printf("==============================================\n");
    printf(" TESTES DE cria_func\n");
    printf("==============================================\n");


    /* -------------------------------------------------------------- */
    /* 1) identidade(x) - 1 param PARAM, sem amarrar                    */
    /* -------------------------------------------------------------- */
    printf("\n[1] identidade(x) com PARAM:\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = PARAM;

        cria_func(identidade, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("identidade(7)", f(7), 7);
        CHECK("identidade(-42)", f(-42), -42);
        CHECK("identidade(0)", f(0), 0);
    }


    /* -------------------------------------------------------------- */
    /* 2) identidade(x) com x amarrado a constante (FIX)               */
    /* -------------------------------------------------------------- */
    printf("\n[2] identidade() com FIX=99:\n");
    {
        typedef int (*fp)(void);
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = FIX;
        params[0].valor.v_int = 99;

        cria_func(identidade, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("identidade()->99", f(), 99);
    }


    /* -------------------------------------------------------------- */
    /* 3) identidade(x) com x amarrado a variavel (IND)                */
    /* -------------------------------------------------------------- */
    printf("\n[3] identidade() com IND (variavel):\n");
    {
        typedef int (*fp)(void);
        int v = 123;
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = IND;
        params[0].valor.v_ptr = &v;

        cria_func(identidade, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("v=123", f(), 123);
        v = 555;                       /* muda a variavel... */
        CHECK("v=555 (apos mudar)", f(), 555);  /* deve refletir */
    }


    /* -------------------------------------------------------------- */
    /* 4) mult(x,10) - PARAM + FIX (exemplo do enunciado: dezenas)     */
    /* -------------------------------------------------------------- */
    printf("\n[4] mult(x,10) com PARAM + FIX:\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = PARAM;
        params[1].tipo_val = INT_PAR;
        params[1].orig_val = FIX;
        params[1].valor.v_int = 10;

        cria_func(mult, params, 2, codigo);
        fp f = (fp) codigo;

        CHECK("mult(1,10)", f(1), 10);
        CHECK("mult(7,10)", f(7), 70);
        CHECK("mult(10,10)", f(10), 100);
    }


    /* -------------------------------------------------------------- */
    /* 5) mult(i,10) - IND + FIX (a outra versao do enunciado)         */
    /* -------------------------------------------------------------- */
    printf("\n[5] mult(i,10) com IND + FIX:\n");
    {
        typedef int (*fp)(void);
        int i;
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = IND;
        params[0].valor.v_ptr = &i;
        params[1].tipo_val = INT_PAR;
        params[1].orig_val = FIX;
        params[1].valor.v_int = 10;

        cria_func(mult, params, 2, codigo);
        fp f = (fp) codigo;

        i = 3;  CHECK("i=3  -> 30",  f(), 30);
        i = 8;  CHECK("i=8  -> 80",  f(), 80);
    }


    /* -------------------------------------------------------------- */
    /* 6) mult(FIX, PARAM) - aqui o PARAM vai para o 2o registrador!   */
    /*    testa o reordenamento: o unico PARAM chega em rdi mas        */
    /*    precisa ir para rsi                                           */
    /* -------------------------------------------------------------- */
    printf("\n[6] mult(5, x) com FIX + PARAM (testa reordenacao):\n");
    {
        typedef int (*fp)(int);
        params[0].tipo_val = INT_PAR;
        params[0].orig_val = FIX;
        params[0].valor.v_int = 5;
        params[1].tipo_val = INT_PAR;
        params[1].orig_val = PARAM;

        cria_func(mult, params, 2, codigo);
        fp f = (fp) codigo;

        CHECK("mult(5,4)", f(4), 20);
        CHECK("mult(5,9)", f(9), 45);
    }


    /* -------------------------------------------------------------- */
    /* 7) soma3(a,b,c) - 3 PARAM                                       */
    /* -------------------------------------------------------------- */
    printf("\n[7] soma3(a,b,c) com 3 PARAM:\n");
    {
        typedef int (*fp)(int,int,int);
        for (int k = 0; k < 3; k++) {
            params[k].tipo_val = INT_PAR;
            params[k].orig_val = PARAM;
        }
        cria_func(soma3, params, 3, codigo);
        fp f = (fp) codigo;

        CHECK("soma3(1,2,3)", f(1,2,3), 6);
        CHECK("soma3(10,20,30)", f(10,20,30), 60);
        CHECK("soma3(-5,5,100)", f(-5,5,100), 100);
    }


    /* -------------------------------------------------------------- */
    /* 8) soma3(PARAM, FIX, PARAM) - mistura, testa reordenacao        */
    /*    f(a,b) deve chamar soma3(a, 100, b)                          */
    /* -------------------------------------------------------------- */
    printf("\n[8] soma3(a, 100, b) - PARAM + FIX + PARAM:\n");
    {
        typedef int (*fp)(int,int);
        params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
        params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
        params[1].valor.v_int = 100;
        params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

        cria_func(soma3, params, 3, codigo);
        fp f = (fp) codigo;

        /* a chega em rdi->r10 (1o PARAM), b chega em rsi->r11 (2o PARAM) */
        CHECK("soma3(1,100,2)", f(1,2), 103);
        CHECK("soma3(10,100,20)", f(10,20), 130);
    }


    /* -------------------------------------------------------------- */
    /* 9) deref(p) com ponteiro PARAM                                  */
    /* -------------------------------------------------------------- */
    printf("\n[9] deref(p) com PTR_PAR + PARAM:\n");
    {
        typedef int (*fp)(int*);
        int v = 777;
        params[0].tipo_val = PTR_PAR;
        params[0].orig_val = PARAM;

        cria_func(deref, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("deref(&777)", f(&v), 777);
    }


    /* -------------------------------------------------------------- */
    /* 10) deref(p) com ponteiro FIX (endereco amarrado)              */
    /* -------------------------------------------------------------- */
    printf("\n[10] deref() com PTR_PAR + FIX (endereco fixo):\n");
    {
        typedef int (*fp)(void);
        int v = 314;
        params[0].tipo_val = PTR_PAR;
        params[0].orig_val = FIX;
        params[0].valor.v_ptr = &v;

        cria_func(deref, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("deref(&v) v=314", f(), 314);
        v = 271;
        CHECK("deref(&v) v=271", f(), 271);
    }


    /* -------------------------------------------------------------- */
    /* 11) tam_str(s) com ponteiro PARAM                              */
    /* -------------------------------------------------------------- */
    printf("\n[11] tam_str(s) com PTR_PAR + PARAM:\n");
    {
        typedef int (*fp)(char*);
        params[0].tipo_val = PTR_PAR;
        params[0].orig_val = PARAM;

        cria_func(tam_str, params, 1, codigo);
        fp f = (fp) codigo;

        CHECK("tam_str(\"abc\")", f("abc"), 3);
        CHECK("tam_str(\"\")", f(""), 0);
        CHECK("tam_str(\"hello!\")", f("hello!"), 6);
    }


    /* -------------------------------------------------------------- */
    /* 12) memcmp amarrado (exemplo do enunciado) - chama biblioteca! */
    /* -------------------------------------------------------------- */
    printf("\n[12] memcmp(fixa, s, n) - FIX ptr + PARAM ptr + PARAM int:\n");
    {
        typedef int (*fp)(void*, size_t);
        static char fixa[] = "quero saber se a outra string e um prefixo dessa";
        char s[] = "quero saber tudo";

        params[0].tipo_val = PTR_PAR;
        params[0].orig_val = FIX;
        params[0].valor.v_ptr = fixa;
        params[1].tipo_val = PTR_PAR;
        params[1].orig_val = PARAM;
        params[2].tipo_val = INT_PAR;
        params[2].orig_val = PARAM;

        cria_func(memcmp, params, 3, codigo);
        fp mesmo_prefixo = (fp) codigo;

        /* prefixo de 12 chars: "quero saber " - igual nos dois -> 0 */
        CHECK("prefixo-12 igual", mesmo_prefixo(s, 12), 0);
        /* string toda (16 chars) -> diferente -> != 0 */
        CHECK("prefixo-16 diferente?", mesmo_prefixo(s, 16) != 0, 1);
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