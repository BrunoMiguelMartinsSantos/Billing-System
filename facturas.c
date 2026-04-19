/**
 * @file facturas.c
 * @author ist1118057
 * Funções de gestão de clientes e facturas.
 * Este ficheiro implementa a leitura e validação dos dados do cliente,
 * a criação e listagem de facturas, a remoção de facturas e produtos,
 * e as operações auxiliares necessárias ao processamento dos comandos
 * 'f', 'c' e 'd'.
*/

#include "sistema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/** CLIENTES */
/** 
 * Cria uma cópia dinâmica de uma string.
 * Aloca memória suficiente para armazenar a string original,
 * copia o seu conteúdo e devolve o ponteiro para a nova string.
 * @param s String a copiar.
 * @return Ponteiro para a nova string ou NULL em caso de erro.
*/
char *copia_string(char *s) {
    char *novo;
    novo = malloc((strlen(s) + 1) * sizeof(char));
    if (novo == NULL)
        return NULL; /** falha na alocação de memória */
    strcpy(novo, s);
    return novo;
}

/**
 * Verifica se um NIF é válido.
 * Um NIF válido deve ter exatamente 9 dígitos e conter apenas
 * caracteres numéricos.
 * @param nif String com o NIF.
 * @return 1 se for válido, 0 caso contrário.
*/
int nif_valido(char *nif) {
    if (strlen(nif) != MAXNIF) {
        return 0;
    }
    if (!so_digitos(nif)) {
        return 0;
    }
    if (nif[0] == '0') {
       return 0;
    }
    return 1;
}

/**
 * Verifica se o nome do cliente é válido.
 * Um nome válido não pode ser vazio e deve começar por uma letra.
 * @param nome String com o nome do cliente.
 * @return 1 se for válido, 0 caso contrário.
*/
int nome_valido(char *nome) {
    if (nome[0] == '\0') 
        return 0;
    if (!isalpha(nome[0]))
        return 0;
    return 1;
}

/**
 * Extrai o nome do cliente a partir da linha de comando.
 * Se o nome estiver entre aspas, preserva os espaços internos.
 * Caso contrário, copia o restante conteúdo da linha como nome.
 * @param p Ponteiro para a posição atual na linha.
 * @param nome Buffer onde o nome será armazenado.
 * @return 1 em caso de sucesso, 0 em caso de erro.
*/
int extrair_nome(char *p, char *nome) {
    char *fim; /** aponta para o fim do nome (quando há aspas) */
    p = saltar_espacos(p);
    if (*p == '\0' || *p == '\n') {
        return 0;
    }
    if (*p == '"') {
        p++;
        fim = p;
        while (*fim != '\0' && *fim != '\n' && *fim != '"') {
            fim++;
        }
        if (*fim != '"') {
            return 0;
        }
        /** copia os caracteres entre p e fim */
        copiar_intervalo(nome, p, fim - p);
        return 1;
    }
    strcpy(nome, p);
    tirar_newline(nome);
    return 1;
}

/**
 * Lê um NIF a partir da posição atual da string.
 * Consome dígitos consecutivos, valida o NIF e avança o ponteiro original.
 * @param p Ponteiro para o ponteiro da string (permite avançar a posição).
 * @param nif Ponteiro onde será guardado o valor do NIF lido.
 * @return 1 em caso de sucesso, 0 se não houver NIF, -1 se for inválido.
 */
int ler_nif_inicial(char **p, int *nif) {
    char nif_str[MAXLINHA];
    int k = 0;
    if (!isdigit(**p)) {
        return 0;
    }
    while (isdigit(**p)) {
        /** guarda o carácter atual em nif_str[k] e incrementa k. */
        nif_str[k++] = **p;
        (*p)++; /** avança o ponteiro original para o próximo carácter. */
    }
    nif_str[k] = '\0';
    /** se começou por dígitos, o que vem a seguir tem de ser separado */
    if (!(**p == ' ' || **p == '\t' || **p == '\n' || **p == '\0')) {
        printf("%s: %s\n", nif_str, ENOSUCH_NIF);
        return -1;
    }
    if (!nif_valido(nif_str)) {
        printf("%s: %s\n", nif_str, ENOSUCH_NIF);
        return -1;
    }
    *nif = atoi(nif_str);
    return 1;
}

