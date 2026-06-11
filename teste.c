/*
 * testes_extras.c — Suite de testes complementares para cria_func
 *
 * Compilar:
 *   gcc -Wall -Wa,--execstack -o testes_extras cria_func.c testes_extras.c
 *
 * Organização:
 *   BLOCO A  (A1–A12) — devem PASSAR
 *   BLOCO B  (B1–B4)  — devem FALHAR
 *       B2: descomentado — usa checar_deve_falhar (não crashar)
 *       B1, B3, B4: comentados — crasham antes de qualquer checagem
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "cria_func.h"

/* usada em B4 */
static int quatro(int a, int b, int c, int d) { return a + b + c + d; }

/* ── helpers ──────────────────────────────────────────────────────────── */
static int passou = 0, falhou = 0;

/* Verifica que o resultado está CORRETO */
static void checar(const char *nome, int obtido, int esperado)
{
    if (obtido == esperado) {
        printf("  [OK]     %s: %d\n", nome, obtido);
        passou++;
    } else {
        printf("\n");
        printf("  ##################################\n");
        printf("  ## FALHOU: %-22s##\n", nome);
        printf("  ##   esperado = %-18d##\n", esperado);
        printf("  ##   obtido   = %-18d##\n", obtido);
        printf("  ##################################\n\n");
        falhou++;
    }
}

/*
 * Verifica que o resultado é ERRADO.
 * Usado nos testes do Bloco B que não crasham:
 *   - "passou" significa: comportou-se errado (como esperávamos)
 *   - "falhou" significa: retornou o valor correto quando não deveria
 */
static void checar_deve_falhar(const char *nome, int obtido, int correto)
{
    if (obtido != correto) {
        printf("  [OK — falhou como esperado] %s\n", nome);
        printf("            obtido=%d  correto seria=%d\n", obtido, correto);
        passou++;
    } else {
        printf("\n");
        printf("  ##################################\n");
        printf("  ## PASSOU QUANDO NÃO DEVIA      ##\n");
        printf("  ## %-30s##\n", nome);
        printf("  ##   obtido = %d (valor correto!) ##\n", obtido);
        printf("  ##################################\n\n");
        falhou++;
    }
}

/* ── funções-alvo ─────────────────────────────────────────────────────── */
int mult(int x, int y)        { return x * y; }
int soma(int x, int y)        { return x + y; }
int tres(int x, int y, int z) { return x + y + z; }


/* ═══════════════════════════════════════════════════════════════════════
 * BLOCO A — devem PASSAR
 * ═══════════════════════════════════════════════════════════════════════ */

/* A1 — [PARAM, FIX(-5)]: FIX com valor negativo
 * "mov $imm32, %esi" usa zero-extend; a função f lê esi como `int`
 * → -5 em dois-complemento é preservado corretamente.
 */
