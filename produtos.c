/**
 * @file produtos.c
 * @author ist1118057 (Bruno Santos)
 * Funções de gestão e listagem de produtos.
 * Este ficheiro inclui as funções responsáveis pela validação de códigos
 * EAN, criação e atualização de produtos, cálculo de informação para
 * resumos e listagem de produtos disponíveis no sistema.
*/

#include "sistema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Calcula a soma ponderada dos dígitos de um código EAN.
 * Percorre todos os dígitos, exceto o último, aplicando peso 1 às
 * posições pares e peso 3 às posições ímpares. A soma obtida é usada
 * no cálculo do dígito de controlo.
 * @param ean String com o código EAN.
 * @param len Comprimento do código.
 * @return Soma ponderada dos dígitos.
*/
int calcular_soma_ean(char *ean, int len) {
    int i, soma = 0;
    for (i = 0; i < len - 1; i++) {
        if (i % 2 == 0) 
            soma += ean[i] - '0';
        else 
            soma += 3 * (ean[i] - '0');
    }
    return soma;
}

/**
 * Verifica se um código EAN-8 ou EAN-13 é válido.
 * Valida o comprimento do código, confirma que contém apenas dígitos
 * e calcula o dígito de controlo, comparando-o com o último dígito.
 * @param ean String com o código EAN.
 * @return 1 se o código for válido, 0 caso contrário.
*/
int ean_valido(char *ean) {
    int len, soma, controlo, ultimo;
    len = strlen(ean);
    if (len != EAN8 && len != EAN13) {
        return 0;
    }
    if (!so_digitos(ean)) {
        return 0;
    }
    soma = calcular_soma_ean(ean, len);
    controlo = (10 - (soma % 10)) % 10;
    ultimo = ean[len - 1] - '0';
    return controlo == ultimo;
}


/**
 * Valida os dados de um produto.
 * Verifica se o código EAN, o código de IVA, o preço, a quantidade
 * e a descrição respeitam as regras do sistema. Em caso de erro,
 * imprime a mensagem correspondente e devolve 0.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto.
 * @param iva Código de IVA do produto.
 * @param preco Preço unitário do produto.
 * @param quantidade Quantidade em stock.
 * @param descricao Descrição do produto.
 * @return 1 se os dados forem válidos, 0 caso contrário.
*/
int validar_dados_produto(Sistema *s, char *ean, char iva, double preco,
                          int quantidade, char *descricao) {
    if (!ean_valido(ean)) {
        printf("%s\n", EINV_EAN);
        return 0;
    }
    if (!iva_existe(s, iva)) {
        printf("%s\n", EINV_IVA);
        return 0;
    }
    if (preco <= 0) {
        printf("%s\n", EINV_PRICE);
        return 0;
    }
    if (quantidade < 0) {
        printf("%s\n", EINV_QTY);
        return 0;
    }
    if (!descricao_valida(descricao)) {
        printf("%s\n", EINV_DESC);
        return 0;
    }
    return 1;
}

/**
 * Cria um novo produto no sistema.
 * Verifica se existe espaço para um novo registo, inicializa os campos
 * do produto, insere-o na tabela de dispersão e atualiza os contadores
 * do sistema.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto.
 * @param iva Código de IVA do produto.
 * @param preco Preço unitário do produto.
 * @param quantidade Quantidade inicial do produto.
 * @param descricao Descrição do produto.
 * @return 1 em caso de sucesso, 0 caso contrário.
*/
int criar_produto_novo(Sistema *s, char *ean, char iva, double preco,
                       int quantidade, char *descricao) {
    int i;
    if (s->n_produtos >= MAXPRODUTOS) {
        printf("%s\n", EINV_PROD);
        return 0;
    }
    i = s->n_produtos;
    strcpy(s->produtos[i].ean, ean);
    s->produtos[i].iva = iva;
    s->produtos[i].preco = preco;
    s->produtos[i].stock = quantidade;
    s->produtos[i].vendidos = 0;
    s->produtos[i].cesto = 0;
    strcpy(s->produtos[i].descricao, descricao);
    s->produtos[i].ativo = 1;
    s->produtos[i].em_lista_cesto = 0;
    if (!inserir_hash_produto(s, ean, i)) {
        printf("%s\n", ENOMEM);
        return 0;
    }
    s->n_produtos++;
    s->n_ativos++;
    printf("%d\n", s->produtos[i].stock);
    return 1;
}

