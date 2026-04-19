/**
 * @file base.c
 * @author ist1118057 (Bruno Santos)
 * Implementação das funções base e utilitárias do sistema.
 * Contém funções de inicialização, libertação de memória,
 * gestão da tabela de IVA, validação de dados, manipulação
 * de strings e operações auxiliares sobre a tabela hash
 * de produtos. 
*/

#include "sistema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/** INICIALIZAÇÃO */

/**
 * Inicializa a tabela de IVA.
 * Marca todos os códigos de IVA como inexistentes,
 * atribuindo o valor -1 a cada posição da tabela.
 * @param s Ponteiro para o sistema.
*/
void limpar_tabela_iva(Sistema *s) { 
    int i;
    for (i = 0; i < IVA; i++) {
        s->iva[i] = -1;
    }
}

/**
 * Inicializa o sistema.
 * Define o sistema sem produtos nem facturas, inicializa os valores
 * acumulados a zero, prepara o cesto vazio e inicializa a tabela de
 * dispersão de produtos e a tabela de IVA.
 * @param s Ponteiro para o sistema.
*/
void inicializar_sistema(Sistema *s) {
    int i;
    s -> n_produtos = 0;
    s -> n_ativos = 0;
    s -> facturas = NULL; /** vetor dinâmico de facturas não alocado*/
    s -> n_facturas = 0;
    s -> espaco_facturas = 0; /** capacidade inicial do vetor de facturas. */
    s -> proxima_factura = 1;
    s -> total_items = 0; /** ainda não foram vendidos quaisquer produtos */
    s-> n_itens_cesto = 0;
    s -> total_facturado = 0.0;
    limpar_tabela_iva(s);
    for (i = 0; i < HASH_PROD; i++) {
        s->hash_produtos[i] = NULL;
    }
}
 
/**
 * Define os valores de IVA por defeito do sistema.
 * Inicializa a tabela de IVA com os valores definidos no enunciado:
 * A 0, B 6, C 13 e D 23.
 * @param s Ponteiro para o sistema.
*/
void iva_default(Sistema*s) {
    limpar_tabela_iva(s);
    s ->iva['A' - 'A'] = IVA_A;
    s ->iva['B' - 'A'] = IVA_B;
    s ->iva['C' - 'A'] = IVA_C;
    s ->iva['D' - 'A'] = IVA_D;
}

/**
 * Lê os códigos e valores de IVA a partir de um ficheiro.
 * Abre o ficheiro indicado e atualiza a tabela de IVA com os pares
 * (letra, valor) lidos. Apenas códigos entre 'A' e 'Z' são considerados.
 * A tabela é previamente limpa, sendo substituída pelos valores lidos.
 * @param s Ponteiro para o sistema.
 * @param nome_ficheiro Nome do ficheiro de IVA.
*/
void ler_ficheiro_iva(Sistema *s, char *nome_ficheiro) {
    FILE *ficheiro;
    char letra;
    int valor;
    ficheiro = fopen(nome_ficheiro, "r");
    /** verifica se houve erro na abertura do ficheiro */
    if (ficheiro == NULL) { 
        return;  /** se não abriu, sai da função sem fazer nada */
    }
    limpar_tabela_iva(s);
    while (fscanf(ficheiro, " %c %d", &letra, &valor) == 2) {
        if (letra >= 'A' && letra <= 'Z') {
            s->iva[letra - 'A'] = valor;
        }
    }
    fclose(ficheiro);
}


/**
 * Liberta a memória da tabela de dispersão de produtos.
 * Percorre todas as posições da tabela de dispersão e liberta
 * os nós das listas ligadas associadas a cada posição.
 * @param s Ponteiro para o sistema.
*/
void libertar_hash_produtos(Sistema *s) {
    int i;
    NoHashProd *atual, *prox;
    for (i = 0; i < HASH_PROD; i++) {
        atual = s->hash_produtos[i];
        while (atual != NULL) {
            prox = atual->prox;
            free(atual);
            atual = prox;
        }
    }
}


/**
 * Liberta toda a memória dinâmica associada ao sistema.
 * Liberta os nomes armazenados nas faturas, a tabela de dispersão
 * de produtos e o vetor dinâmico de faturas.
 * @param s Ponteiro para o sistema.
*/
void libertar_sistema(Sistema *s) {
    int i;
    for (i = 0; i < s->n_facturas; i++) {
        if (s->facturas[i].nome != NULL) {
            free(s->facturas[i].nome);
        }
    }
    libertar_hash_produtos(s);
    free(s->facturas);
}

/** HASH E FUNÇÕES AUXILIARES */

