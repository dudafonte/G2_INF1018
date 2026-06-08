# G2_INF1018 — Gerador Dinâmico de Funções

**INF1018 - Software Básico (2026.1) — PUC-Rio**

Lívia Ramos Vallim — 2320338 3WA  
Maria Eduarda Fonte de Macedo — 2320450 3WA

---

## Descrição

Implementação da função `cria_func`, que gera dinamicamente código de máquina x86-64 em tempo de execução. Dado o endereço de uma função `f` e a descrição de seus parâmetros, `cria_func` grava no vetor `codigo` o código de uma nova função que chama `f` com os parâmetros configurados — podendo amarrá-los a valores fixos, variáveis ou repassá-los do chamador.

## Estrutura

```
.
├── cria_func.c   # implementação de cria_func
├── cria_func.h   # definições de DescParam, TipoValor, OrigemValor
├── teste.c       # testes
└── relatorio.txt # relatório de testes
```

## Compilação

```bash
gcc -Wall -Wa,--execstack -o teste cria_func.c teste.c
```

> A flag `--execstack` é necessária para permitir a execução de código gerado dinamicamente em área de dados.

## Execução

```bash
./teste
```

## Parâmetros

Cada parâmetro de `f` é descrito por um `DescParam` com três campos:

| Campo | Tipo | Descrição |
|---|---|---|
| `tipo_val` | `INT_PAR` / `PTR_PAR` | Inteiro (32 bits) ou ponteiro (64 bits) |
| `orig_val` | `PARAM` / `FIX` / `IND` | Origem do valor (ver abaixo) |
| `valor` | `v_int` / `v_ptr` | Valor ou endereço (quando amarrado) |

### Origens

- **PARAM**: parâmetro repassado do chamador para `f`
- **FIX**: valor constante embutido no código gerado
- **IND**: valor lido em runtime do endereço em `valor.v_ptr`

## Exemplo

```c
#include "cria_func.h"

int mult(int x, int y) { return x * y; }

// cria uma função que multiplica seu argumento por 10
DescParam params[2];
unsigned char codigo[500];

params[0].tipo_val = INT_PAR;
params[0].orig_val = PARAM;

params[1].tipo_val = INT_PAR;
params[1].orig_val = FIX;
params[1].valor.v_int = 10;

cria_func(mult, params, 2, codigo);

typedef int (*fp)(int);
fp f = (fp) codigo;
f(5);  // retorna 50
```