/**
 * Atualiza os dados de um produto já existente.
 * Se o produto estiver presente no cesto, a alteração do preço não é
 * permitida. Caso contrário, atualiza o stock, o código de IVA, o
 * preço e a descrição do produto, imprimindo o stock resultante.
 * @param s Ponteiro para o sistema.
 * @param indice Índice do produto no vetor de produtos.
 * @param iva Novo código de IVA.
 * @param preco Novo preço do produto.
 * @param quantidade Quantidade a acrescentar ao stock.
 * @param descricao Nova descrição do produto.
 * @return 1 em caso de sucesso, 0 em caso de erro.
*/
int atualizar_produto_existente(Sistema *s, int indice, char iva, double preco,
                                int quantidade, char *descricao) {
    if (s->produtos[indice].cesto > 0 && s->produtos[indice].preco != preco) {
        printf("%s\n", EPROD_USE);
        return 0;
    }
    s->produtos[indice].stock += quantidade;
    s->produtos[indice].iva = iva;
    s->produtos[indice].preco = preco;
    strcpy(s->produtos[indice].descricao, descricao);
    printf("%d\n", s->produtos[indice].stock);
    return 1;
}

/**
 * Executa o comando 'p'.
 * Lê os dados de um produto, valida-os e cria um novo registo ou
 * atualiza um produto já existente no sistema.
 * Formato de entrada: p <ean> <iva> <preco> <quantidade> <descricao>
 * Formato de saída: <stock> resultante do produto.
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_p(Sistema *s, char *linha) {
    char ean[EAN13 + 1], iva;
    double preco;
    int quantidade, indice;
    char descricao[MAXLINHA];
    /** lê todos os caracteres até encontrar \n */
    if (sscanf(linha, "p %13s %c %lf %d %[^\n]",
               ean, &iva, &preco, &quantidade, descricao) != 5) {
        return;
    }
    if (!validar_dados_produto(s, ean, iva, preco, 
                               quantidade, descricao)) {
        return;
    }
    indice = procurar_produto(s, ean);
    if (indice != -1) {
        atualizar_produto_existente(s, indice, iva, preco, 
                                    quantidade, descricao);
        return;
    }
    criar_produto_novo(s, ean, iva, preco, quantidade, descricao);
}

/**
 * Executa o comando 'r'.
 * Sem argumentos, imprime o resumo global de faturação do sistema e
 * a tabela de IVA por ordem alfabética. Com um código EAN, imprime
 * a informação de existências e vendas do produto correspondente.
 * Formato de entrada: r [ <ean> ]
 * Formato de saída:
 * Sem argumentos:
 * <total-items-comprados> <num-facturas> <total-faturado>
 * seguido da tabela de Iva: <codigo> <percentagem>%
 * Com argumento:
 * <stock> <vendidos> <descricao>
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_r(Sistema *s, char *linha) {
    char ean[EAN13 + 1];
    int n, pos, i;
    n = sscanf(linha, "r %13s", ean);
    /** caso: r */
    if (n != 1) {
        printf("%d %d %.2f\n", 
               s->total_items, 
               s->proxima_factura - 1, /** total de faturas emitidas */
               s->total_facturado);
        for (i = 0; i < 26; i++) {
            if (s->iva[i] != -1) {
                /** converte índice (0...25) em letra ('A'–'Z') */
                printf("%c %d%%\n", 'A' + i, s->iva[i]);
            }
        }
        return;
    }
    /** caso: r <ean> */
    if (!ean_valido(ean)) {
        printf("%s\n", EINV_EAN);
        return;
    }
    pos = procurar_produto(s, ean);
    if (pos == -1) {
        printf("%s: no such product\n", ean);
        return;
    }
    printf("%d %d %s\n", 
           s->produtos[pos].stock, 
           s->produtos[pos].vendidos + s->produtos[pos].cesto, 
           s->produtos[pos].descricao);
}