/**
 * Calcula o valor de hash de uma string.
 * Aplica uma função de dispersão polinomial do tipo
 * h = HASH_BASE * h + carácter para cada carácter da string,
 * mantendo o resultado no intervalo [0, tamanho-1] através
 * do operador módulo.
 * @param s String a processar.
 * @param tamanho Dimensão da tabela de dispersão.
 * @return Valor de hash no intervalo [0, tamanho-1].
*/
int hash_string(char *s, int tamanho) {
	int h = 0;
	while (*s) {
		h = (HASH_BASE * h + *s) % tamanho;
		s++;
	}
	return h;
}

/**
 * Verifica se uma string contém apenas dígitos.
 * Percorre a string e testa cada caracter com isdigit.
 * Se encontrar algum caracter que não seja dígito, retorna 0.
 * @param s String a verificar.
 * @return 1 se todos os caracteres forem dígitos, 0 caso contrário.
*/
int so_digitos(char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (!isdigit(s[i])) { 
            return 0;
        }
    }
    return 1;
}

/**
 * Verifica se a descrição de um produto é válida.
 * A descrição não pode ser vazia, deve respeitar o comprimento máximo
 * e começar por uma letra maiúscula ou por um carácter acentuado.
 * @param s String da descrição.
 * @return 1 se a descrição for válida, 0 caso contrário.
*/
int descricao_valida(char *s) {
    if (s[0] == '\0' || strlen(s) > MAXDESCRICAO)
        return 0;
    /** verifica se começa por maiúscula ou por um carácter acentuado (UTF-8) */
    if (!((s[0] >= 'A' && s[0] <= 'Z') || s[0] < 0))
        return 0;
    return 1;
}

/**
 * Avança numa string ignorando espaços e tabs.
 * Devolve um ponteiro para o primeiro carácter que não seja espaço
 * (' ') nem tab ('\t').
 * @param p Ponteiro para a string.
 * @return Ponteiro para o primeiro carácter não branco.
*/
char *saltar_espacos(char *p) {
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    return p;
}

/**
 * Copia um intervalo de caracteres de uma string.
 * Copia os primeiros n caracteres de origem para destino e
 * adiciona '\0' no final.
 * @param dest String de destino.
 * @param origem String de origem.
 * @param n Número de caracteres a copiar.
*/
void copiar_intervalo(char *dest, char *origem, int n) {
    int i;
    for (i = 0; i < n; i++) {
        dest[i] = origem[i];
    }
    dest[n] = '\0';
}

/**
 * Remove o carácter de nova linha ('\n') no final de uma string.
 * Se a string terminar com '\n', substitui-o pelo terminador '\0'.
 * @param s String a modificar.
*/
void tirar_newline(char *s) {
    int len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

/**
 * Verifica se um código de IVA existe no sistema.
 * Um código é válido se for uma letra maiúscula entre 'A' e 'Z'
 * e se tiver um valor associado na tabela de IVA.
 * @param s Ponteiro para o sistema.
 * @param iva Código de IVA a verificar.
 * @return 1 se o código existir, 0 caso contrário.
*/
int iva_existe(Sistema *s, char iva) {
    /** verifica se o carácter não é uma letra maiúscula */
    if (iva < 'A' || iva > 'Z') 
        return 0;
    if (s->iva[iva - 'A'] == -1) 
        return 0;
    return 1;
}

/** PRODUTOS */

/**
 * Insere um produto na tabela de dispersão.
 * Calcula a posição correspondente ao código EAN e cria um novo nó
 * com esse código e com o índice do produto no vetor de produtos.
 * O nó é inserido no início da lista ligada dessa posição, permitindo
 * resolver colisões.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto.
 * @param indice_produto Índice do produto no vetor de produtos.
 * @return 1 em caso de sucesso.
*/
int inserir_hash_produto(Sistema *s, char *ean, int indice_produto) {
    int h = hash_string(ean, HASH_PROD);
    NoHashProd *novo = malloc(sizeof(NoHashProd));
    strcpy(novo->ean, ean);
    novo->indice_produto = indice_produto;
    novo->prox = s->hash_produtos[h]; /** liga o novo nó à lista existente */
    /** atualiza a tabela para apontar para o novo nó */
    s->hash_produtos[h] = novo;
    return 1;
}

/**
 * Procura um produto pelo código EAN.
 * Calcula a posição na tabela de dispersão e percorre a lista ligada
 * associada, procurando um nó com o EAN indicado. Se encontrar e o
 * produto estiver ativo, devolve o índice no vetor de produtos.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN a procurar.
 * @return Índice do produto se encontrado e ativo, -1 caso contrário.
 */
int procurar_produto(Sistema *s, char *ean) {
    int h = hash_string(ean, HASH_PROD);
    /** aponta para o início da lista ligada na posição h da tabela hash */
    NoHashProd *atual = s->hash_produtos[h];
    while (atual != NULL) {
        if (strcmp(atual->ean, ean) == 0) {
            if (s->produtos[atual->indice_produto].ativo) {
                return atual->indice_produto;
            }
            return -1;
        }
        atual = atual->prox;
    }
    return -1;
}