/** 
 * Lê e valida o nome de um cliente.
 * Extrai o nome (com ou sem aspas) e verifica se cumpre as regras
 * de validação.
 * @param p Ponteiro para a posição atual na string.
 * @param nome Buffer onde o nome será armazenado.
 * @return 1 em caso de sucesso, 0 em caso de erro.
 */

int ler_nome_cliente(char *p, char *nome) {
    if (!extrair_nome(p, nome)) {
        printf("%s\n", EINV_NAME);
        return 0;
    }
    if (!nome_valido(nome)) {
        printf("%s\n", EINV_NAME);
        return 0;
    }
    return 1;
}

/**
 * Valida e guarda um NIF.
 * Recebe uma string com um possível NIF, verifica se é válido e,
 * em caso afirmativo, converte-o para inteiro e guarda-o na variável nif.
 * @param arg String que contém o NIF a validar.
 * @param nif Ponteiro onde será guardado o valor inteiro do NIF.
 * @return 1 se o NIF for válido, 0 caso contrário.
*/
static int guardar_nif(char *arg, int *nif) {
	if (!nif_valido(arg)) {
		printf("%s: %s\n", arg, ENOSUCH_NIF);
		return 0;
	}
	*nif = atoi(arg);
	return 1;
}

/**
 * Lê os dados do cliente a partir da linha de comando.
 * Inicializa o NIF e o nome com os valores por defeito e, conforme
 * o conteúdo da linha, tenta ler um nome, um NIF, ou ambos.
 * @param linha Linha de entrada com o comando.
 * @param nif Ponteiro onde será guardado o NIF lido.
 * @param nome Buffer onde será guardado o nome do cliente.
 * @return 1 em caso de sucesso, 0 em caso de erro.
*/
int ler_dados_cliente(char *linha, int *nif, char *nome) {
	char *p, *inicio, arg1[MAXLINHA];
	*nif = NIF_FINAL;
	strcpy(nome, NOME_FINAL);
	p = saltar_espacos(linha + 1);
	if (*p == '\0' || *p == '\n') {
		return 1;
	}
	if (*p == '"') {
		return ler_nome_cliente(p, nome);
	}
	inicio = p;
	while (*p != '\0' && *p != '\n' && *p != ' ' && *p != '\t') {
		p++;
	}
	copiar_intervalo(arg1, inicio, p - inicio);
	p = saltar_espacos(p);
	if (*p != '\0' && *p != '\n') {
		return guardar_nif(arg1, nif) && ler_nome_cliente(p, nome);
	}
	if (so_digitos(arg1)) {
		return guardar_nif(arg1, nif);
	}
	return ler_nome_cliente(inicio, nome);
}

/** FACTURAS */
/**
 * Garante capacidade no vetor dinâmico de facturas.
 * Se o vetor estiver cheio, aumenta a sua capacidade (duplicando-a
 * ou inicializando-a, se for a primeira alocação).
 * @param s Ponteiro para o sistema.
 * @return 1 em caso de sucesso, 0 se falhar a alocação de memória.
*/
int garantir_espaco_facturas(Sistema *s) {
    Factura *aux; /** ponteiro auxiliar que guarda o resultado de realloc */
    int novo_espaco; /** guarda a nova capacidade do vetor */
    if (s->n_facturas < s->espaco_facturas) {
        return 1;
    }
    if (s->espaco_facturas == 0)  {
        novo_espaco = FACTURAS_INICIAIS;
    }
    else {
        novo_espaco = s->espaco_facturas * 2;
    }
    /** Tenta redimensionar o vetor de facturas */
    aux = realloc(s->facturas, novo_espaco * sizeof(Factura));
    if (aux == NULL) {
        return 0;
    }
    /** Atualiza o ponteiro principal para o novo bloco de memória */
    s->facturas = aux;
    /** Atualiza a capacidade do vetor */
    s->espaco_facturas = novo_espaco;
    return 1;
}