void teste_a1_neg_fix(void)
{
    printf("\n[A1] PARAM, FIX(-5) — multiplicador negativo\n");
    DescParam p[2]; unsigned char c[500]; typedef int (*fp)(int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX; p[1].valor.v_int=-5;
    cria_func(mult, p, 2, c); fp f=(fp)c;
    checar("f(3)  == -15", f(3),  -15);
    checar("f(-2) ==  10", f(-2),  10);
    checar("f(0)  ==   0", f(0),    0);
}

/* A2 — [PARAM, FIX(0)]: FIX zero
 * Zero pode esconder bug se houver um "if(v_int)" antes de emitir o mov.
 */
void teste_a2_zero_fix(void)
{
    printf("\n[A2] PARAM, FIX(0) — zero como constante\n");
    DescParam p[2]; unsigned char c[500]; typedef int (*fp)(int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX; p[1].valor.v_int=0;
    cria_func(mult, p, 2, c); fp f=(fp)c;
    checar("f(999) == 0", f(999), 0);
    checar("f(-1)  == 0", f(-1),  0);
    checar("f(0)   == 0", f(0),   0);
}

/* A3 — [PARAM, PARAM] com negativos
 * Save/restore usa REX.W (64 bits); os 32 bits baixos preservam o negativo.
 */
void teste_a3_neg_param(void)
{
    printf("\n[A3] PARAM, PARAM — argumentos negativos\n");
    DescParam p[2]; unsigned char c[500]; typedef int (*fp)(int,int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=INT_PAR; p[1].orig_val=PARAM;
    cria_func(soma, p, 2, c); fp f=(fp)c;
    checar("f(-5, -3) == -8", f(-5,-3), -8);
    checar("f(-10, 5) == -5", f(-10,5), -5);
    checar("f(-1,  1) ==  0", f(-1, 1),  0);
}

/* A4 — [PARAM, PARAM, FIX(50)]: FIX no último slot (rdx, opcode 0xBA)
 * Combinação não testada; verifica save/restore de dois PARAMs com FIX em j=2.
 */
void teste_a4_param_param_fix(void)
{
    printf("\n[A4] PARAM, PARAM, FIX(50) — FIX no slot 2 (rdx)\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)(int,int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=INT_PAR; p[1].orig_val=PARAM;
    p[2].tipo_val=INT_PAR; p[2].orig_val=FIX; p[2].valor.v_int=50;
    cria_func(tres, p, 3, c); fp f=(fp)c;
    checar("f(1,  2) ==  53", f(1,  2),  53);
    checar("f(-3,-7) ==  40", f(-3,-7),  40);
    checar("f(0,  0) ==  50", f(0,  0),  50);
}

/* A5 — [FIX(1), FIX(2), PARAM]: PARAM no último slot
 * O caso mais crítico: único arg chega em rdi mas precisa ir para rdx.
 * Se Step-1 e Step-2 estivessem invertidos, este teste quebraria.
 */
void teste_a5_fix_fix_param(void)
{
    printf("\n[A5] FIX(1), FIX(2), PARAM — reencaminhamento rdi -> rdx\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)(int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=FIX;   p[0].valor.v_int=1;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX;   p[1].valor.v_int=2;
    p[2].tipo_val=INT_PAR; p[2].orig_val=PARAM;
    cria_func(tres, p, 3, c); fp f=(fp)c;
    checar("f(3)   ==   6", f(3),    6);
    checar("f(100) == 103", f(100), 103);
    checar("f(-3)  ==   0", f(-3),   0);
}

/* A6 — [IND, IND, FIX(10)]: dois IND consecutivos
 * Dois IND reusam %rax como scratch; verifica que o segundo não clobber o primeiro.
 */
void teste_a6_ind_ind_fix(void)
{
    printf("\n[A6] IND, IND, FIX(10) — dois IND em sequencia (rax clobber?)\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)();
    int a, b;
    p[0].tipo_val=INT_PAR; p[0].orig_val=IND; p[0].valor.v_ptr=&a;
    p[1].tipo_val=INT_PAR; p[1].orig_val=IND; p[1].valor.v_ptr=&b;
    p[2].tipo_val=INT_PAR; p[2].orig_val=FIX; p[2].valor.v_int=10;
    cria_func(tres, p, 3, c); fp f=(fp)c;
    a= 1; b= 2; checar("a=1, b=2   -> 13", f(), 13);
    a= 5; b= 5; checar("a=5, b=5   -> 20", f(), 20);
    a=-10;b= 5; checar("a=-10,b=5  ->  5", f(),  5);
    a= 0; b= 0; checar("a=0, b=0   -> 10", f(), 10);
}

/* A7 — [IND, FIX(5), IND]: IND nos extremos, FIX no meio
 * Testa opcodes ind_op_int[0]=0x38 (→edi) e ind_op_int[2]=0x10 (→edx)
 * em posições não contíguas.
 */
void teste_a7_ind_fix_ind(void)
{
    printf("\n[A7] IND, FIX(5), IND — IND nos extremos\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)();
    int a, b;
    p[0].tipo_val=INT_PAR; p[0].orig_val=IND; p[0].valor.v_ptr=&a;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX; p[1].valor.v_int=5;
    p[2].tipo_val=INT_PAR; p[2].orig_val=IND; p[2].valor.v_ptr=&b;
    cria_func(tres, p, 3, c); fp f=(fp)c;
    a= 1; b= 2; checar("a=1,  b=2  ->  8", f(),  8);
    a= 0; b= 0; checar("a=0,  b=0  ->  5", f(),  5);
    a=-5; b=-5; checar("a=-5, b=-5 -> -5", f(), -5);
}

/* A8 — IND com variável negativa (INT_PAR)
 * "mov (%rax), %edi" lê 32 bits com zero-extend; -5 (0xFFFFFFFB) em edi
 * é lido pela função como `int` = -5. Correto apesar do zero-extend.
 */
void teste_a8_ind_negativo(void)
{
    printf("\n[A8] IND com int negativo — zero-extend de 32 bits\n");
    DescParam p[2]; unsigned char c[500]; typedef int (*fp)();
    int neg;
    p[0].tipo_val=INT_PAR; p[0].orig_val=IND; p[0].valor.v_ptr=&neg;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX; p[1].valor.v_int=10;
    cria_func(soma, p, 2, c); fp f=(fp)c;
    neg=  -5; checar("neg=-5   ->   5", f(),   5);
    neg=-100; checar("neg=-100 -> -90", f(), -90);
    neg= -10; checar("neg=-10  ->   0", f(),   0);
}

/* A9 — [IND, IND, IND]: todos indiretos
 * Três pares movabs+mov reusam %rax. Também prova leitura DINÂMICA:
 * mudar a variável após cria_func reflete na próxima chamada.
 */
void teste_a9_ind_ind_ind(void)
{
    printf("\n[A9] IND, IND, IND — todos indiretos; leitura dinamica\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)();
    int a, b, c3;
    p[0].tipo_val=INT_PAR; p[0].orig_val=IND; p[0].valor.v_ptr=&a;
    p[1].tipo_val=INT_PAR; p[1].orig_val=IND; p[1].valor.v_ptr=&b;
    p[2].tipo_val=INT_PAR; p[2].orig_val=IND; p[2].valor.v_ptr=&c3;
    cria_func(tres, p, 3, c); fp f=(fp)c;
    a= 1; b= 2; c3= 3; checar(" 1+ 2+ 3 =  6", f(),  6);
    a=-1; b=-2; c3=-3; checar("-1-2-3   = -6", f(), -6);
    a= 0; b= 0; c3= 0; checar(" 0+ 0+ 0 =  0", f(),  0);
    a=42;              checar("42+ 0+ 0 = 42", f(), 42); /* b=0, c3=0 ainda */
}

/* A10 — FIX com INT_MAX e INT_MIN
 * 0x7FFFFFFF e 0x80000000 revelam bugs de sign-extension ou truncamento.
 */
void teste_a10_boundary_values(void)
{
    printf("\n[A10] FIX(INT_MIN) e FIX(INT_MAX) — valores limite\n");
    DescParam p[2]; unsigned char c1[500], c2[500]; typedef int (*fp)(int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX;

    p[1].valor.v_int=INT_MIN;
    cria_func(soma, p, 2, c1); fp fmin=(fp)c1;
    checar("soma(0, INT_MIN) == INT_MIN",   fmin(0),    INT_MIN);
    checar("soma(1, INT_MIN) == INT_MIN+1", fmin(1), INT_MIN+1);

    p[1].valor.v_int=INT_MAX;
    cria_func(soma, p, 2, c2); fp fmax=(fp)c2;
    checar("soma(0,  INT_MAX) == INT_MAX",   fmax(0),     INT_MAX);
    checar("soma(-1, INT_MAX) == INT_MAX-1", fmax(-1), INT_MAX-1);
}

/* A11 — [PARAM, FIX(10), FIX(20)]: só o primeiro é PARAM
 * Dois FIX após um PARAM verificam que os opcodes 0xBE e 0xBA são emitidos
 * nos slots certos sem perder o valor salvo em slot-0.
 */
void teste_a11_param_fix_fix(void)
{
    printf("\n[A11] PARAM, FIX(10), FIX(20) — dois FIX apos PARAM\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)(int);
    p[0].tipo_val=INT_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX; p[1].valor.v_int=10;
    p[2].tipo_val=INT_PAR; p[2].orig_val=FIX; p[2].valor.v_int=20;
    cria_func(tres, p, 3, c); fp f=(fp)c;
    checar("f(0)   ==  30", f(0),    30);
    checar("f(5)   ==  35", f(5),    35);
    checar("f(-30) ==   0", f(-30),   0);
}

/* A12 — PTR PARAM + PTR FIX(j=1) + INT PARAM com memcmp
 * Testes originais têm PTR FIX em j=0. Aqui PTR FIX em j=1 usa
 * "movabs $ptr64, %rsi" (opcode 0x48 0xBE).
 */
void teste_a12_ptr_fix_pos1(void)
{
    printf("\n[A12] PTR PARAM + PTR FIX(j=1) + INT PARAM — FIX ptr no slot 1\n");
    DescParam p[3]; unsigned char c[500]; typedef int (*fp)(void *, size_t);
    char base[] = "hello world";
    p[0].tipo_val=PTR_PAR; p[0].orig_val=PARAM;
    p[1].tipo_val=PTR_PAR; p[1].orig_val=FIX;   p[1].valor.v_ptr=base;
    p[2].tipo_val=INT_PAR; p[2].orig_val=PARAM;
    cria_func(memcmp, p, 3, c); fp f=(fp)c;
    checar("prefixo-5  'hello abc'   == 0",  f("hello abc",  5), 0);
    checar("prefixo-5  'HELLO abc'   != 0", (f("HELLO abc",  5) != 0), 1);
    checar("prefixo-11 'hello world' == 0",  f("hello world",11), 0);
}


/* ═══════════════════════════════════════════════════════════════════════
 * BLOCO B — devem FALHAR
 * ═══════════════════════════════════════════════════════════════════════ */

/*
 * B1 — IND com v_ptr = NULL  →  SIGSEGV  (COMENTADO — crasha)
 *
 * Gera: movabs $0,%rax; mov (%rax),%edi — leitura do endereço 0.
 * cria_func não valida ponteiros; responsabilidade é do chamador.
 *
void teste_b1_ind_null(void)
{
    printf("\n[B1] IND com NULL — SIGSEGV esperado\n");
    DescParam p[2]; unsigned char c[500]; typedef int (*fp)();
    p[0].tipo_val=INT_PAR; p[0].orig_val=IND; p[0].valor.v_ptr=NULL;
    p[1].tipo_val=INT_PAR; p[1].orig_val=FIX; p[1].valor.v_int=1;
    cria_func(soma, p, 2, c);
    fp f=(fp)c;
    printf("resultado: %d\n", f()); // SIGSEGV aqui
}
*/

/*
 * B2 — n=0 (abaixo do mínimo)  →  "passa por acidente"
 *
 * Com n=0 os loops não iteram: nenhum PARAM é salvo ou carregado.
 * A função gerada chama mult com o que estiver em rdi/rsi no momento.
 * Se chamada como f(5,3), rdi=5 e rsi=3 chegam intactos até mult
 * → mult(5,3)=15, resultado CORRETO por acidente de convenção de chamada!
 *
 * checar_deve_falhar detecta exatamente isso: retornou 15 quando não deveria.
 */
void teste_b2_n_zero(void)
{
    printf("\n[B2] n=0 — nenhum param gerenciado; passa por acidente?\n");
    DescParam p[1]; /* alocado mas ignorado (n=0) */
    unsigned char c[500];
    typedef int (*fp)(int, int);

    cria_func(mult, p, 0, c);
    fp f = (fp)c;

    /* Com n=2 PARAM o correto seria mult(5,3)=15.
       Com n=0, rdi/rsi não são tocados → mult recebe 5 e 3 assim mesmo. */
    checar_deve_falhar("n=0: mult(5,3) deveria ser errado, correto=15",
                       f(5, 3), 15);

    /* Muda os argumentos: se houver resultado diferente de 6, confirma lixo */
    checar_deve_falhar("n=0: mult(2,3) deveria ser errado, correto=6",
                       f(2, 3), 6);
}

/*
 * B3 — buffer de 15 bytes para 3×IND  →  stack smash  (COMENTADO — crasha)
 *
 * Código gerado ≈ 58 bytes; buffer[15] causa overflow de pilha.
 * Com -fstack-protector (padrão no GCC moderno) → "stack smashing detected"
 * na saída de cria_func. Sem protetor → UB silencioso / crash tardio.
 *
void teste_b3_buffer_overflow(void)
{
    printf("\n[B3] buffer de 15 bytes — stack smash esperado\n");
    DescParam p[3]; unsigned char c[15]; typedef int (*fp)();
    int a=1, b=2, c3=3;
    p[0].tipo_val=INT_PAR; p[0].orig_val=IND; p[0].valor.v_ptr=&a;
    p[1].tipo_val=INT_PAR; p[1].orig_val=IND; p[1].valor.v_ptr=&b;
    p[2].tipo_val=INT_PAR; p[2].orig_val=IND; p[2].valor.v_ptr=&c3;
    cria_func(tres, p, 3, c); // overflow aqui
    fp f=(fp)c;
    printf("resultado: %d\n", f());
}
*/

/*
 * B4 — n=4 (acima do máximo=3)  →  OOB nas arrays internas  (COMENTADO — pode crashar)
 *
 * salva_hi[], offsets[], fix_op[] têm tamanho 3.
 * Para j=3, acessam índice 3 → UB. O byte adjacente (padding ou outra var)
 * vira ModRM de uma instrução malformada; pode gerar escrita em memória
 * arbitrária → crash imprevisível. Por isso permanece comentado.
 *
 * Se descomentar, o checar_deve_falhar abaixo capturaria se por acaso
 * o resultado vier correto (1+2+3+4=10):
 *
void teste_b4_n_maior_3(void)
{
    printf("\n[B4] n=4 — OOB nas arrays; resultado errado ou crash\n");
    DescParam p[4]; unsigned char c[500]; typedef int (*fp)(int,int,int,int);
    for (int k=0; k<4; k++) { p[k].tipo_val=INT_PAR; p[k].orig_val=PARAM; }
    cria_func(quatro, p, 4, c);
    fp f=(fp)c;
    checar_deve_falhar("quatro(1,2,3,4) correto=10, n=4 deve diferir",
                       f(1,2,3,4), 10);
}
*/


/* ═══════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════ */
int main(void)
{
    printf("=== BLOCO A — devem PASSAR ===\n");
    teste_a1_neg_fix();
    teste_a2_zero_fix();
    teste_a3_neg_param();
    teste_a4_param_param_fix();
    teste_a5_fix_fix_param();
    teste_a6_ind_ind_fix();
    teste_a7_ind_fix_ind();
    teste_a8_ind_negativo();
    teste_a9_ind_ind_ind();
    teste_a10_boundary_values();
    teste_a11_param_fix_fix();
    teste_a12_ptr_fix_pos1();

    printf("\n\n=== BLOCO B — devem FALHAR ===\n");
    teste_b2_n_zero();   /* unico B seguro para rodar sem crashar */

    printf("\n====================================\n");
    if (falhou == 0) {
        printf("  TODOS OS %d TESTES PASSARAM\n", passou);
    } else {
        printf("  %d passou(aram)\n", passou);
        printf("\n");
        printf("  ##################################\n");
        printf("  ##  %2d TESTE(S) REPROVADO(S)   ##\n", falhou);
        printf("  ##################################\n");
    }
    printf("====================================\n");

    printf("\n  B1, B3, B4 permanecem comentados (crasham antes de checar).\n");
    printf("  Descomente um por vez para observar o comportamento.\n");

    return falhou > 0 ? 1 : 0;
}