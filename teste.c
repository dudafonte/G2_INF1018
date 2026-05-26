#include <stdio.h>
#include <string.h>
#include "cria_func.h"


/* funções auxiliares de teste (essa e para checar se as outras estao passando)*/
int passou = 0;
int falhou = 0;

void checar(const char *nome, int obtido, int esperado)
{
    if (obtido == esperado)
    {
        printf("    [OK] %s: %d\n", nome, obtido);
        passou++;
    }
    else
    {
        printf("    [FALHOU] %s: esperado: %d, obtido: %d\n", nome, esperado, obtido);
        falhou++;
    }
}

/* funções para testar */
int mult(int x, int y)        { return x*y; }
int soma(int x, int y)        { return x+y; }
int tres(int x, int y, int z) { return x+y+z; }

/* =========== TESTES =========== */

/* teste 1: [PARAM, FIX] */
void teste_param_fix()
{
    printf("\n[Teste 1] PARAM, FIX (mult x * 10)\n");
    DescParam params[2];
    unsigned char codigo[500];
    typedef int (*fp)(int);

    params[0].tipo_val = INT_PAR;
    params[0].orig_val = PARAM;
    params[1].tipo_val = INT_PAR;
    params[1].orig_val = FIX;
    params[1].valor.v_int = 10;

    cria_func(mult, params, 2, codigo);
    fp f = (fp) codigo;
    checar("f(5) == 50", f(5), 50);
    checar("f(3) == 30", f(3), 30);
    checar("f(1) == 10", f(1), 10);
}

/* teste 2: [FIX, PARAM] - ordem invertida */
void teste_fix_param()
{
    printf("\n[Teste 2] FIX, PARAm (mult 10 * x)\n");
    DescParam params[2];
    unsigned char codigo[500];
    typedef int (*fp)(int);

    params[0].tipo_val = INT_PAR;
    params[0].orig_val = FIX;
    params[0].valor.v_int = 10;
    params[1].tipo_val = INT_PAR;
    params[1].orig_val = PARAM;
    cria_func(mult, params, 2, codigo);
    fp f = (fp) codigo;
    checar("f(5) == 50", f(5), 50);
    checar("f(7) == 70", f(7), 70);
}

/* teste 3: [IND, FIX] - exemplo 2 do enunciado, sem argumentos */
void teste_ind_fix() 
{
    printf("\n[Teste 3] IND, FIX (mult i * 10, sem args)\n");
    DescParam params[2];
    unsigned char codigo[500];
    typedef int (*fp)();
    int i;

    params[0].tipo_val = INT_PAR; 
    params[0].orig_val = IND;
    params[0].valor.v_ptr = &i;
    params[1].tipo_val = INT_PAR; 
    params[1].orig_val = FIX;
    params[1].valor.v_int = 10;

    cria_func(mult, params, 2, codigo);
    fp f = (fp) codigo;

    i = 3;  checar("i=3  -> 30",  f(), 30);
    i = 7;  checar("i=7  -> 70",  f(), 70);
    i = 10; checar("i=10 -> 100", f(), 100);
}

/* teste 4: [FIX, FIX] - os dois amarrados, sem argumentos */
void teste_fix_fix() {
    printf("\n[Teste 4] FIX, FIX (mult 6 * 7, sem args)\n");
    DescParam params[2];
    unsigned char codigo[500];
    typedef int (*fp)();

    params[0].tipo_val = INT_PAR; params[0].orig_val = FIX;
    params[0].valor.v_int = 6;
    params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
    params[1].valor.v_int = 7;

    cria_func(mult, params, 2, codigo);
    fp f = (fp) codigo;

    checar("f() == 42", f(), 42);
}