/**
 * @brief Calcula os dados de uma fatura a partir do cesto.
 * Determina o número total de itens e o valor total com IVA dos
 * produtos atualmente presentes no cesto, arredondando o total
 * final aos cêntimos.
 * @param s Ponteiro para o sistema.
 * @param n_items Ponteiro onde será guardado o número total de itens.
 * @param total Ponteiro onde será guardado o valor total da fatura.
*/
void calcular_dados_factura(Sistema *s, int *n_items, double *total) {
    int i, pos;
    *n_items = 0;
    *total = 0.0;
    for (i = 0; i < s->n_itens_cesto; i++) {
        pos = s->itens_cesto[i];
        if (s->produtos[pos].ativo && s->produtos[pos].cesto > 0) {
            *n_items += s->produtos[pos].cesto;
            *total += total_com_iva(s, 
                                    &s->produtos[pos], 
                                    s->produtos[pos].cesto);
        }
    }
    *total = arredonda_centimos(*total);
}

/**
 * Transfere os produtos do cesto para vendidos.
 * Para cada produto no cesto, soma a quantidade ao total de vendidos
 * e limpa o cesto, removendo também o produto da lista do cesto.
 * @param s Ponteiro para o sistema.
*/
void transferir_cesto_para_vendidos(Sistema *s) {
    int i, pos;
    for (i = 0; i < s->n_itens_cesto; i++) {
        pos = s->itens_cesto[i];
        if (s->produtos[pos].ativo && s->produtos[pos].cesto > 0) {
            s->produtos[pos].vendidos += s->produtos[pos].cesto;
            s->produtos[pos].cesto = 0;
            s->produtos[pos].em_lista_cesto = 0;
        }
    }
    s->n_itens_cesto = 0;
}

/**
 * Inicializa uma fatura com os dados fornecidos.
 * Preenche os campos principais da fatura e marca-a como ativa.
 * @param f Ponteiro para a fatura.
 * @param numero Número da fatura.
 * @param nif NIF do cliente.
 * @param nome Nome do cliente (já alocado).
 * @param n_items Número total de itens.
 * @param total Valor total da fatura.
*/
void preencher_factura(Factura *f, int numero, int nif, 
                       char *nome, int n_items, double total) {
    f->numero = numero;
    f->nif = nif;
    f->nome = nome;
    f->n_items = n_items;
    f->total = total;
    f->ativa = 1;
}

/**
 * Executa o comando 'f' (emitir fatura).
 * Lê os dados do cliente (NIF e/ou nome), calcula o total do cesto,
 * cria uma nova fatura e transfere os produtos do cesto para vendidos,
 * atualizando os totais do sistema.
 * Formato de entrada: f [[ <nif> ] <nome-cliente> ]
 *  - Sem argumentos: usa NIF e nome por defeito.
 *  - Com um argumento: pode ser NIF ou nome.
 *  - Com dois argumentos: primeiro NIF, depois nome.
 * Formato de saída:
 * <numero-de-produtos> <valor-pago> <numero-de-factura>
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_f(Sistema *s, char *linha) {
    char nome[MAXLINHA], *nome_copiado;
    int nif, n_items;
    double total;
    Factura *f;
    if (!ler_dados_cliente(linha, &nif, nome)) {
        return;
    }
    if (strcmp(nome, "error") == 0) {
        return;
    }
    if (!garantir_espaco_facturas(s)) {
        printf("%s\n", ENOMEM);
        return;
    }
    nome_copiado = copia_string(nome);
    if (nome_copiado == NULL) {
        printf("%s\n", ENOMEM);
        return;
    }
    calcular_dados_factura(s, &n_items, &total);
    f = &s->facturas[s->n_facturas]; /** obter posição da nova fatura */
    preencher_factura(f, s->proxima_factura, nif, nome_copiado, n_items, total);
    transferir_cesto_para_vendidos(s);
    s->n_facturas++; /** atualiza número total de facturas */
    s->proxima_factura++; /** atualiza número total de facturas */
    s->total_items += n_items; /** incrementa o número da próxima factura */
    s->total_facturado = arredonda_centimos(s->total_facturado + total);
    printf("%d %.2f %d\n", n_items, total, f->numero);
}

