/**
 * @file cesto.c
 * @author ist1118057 (Bruno Santos)
 * Funções de gestão do cesto de compras.
 * Este ficheiro implementa as operações relacionadas com o cesto,
 * incluindo o cálculo de preços com IVA, listagem dos produtos
 * presentes no cesto, atualização de quantidades e execução do
 * comando 'a'.
*/

#include "sistema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/** CESTO */
/**
 * Arredonda um valor para duas casas decimais (cêntimos).
 * Aplica arredondamento simétrico, tratando valores positivos e negativos
 * de forma consistente. Multiplica por 100, aplica arredondamento e divide
 * novamente por 100.
 * @param x Valor a arredondar.
 * @return Valor arredondado às centésimas.
*/
double arredonda_centimos(double x) {
    if (x >= 0) {
        return (long)(x * 100 + ARRED) / 100.0;
    } 
    else
        return (long)(x * 100 - ARRED) / 100.0;
}

/**
 * Calcula o valor total com IVA de um produto.
 * Multiplica a quantidade pelo preço unitário e aplica a taxa de IVA
 * correspondente ao produto, devolvendo o resultado arredondado
 * às centésimas.
 * @param s Ponteiro para o sistema.
 * @param p Ponteiro para o produto.
 * @param quantidade Quantidade de unidades.
 * @return Valor total com IVA incluído.
*/
double total_com_iva(Sistema *s, Produto *p, int quantidade) {
    int taxa;
    double total;
    taxa = s->iva[p->iva - 'A'];
    total = quantidade * p->preco * (100.0 + taxa) / 100.0;
    return arredonda_centimos(total);
}

/**
 * Imprime um item do cesto.
 * Apresenta o código de IVA, o preço unitário, a quantidade no cesto,
 * o valor total com IVA e a descrição do produto.
 * @param s Ponteiro para o sistema.
 * @param p Ponteiro para o produto.
*/
void imprimir_item_cesto(Sistema *s, Produto *p) {
    double total = total_com_iva(s, p, p->cesto);
    printf("%c %.2f %d %.2f %s\n",
           p->iva, p->preco, p->cesto, total, p->descricao);
}

/**
 * Lista os produtos atualmente presentes no cesto.
 * Recolhe os produtos ativos com quantidade positiva no cesto,
 * ordena-os por código EAN e imprime cada item ordenado.
 * @param s Ponteiro para o sistema.
*/
void listar_cesto(Sistema *s) {
    int i, pos, n_indices = 0;
    int indices[MAXPRODUTOS], aux[MAXPRODUTOS];
    for (i = 0; i < s->n_itens_cesto; i++) {
        pos = s->itens_cesto[i]; /** índice do produto no vetor produtos */
        if (s->produtos[pos].ativo && s->produtos[pos].cesto > 0) {
            /** guarda produtos válidos para ordenar */
            indices[n_indices++] = pos;
        }
    }
    if (n_indices > 1) {
        ordenar(s, indices, aux, 0, n_indices - 1, ORD_PRODUTOS);
    }
    for (i = 0; i < n_indices; i++) {
        imprimir_item_cesto(s, &s->produtos[indices[i]]);
    }
}

/**
 * Regista um produto no vetor de itens do cesto.
 * Adiciona o índice do produto ao vetor itens_cesto apenas se ainda
 * não estiver presente, evitando duplicações, e marca o produto como
 * pertencente à lista do cesto.
 * @param s Ponteiro para o sistema.
 * @param pos Índice do produto no vetor de produtos.
*/
void registar_no_cesto(Sistema *s, int pos) {
    if (!s->produtos[pos].em_lista_cesto) {
        /** adiciona produto ao vetor do cesto */
        s->itens_cesto[s->n_itens_cesto++] = pos;
        s->produtos[pos].em_lista_cesto = 1;
    }
}


/**
 * Altera a quantidade de um produto no cesto.
 * Valida o código EAN, verifica se o produto existe e atualiza as
 * quantidades em stock e no cesto. Se a quantidade for positiva,
 * adiciona unidades ao cesto; se for negativa, remove unidades do
 * cesto e repõe-as em stock.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto.
 * @param quantidade Quantidade a adicionar ou remover.
 * @return 1 em caso de sucesso, 0 em caso de erro.
*/
int alterar_cesto(Sistema *s, char *ean, int quantidade) {
    if (!ean_valido(ean)) {
        printf("%s\n", EINV_EAN);
        return 0;
    }
    int pos = procurar_produto(s, ean);
    if (pos == -1) {
        printf("%s: no such product\n", ean);
        return 0;
    }
    if (quantidade >= 0) { /**adicionar ao cesto(quantidade positiva ou zero)*/
        if (s->produtos[pos].stock < quantidade) {
            printf("%s\n", ENOSTOCK);
            return 0;
        }
        s->produtos[pos].stock -= quantidade; /** retira do stock */
        s->produtos[pos].cesto += quantidade; /** adiciona ao cesto */
        if (s->produtos[pos].cesto > 0) registar_no_cesto(s, pos);
    }
    else { /** caso remover do cesto (quantidade negativa) */       
        if (s->produtos[pos].cesto < -quantidade) { /** verifica se há */
            printf("%s\n", EINV_QTY); /** quantidade suficiente no cesto */
            return 0;
        } 
        s->produtos[pos].stock += -quantidade; /** repõe no stock */
        s->produtos[pos].cesto -= -quantidade; /** remove do cesto */
    }
    imprimir_item_cesto(s, &s->produtos[pos]);
    return 1;
}

/**
 * Executa o comando 'a'.
 * Sem argumentos, lista os produtos atualmente presentes no cesto.
 * Com um argumento, adiciona uma unidade do produto indicado.
 * Com dois argumentos, altera a quantidade do produto no cesto
 * usando a quantidade e o código EAN fornecidos.
 * Formato de entrada: a [ [ <quantidade> ] <ean> ]
 * Formato de saída:<iva> <preco-unitario><quantidade><total-com-iva><descricao>
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_a(Sistema *s, char *linha) {
	char arg1[MAXLINHA]; /** pode ser quantidade ou EAN */
	char arg2[MAXLINHA]; /** usado como EAN quando há quantidade */
	char ean[EAN13 + 1]; /** EAN final a usar */
	int n;
	int quantidade;
	n = sscanf(linha, "a %s %s", arg1, arg2);
	if (n <= 0) { /** sem argumentos: listar cesto */
		listar_cesto(s);
		return;
	}
	if (n == 1) { /** um argumento: é o EAN, quantidade = 1 */
		quantidade = 1;
		strcpy(ean, arg1);
	}
	else { /** dois argumentos: arg1 = quantidade, arg2 = EAN */
		quantidade = atoi(arg1); /** converte string para inteiro */
		strcpy(ean, arg2);
	}
	alterar_cesto(s, ean, quantidade);
}