/* teste 5: [PARAM, PARAM, PARAM] - 3 params, nenhum amarrado */
void teste_tres_params() {
    printf("\n[Teste 5] PARAM, PARAM, PARAM (tres args)\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(int, int, int);

    params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
    params[1].tipo_val = INT_PAR; params[1].orig_val = PARAM;
    params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

    cria_func(tres, params, 3, codigo);
    fp f = (fp) codigo;

    checar("f(1,2,3) == 6",  f(1,2,3), 6);
    checar("f(10,20,30) == 60", f(10,20,30), 60);
}

/* teste 6: [FIX, PARAM, PARAM] - slots misturados */
void teste_fix_param_param() {
    printf("\n[Teste 6] FIX, PARAM, PARAM (tres: 100 + y + z)\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(int, int);

    params[0].tipo_val = INT_PAR; params[0].orig_val = FIX;
    params[0].valor.v_int = 100;
    params[1].tipo_val = INT_PAR; params[1].orig_val = PARAM;
    params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

    cria_func(tres, params, 3, codigo);
    fp f = (fp) codigo;

    checar("f(1,2) == 103", f(1,2), 103);
    checar("f(5,5) == 110", f(5,5), 110);
}

/* teste 7: memcmp com PTR_PAR - exemplo do enunciado */
void teste_memcmp() {
    printf("\n[Teste 7] PTR_PAR com memcmp\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(void*, size_t);

    char fixa[] = "hello world";

    params[0].tipo_val = PTR_PAR; params[0].orig_val = FIX;
    params[0].valor.v_ptr = fixa;
    params[1].tipo_val = PTR_PAR; params[1].orig_val = PARAM;
    params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

    cria_func(memcmp, params, 3, codigo);
    fp mesmo_prefixo = (fp) codigo;

    checar("prefixo 5 igual",  mesmo_prefixo("hello abc", 5), 0);
    checar("prefixo 11 igual", mesmo_prefixo("hello world", 11), 0);
    checar("prefixo 6 diff", mesmo_prefixo("hello!", 6), memcmp(fixa, "hello!", 6));
}

/* teste 8: IND + PTR_PAR - ponteiro amarrado indiretamente */
void teste_ind_ptr() {
    printf("\n[Teste 8] IND, PTR_PAR com memcmp\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(size_t);

    char fixa[] = "hello world";
    char* ptr_fixa = fixa; 

    params[0].tipo_val = PTR_PAR; params[0].orig_val = IND;
    params[0].valor.v_ptr = &ptr_fixa;  

    params[1].tipo_val = PTR_PAR; params[1].orig_val = FIX;
    params[1].valor.v_ptr = "hello world";

    params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

    cria_func(memcmp, params, 3, codigo);
    fp f = (fp) codigo;

    checar("prefixo 5 igual",  f(5), 0);
    checar("prefixo 11 igual", f(11), 0);

    ptr_fixa = "outro texto";
    checar("depois de mudar ptr, prefixo 5 diff", f(5) != 0, 1);
}

/* teste 9: 1 parâmetro apenas */
void teste_um_param() {
    printf("\n[Teste 9] 1 parametro (PARAM)\n");
    DescParam params[1];
    unsigned char codigo[500];
    typedef int (*fp)(int);

    int identidade(int x) { return x; }  

    params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;

    cria_func(identidade, params, 1, codigo);
    fp f = (fp) codigo;

    checar("f(7) == 7",   f(7),  7);
    checar("f(42) == 42", f(42), 42);
}

/* teste 10: PARAM, IND, PARAM - mistura de origens */
void teste_param_ind_param() {
    printf("\n[Teste 10] PARAM, IND, PARAM (tres: x + i + z)\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(int, int);
    int i;

    params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
    params[1].tipo_val = INT_PAR; params[1].orig_val = IND;
    params[1].valor.v_ptr = &i;
    params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

    cria_func(tres, params, 3, codigo);
    fp f = (fp) codigo;

    i = 100;
    checar("i=100, f(1,2) == 103", f(1,2), 103);
    i = 50;
    checar("i=50,  f(1,2) == 53",  f(1,2), 53);
}

/* teste 11: FIX, FIX, FIX - todos amarrados */
void teste_fix_fix_fix() {
    printf("\n[Teste 11] FIX, FIX, FIX (tres: 1 + 2 + 3)\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)();

    params[0].tipo_val = INT_PAR; params[0].orig_val = FIX;
    params[0].valor.v_int = 1;
    params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
    params[1].valor.v_int = 2;
    params[2].tipo_val = INT_PAR; params[2].orig_val = FIX;
    params[2].valor.v_int = 3;

    cria_func(tres, params, 3, codigo);
    fp f = (fp) codigo;

    checar("f() == 6", f(), 6);
}

/* teste 12: PARAM, FIX, PARAM - slot 0 e slot 1 não contíguos com j */
void teste_param_fix_param() {
    printf("\n[Teste 12] PARAM, FIX, PARAM (tres: x + 100 + z)\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(int, int);

    params[0].tipo_val = INT_PAR; params[0].orig_val = PARAM;
    params[1].tipo_val = INT_PAR; params[1].orig_val = FIX;
    params[1].valor.v_int = 100;
    params[2].tipo_val = INT_PAR; params[2].orig_val = PARAM;

    cria_func(tres, params, 3, codigo);
    fp f = (fp) codigo;

    checar("f(1,2)   == 103", f(1,2),   103);
    checar("f(10,20) == 130", f(10,20), 130);
}

/* teste 13: FIX, PARAM, FIX */
void teste_fix_param_fix() {
    printf("\n[Teste 13] FIX, PARAM, FIX (tres: 10 + x + 5)\n");
    DescParam params[3];
    unsigned char codigo[500];
    typedef int (*fp)(int);

    params[0].tipo_val = INT_PAR; params[0].orig_val = FIX;
    params[0].valor.v_int = 10;
    params[1].tipo_val = INT_PAR; params[1].orig_val = PARAM;
    params[2].tipo_val = INT_PAR; params[2].orig_val = FIX;
    params[2].valor.v_int = 5;

    cria_func(tres, params, 3, codigo);
    fp f = (fp) codigo;

    checar("f(1)  == 16", f(1),  16);
    checar("f(20) == 35", f(20), 35);
}

/* =========== MAIN =========== */
int main(void) {
    teste_param_fix();
    teste_fix_param();
    teste_ind_fix();
    teste_fix_fix();
    teste_tres_params();
    teste_fix_param_param();
    teste_memcmp();
    teste_ind_ptr();
    teste_um_param();
    teste_param_ind_param();
    teste_fix_fix_fix();
    teste_param_fix_param();
    teste_fix_param_fix();

    printf("\n============================\n");
    printf("  %d passou(aram), %d falhou(aram)\n", passou, falhou);
    printf("============================\n");

    return falhou > 0 ? 1 : 0;
}