/**
 * Imprime os dados principais de uma fatura.
 * Apresenta o número da fatura, o valor total e o nome do cliente.
 * @param f Ponteiro para a fatura.
*/
void imprimir_factura(Factura *f) {
    printf("%d %.2f %s\n", f->numero, f->total, f->nome);
}

/**
 * Lista todas as faturas ativas do sistema.
 * Recolhe os índices das faturas ativas, ordena-os por nome do cliente
 * e, em caso de empate, pelo número da fatura, e imprime cada fatura.
 * @param s Ponteiro para o sistema.
 * @return 1 em caso de sucesso, 0 em caso de erro de memória.
*/
int listar_facturas_ativas(Sistema *s) {
    int i, idx, n_indices;
    int *indices, *aux; /** vetores dinâmicos para ordenação */
    if (s->n_facturas == 0) {
        return 1;
    }
    indices = malloc(s->n_facturas * sizeof(int));
    aux = malloc(s->n_facturas * sizeof(int));
    if (indices == NULL || aux == NULL) {
        free(indices);
        free(aux);
        return 0;
    }
    n_indices = 0;
    for (i = 0; i < s->n_facturas; i++) {
        if (s->facturas[i].ativa) { /** seleciona apenas facturas ativas */
            indices[n_indices++] = i;
        }
    }
    if (n_indices > 1) { /** se houver mais de uma factura, ordena os índices */
        ordenar(s, indices, aux, 0, n_indices - 1, ORD_FACTURAS);
    }
    for (i = 0; i < n_indices; i++) {
        idx = indices[i];
        imprimir_factura(&s->facturas[idx]);
    }
    free(indices);
    free(aux);
    return 1;
}

/**
 * Lista as faturas ativas de um cliente.
 * Percorre o vetor de faturas e imprime apenas as que estão ativas
 * e cujo nome do cliente coincide com o nome fornecido.
 * @param s Ponteiro para o sistema.
 * @param nome Nome do cliente.
 * @return 1 se encontrar pelo menos uma fatura, 0 caso contrário.
*/
int listar_facturas_cliente(Sistema *s, char *nome) {
    int i, encontrou;
    encontrou = 0;
    for (i = 0; i < s->n_facturas; i++) {
        if (s->facturas[i].ativa && strcmp(s->facturas[i].nome, nome) == 0) {
            imprimir_factura(&s->facturas[i]);
            encontrou = 1;
        }
    }
    return encontrou;
}

/**
 * Executa o comando 'c'.
 * Sem argumentos, lista todas as faturas ativas do sistema.
 * Com um nome, extrai e valida o nome do cliente e lista apenas
 * as faturas ativas associadas a esse cliente.
 * Formato de entrada: c [ <nome-cliente> ]
 * Formato de saída: <numero-fatura> <valor-faturado> <nome-cliente>
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_c(Sistema *s, char *linha) {
    char nome[MAXLINHA];
    char *p;
    p = linha + 1;
    p = saltar_espacos(p);
    /** caso sem argumentos: listar todas as facturas ativas */
    if (*p == '\0' || *p == '\n') { 
        if (!listar_facturas_ativas(s)) {
            printf("%s\n", ENOMEM);
        }
        return; /** se não houver facturas ativas, não imprime nada */
    }
    if (!extrair_nome(p, nome)) { /** extrai o nome do cliente */
        printf("%s\n", EINV_NAME);
        return;
    }
    if (!nome_valido(nome)) { /** valida o nome */
        printf("%s\n", EINV_NAME);
        return;
    }
    if (!listar_facturas_cliente(s, nome)) { /** lista facturas do cliente */
        printf("%s: no such client\n", nome);
    }
}

