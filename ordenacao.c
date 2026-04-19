/**
 * @file ordenacao.c
 * @author ist1118057 (Bruno Santos)
 * Funções auxiliares de ordenação do sistema.
 * Este ficheiro implementa a comparação e ordenação de índices de
 * produtos e facturas, usando o algoritmo merge sort e critérios
 * adequados a cada tipo de elemento.
*/

#include "sistema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/** ORDENAÇÃO */
/**
 * Indica se um elemento deve aparecer antes de outro na ordenação.
 * Se o tipo for ORD_PRODUTOS, compara produtos pelo código EAN.
 * Caso contrário, compara faturas pelo nome do cliente e, em caso
 * de empate, pelo número da fatura.
 * @param s Ponteiro para o sistema.
 * @param i Índice do primeiro elemento.
 * @param j Índice do segundo elemento.
 * @param tipo Tipo de ordenação a aplicar.
 * @return 1 se o elemento i vier antes de j, 0 caso contrário.
*/
int antes(Sistema *s, int i, int j, int tipo) {
    int cmp; /** variável auxiliar para guardar o resultado de strcmp */
    if (tipo == ORD_PRODUTOS) { /** se estou a ordenar produtos... */
        return strcmp(s->produtos[i].ean, s->produtos[j].ean) < 0;
    }
    cmp = strcmp(s->facturas[i].nome, s->facturas[j].nome);
    /** Se o nome da factura i vier antes alfabeticamente, devolve 1 */
    if (cmp < 0) {
        return 1;
    }
    /** Se o nome da factura i vier depois, devolve 0 */
    if (cmp > 0) { 
        return 0;
    }
    return s->facturas[i].numero < s->facturas[j].numero;
}

/**
 * Junta duas metades já ordenadas de um vetor de índices.
 * Intercala os elementos dos intervalos [esquerda, meio] e
 * [meio + 1, direita] para o vetor auxiliar, copiando depois
 * o resultado ordenado para o vetor original.
 * @param s Ponteiro para o sistema.
 * @param v Vetor de índices a ordenar.
 * @param aux Vetor auxiliar usado na intercalação.
 * @param esquerda Limite esquerdo do intervalo.
 * @param meio Índice final da metade esquerda.
 * @param direita Limite direito do intervalo.
 * @param tipo Tipo de ordenação a aplicar.
*/
void merge(Sistema *s, int v[], int aux[], 
           int esquerda, int meio, int direita, int tipo) {
    int i, j, k;
    i = esquerda; /** i percorre a metade esquerda */
    j = meio + 1; /** j percorre a metade direita */
    k = esquerda; /** k percorre o vetor auxiliar */
    /** enquanto houver elementos nas duas metades */
    while (i <= meio && j <= direita) {
        /** escolhe o menor entre v[i] e v[j] */
        if (antes(s, v[i], v[j], tipo)) { 
            aux[k++] = v[i++]; /** copia da metade esquerda */
        } else {
            aux[k++] = v[j++]; /** copia da metade direita */
        }
    }
    while (i <= meio) { /** copia o resto da metade esquerda (se existir) */
        aux[k++] = v[i++];
    }
    while (j <= direita) { /** copia o resto da metade direita (se existir) */
        aux[k++] = v[j++];
    }
    /** copia os elementos ordenados de volta para v */
    for (i = esquerda; i <= direita; i++) { 
        v[i] = aux[i];
    }
}

/**
 * Ordena um vetor de índices usando merge sort.
 * Divide recursivamente o intervalo em duas metades, ordena cada uma
 * delas e junta o resultado com a função merge.
 * @param s Ponteiro para o sistema.
 * @param v Vetor de índices a ordenar.
 * @param aux Vetor auxiliar.
 * @param esquerda Limite esquerdo do intervalo.
 * @param direita Limite direito do intervalo.
 * @param tipo Tipo de ordenação a aplicar.
*/
void ordenar(Sistema *s, int v[], int aux[], 
            int esquerda, int direita, int tipo) {
    int meio;
    /** Caso base: se o intervalo tiver 0 ou 1 elemento, já está ordenado */
    if (esquerda >= direita) { 
        return;
    }
    meio = (esquerda + direita) / 2; /** Calcula o índice do meio */
    /** Ordena recursivamente a metade esquerda */
    ordenar(s, v, aux, esquerda, meio, tipo);  
    /** Ordena recursivamente a metade direita */
    ordenar(s, v, aux, meio + 1, direita, tipo); 
    /** Junta as duas metades já ordenadas */
    merge(s, v, aux, esquerda, meio, direita, tipo);
}