/** LISTAGEM */
/**
 * Verifica se uma string corresponde a um padrão com wildcards.
 * O padrão pode conter:
 * '*' que corresponde a qualquer sequência de caracteres (incluindo vazia)
 * '?' que corresponde a exatamente um carácter
 * @param padrao String com o padrão.
 * @param texto String a testar.
 * @return 1 se houver correspondência, 0 caso contrário.
*/
int wildcard_match(char *padrao, char *texto) {
    /** Se o padrão terminou, só há match se o texto também terminou */
    if (*padrao == '\0') {
        return *texto == '\0';
    }
    /** 
     * '*' pode representar:
     * - 0 caracteres: avança no padrão
     * - 1 ou mais: consome um carácter do texto e mantém o '*' 
    */
    if (*padrao == '*') {
        return wildcard_match(padrao + 1, texto) || 
               (*texto && wildcard_match(padrao, texto + 1));
    }
    /** '?' corresponde exatamente a um carácter */
    if (*padrao == '?') {
        return *texto && wildcard_match(padrao + 1, texto + 1);
    }
    /** Caracteres  normais têm de ser iguais */
    if (*padrao == *texto) {
        return wildcard_match(padrao + 1, texto + 1);
    }
    return 0;
}

/**
 * Imprime os dados de um produto.
 * Apresenta o código EAN, código de IVA, preço, quantidade total vendida
 * (incluindo unidades no cesto), stock disponível e descrição.
 * @param p Ponteiro para o produto a imprimir.
*/
void imprimir_produto(Produto *p) {
    printf("%s %c %.2f %d %d %s\n",
           p->ean,
           p->iva,
           p->preco,
           p->vendidos + p->cesto,
           p->stock,
           p->descricao);
}

/**
 * Lista todos os produtos ativos com stock disponível.
 * Percorre o vetor de produtos e imprime aqueles que estão ativos
 * e têm stock superior a zero.
 * @param s Ponteiro para o sistema.
 * @return 1 se existir pelo menos um produto disponível, 0 caso contrário.
*/
int listar_produtos_disponiveis(Sistema *s) {
    int i, encontrou = 0;
    for (i = 0; i < s->n_produtos; i++) {
        if (s->produtos[i].ativo && s->produtos[i].stock > 0) {
            imprimir_produto(&s->produtos[i]);
            encontrou = 1;
        }
    }
    return encontrou;
}

/**
 * Lista produtos ativos cujo EAN corresponde a um padrão.
 * Percorre o vetor de produtos e imprime aqueles que estão ativos,
 * têm stock disponível e cujo código EAN corresponde ao padrão
 * fornecido (com wildcards '*' e '?').
 * @param s Ponteiro para o sistema.
 * @param padrao Padrão a aplicar ao código EAN.
 * @return 1 se existir pelo menos um produto correspondente, 0 caso contrário.
*/
int listar_produtos_padrao(Sistema *s, char *padrao) {
    int i, encontrou = 0;
    for (i = 0; i < s->n_produtos; i++) {
        if (s->produtos[i].ativo &&
            s->produtos[i].stock > 0 &&
            wildcard_match(padrao, s->produtos[i].ean)) {
            imprimir_produto(&s->produtos[i]);
            encontrou = 1;
        }
    }
    return encontrou;
}

/**
 * Executa o comando 'l'.
 * Lista os produtos disponíveis no sistema. Sem argumentos, imprime
 * todos os produtos com stock disponível. Com argumentos, imprime
 * apenas os produtos cujos códigos EAN correspondem aos padrões indicados.
 * Formato de entrada: l [ <wildcard> { <wildcard> } ]
 * Formato de saída: <ean> <iva> <preco> <vendidos> <stock> <descricao>
 * @param s Ponteiro para o sistema.
 * @param linha Linha de entrada com o comando.
*/
void cmd_l(Sistema *s, char *linha) {
	int n; /** número de caracteres lidos por sscanf */
	char padrao[MAXLINHA]; /** guarda cada padrão (palavra) lido */
    /** aponta para o carácter a seguir ao comando 'l' */
	char *p = linha + 1; 
	p = saltar_espacos(p);
	if (*p == '\0' || *p == '\n') { /** sem argumentos */
		if (!listar_produtos_disponiveis(s)) {
			printf("*: no such product\n");
		}
		return; /** termina o comando */
	}
	while (sscanf(p, "%s%n", padrao, &n) == 1) {
        /** caso '*': listar todos os produtos disponíveis */
		if (strcmp(padrao, "*") == 0) { 
			if (!listar_produtos_disponiveis(s)) {
				printf("*: no such product\n");
			}
		}
		else {
			if (!listar_produtos_padrao(s, padrao)) {
				printf("%s: no such product\n", padrao);
			}
		}
        /** avança o ponteiro p pelo número de caracteres lidos */
		p += n; 
        /** salta espaços antes do próximo padrão */
		p = saltar_espacos(p); 
	}
}