/**
 * Procura uma fatura ativa pelo seu número.
 * Converte o número da fatura no índice correspondente no vetor
 * de faturas e verifica se a fatura existe e está ativa.
 * @param s Ponteiro para o sistema.
 * @param numero Número da fatura.
 * @return Índice da fatura se existir e estiver ativa, -1 caso contrário.
*/
int procurar_factura(Sistema *s, int numero) {
    int idx = numero - 1;
    if (idx < 0 || idx >= s->n_facturas) {
        return -1;
    }
    if (!s->facturas[idx].ativa) {
        return -1;
    }
    return idx;
}

/**
 * Apaga (desativa) uma fatura pelo seu número.
 * Procura a fatura ativa correspondente, imprime os seus dados,
 * atualiza os totais do sistema e marca a fatura como inativa.
 * @param s Ponteiro para o sistema.
 * @param numero Número da fatura a apagar.
*/
void apagar_factura(Sistema *s, int numero) {
    int idx; /** índice da factura encontrada */
    double total;
    idx = procurar_factura(s, numero); /** procura a factura pelo número */
    if (idx == -1) {
        printf("%d: no such invoice\n", numero);
        return;
    }
    /** imprime os dados da factura antes de apagar */
    printf("%.2f %d %s\n", 
           s->facturas[idx].total,
           s->facturas[idx].nif,
           s->facturas[idx].nome);
    /** atualiza o total de itens do sistema */
    s->total_items -= s->facturas[idx].n_items;
    /** atualiza o total de itens do sistema */
    total = s->total_facturado - s->facturas[idx].total; 
    s->total_facturado = arredonda_centimos(total);
    s->facturas[idx].ativa = 0;  /** marca a factura como inativa */
}

/**
 * Remove stock de um produto identificado pelo código EAN.
 * Valida o EAN, verifica se o produto existe, se não está em uso
 * no cesto e se a quantidade a remover é válida. Atualiza o stock
 * e desativa o produto caso o stock chegue a zero.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto.
 * @param quantidade Quantidade a remover.
*/
void remover_stock_produto(Sistema *s, char *ean, int quantidade) {
    int pos;
    if (!ean_valido(ean)) {
        printf("%s\n", EINV_EAN);
        return;
    }
    pos = procurar_produto(s, ean);
    if (pos == -1) {
        printf("%s: no such product\n", ean);
        return;
    }
    if (s->produtos[pos].cesto > 0) {
        printf("%s\n", EPROD_USE);
        return;
    }
    if (quantidade <= 0 || quantidade > s->produtos[pos].stock) {
        printf("%s\n", EINV_QTY);
        return;
    }
    s->produtos[pos].stock -= quantidade;
    printf("%d %s\n",
           s->produtos[pos].stock,
           s->produtos[pos].descricao);
    if (s->produtos[pos].stock == 0) {
        s->produtos[pos].ativo = 0;
        s->n_ativos--;
    }
}

/**
 * Executa o comando 'd'.
 * Com um argumento, remove uma fatura ativa pelo seu número.
 * Com dois argumentos, retira stock a um produto identificado
 * pelo código EAN.
 * Formato de entrada: d <numero> [ <quantidade> ]
 * Formato de saída:
 * - Com um argumento:
 *   <valor-pago> <nif> <nome-cliente>
 *   (a fatura é removida, sem repor os itens comprados)
 * - Com dois argumentos:
 *   <quantidade-restante> <descricao>
 *   (se o stock ficar a zero, o produto é removido dos ativos)
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_d(Sistema *s, char *linha) {
	char arg1[MAXLINHA];
	char arg2[MAXLINHA];
	char ean[EAN13 + 1];
	int n;
	int quantidade;
	int numero;
	n = sscanf(linha, "d %s %s", arg1, arg2);
	if (n == 1) {
        /** converte argumento para número da factura */
		numero = atoi(arg1);
		apagar_factura(s, numero);
		return;
	}
	if (n == 2) {
		strcpy(ean, arg1);
		quantidade = atoi(arg2); /** segundo argumento é a quantidade */
		remover_stock_produto(s, ean, quantidade);
	}
}