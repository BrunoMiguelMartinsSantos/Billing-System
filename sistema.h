#ifndef SISTEMA_H
#define SISTEMA_H

/**
 * @file sistema.h
 * @author ist1118057 (Bruno Santos)
 * Constantes, estruturas e protótipos do sistema de facturação.
 * Este ficheiro define os limites do sistema, mensagens de erro,
 * estruturas de dados e protótipos das funções usadas na gestão de
 * produtos, cesto e facturas. 
*/

#define MAXLINHA 65536
#define MAXPRODUTOS 10000
#define MAXDESCRICAO 50
#define IVA 26 /** 26 posições porque existem 26 letras */
#define IVA_A 0
#define IVA_B 6
#define IVA_C 13
#define IVA_D 23
#define EAN8 8
#define EAN13 13
#define MAXEANSCAN 13
#define MAXNIF 9
/** para inicializar a capacidade inicial do vetor dinâmico de facturas */
#define FACTURAS_INICIAIS 10 
#define ARRED 0.5000001
#define NIF_FINAL 999999999
#define NOME_FINAL "Cliente final"
#define ORD_PRODUTOS 0
#define ORD_FACTURAS 1
/** número primo ajuda a minimizar colisões e distribuir melhor os valores */
#define HASH_PROD 10007
/**
 * Base utilizada na função de dispersão.
 * Constante usada no cálculo do hash (h = HASH_BASE * h + c).
 * O valor 127, por ser primo, ajuda a obter uma melhor distribuição
 * dos elementos na tabela.
*/
#define HASH_BASE 127

#define ENOMEM "No memory"
#define EINV_EAN "invalid ean"
#define EINV_IVA "invalid iva"
#define EINV_PRICE "invalid price"
#define EINV_QTY "invalid quantity"
#define EINV_DESC "invalid description"
#define EPROD_USE "product in use"
#define EINV_PROD "invalid product"
#define ENOSTOCK "no stock"
#define EINV_NAME "invalid name"
#define ENOSUCH_NIF "no such nif"

/**
 * Nó da lista ligada da tabela de dispersão de produtos.
 * Cada nó armazena um código EAN e o índice do respetivo
 * produto no vetor de produtos, permitindo tratar colisões
 * na tabela hash através de encadeamento.
*/
typedef struct NoHashProd {
    char ean[EAN13 + 1];
    int indice_produto;
    struct NoHashProd *prox; /** Ponteiro para o próximo nó. */
} NoHashProd;
 
/**
 * Estrutura que representa um produto do sistema.
 * Contém a informação essencial de um produto, incluindo identificação,
 * preço, stock e estado, sendo utilizada no vetor de produtos.
*/
typedef struct {
    char ean[EAN13 + 1];     
    char iva; /** classe de IVA (A, B, C, D, …) */
    double preco; /** preço sem IVA */
    int stock;
    int vendidos;
    int cesto;
    char descricao[MAXDESCRICAO + 1]; /** descrição do produto */
    /** indica se o produto existe ou foi removido (1 = existe, 0 = removido) */
    int ativo; 
    int em_lista_cesto;
} Produto;

/**
 * Estrutura que representa uma fatura emitida pelo sistema.
 * Armazena a informação relativa a uma compra, incluindo identificação
 * da fatura, dados do cliente e valores associados.
*/
typedef struct {
    int numero; /** número da fatura */
    int nif;
    char *nome;
    int n_items; /** quantidade de produtos comprados */
    double total; /** valor pago */
    /** indica se fatura existe ou já foi apagada (1 = existe, 0 = apagada) */
    int ativa; 
} Factura;

/**
 * Estrutura principal do sistema de faturação.
 * Armazena os produtos, as faturas, a tabela de IVA,
 * a tabela hash de produtos e a informação associada ao cesto.
*/ 
typedef struct {
    Produto produtos[MAXPRODUTOS];
    int n_produtos; /** guarda quantos produtos já foram criados */
    int n_ativos; /** guarda quantidade de produtos ativos */
    Factura *facturas; 
    int n_facturas; /** número de facturas guardadas */
    int espaco_facturas; /** capacidade do array de facturas */
    int proxima_factura; /** número da próxima factura */
    int iva[IVA]; /** guarda as percentagens de IVA */
    int total_items; /** total de produtos vendidos em todas as facturas */
    double total_facturado; /** soma das faturas */
    NoHashProd *hash_produtos[HASH_PROD];
    int itens_cesto[MAXPRODUTOS]; /** guarda índices dos produtos no cesto */
    int n_itens_cesto; /** contador: diz n.º elementos no cesto */
} Sistema;

/** INICIALIZAÇÃO */
void limpar_tabela_iva(Sistema *s);
void inicializar_sistema(Sistema *s);
void iva_default(Sistema *s);
void ler_ficheiro_iva(Sistema *s, char *nome_ficheiro);
void libertar_hash_produtos(Sistema *s);
void libertar_sistema(Sistema *s);

/** HASH E FUNÇÕES AUXILIARES */
int hash_string(char *s, int tamanho);
int so_digitos(char *s);
int descricao_valida(char *s);
char *saltar_espacos(char *p);
void copiar_intervalo(char *dest, char *origem, int n);
void tirar_newline(char *s);
int iva_existe(Sistema *s, char iva);

/** PRODUTOS */
int inserir_hash_produto(Sistema *s, char *ean, int indice_produto);
int procurar_produto(Sistema *s, char *ean);
int ean_valido(char *ean);
int calcular_soma_ean(char *ean, int len);
int validar_dados_produto(Sistema *s, char *ean, char iva, double preco,
                          int quantidade, char *descricao);
int criar_produto_novo(Sistema *s, char *ean, char iva, double preco,
                       int quantidade, char *descricao);
int atualizar_produto_existente(Sistema *s, int indice, char iva, 
                                double preco, int quantidade, 
                                char *descricao);
void cmd_p(Sistema *s, char *linha);
void cmd_r(Sistema *s, char *linha);

/** LISTAGEM */
int wildcard_match(char *padrao, char *texto);
void imprimir_produto(Produto *p);
int listar_produtos_disponiveis(Sistema *s);
int listar_produtos_padrao(Sistema *s, char *padrao);
void cmd_l(Sistema *s, char *linha);

/** ORDENAÇÃO */
int antes(Sistema *s, int i, int j, int tipo);
void merge(Sistema *s, int v[], int aux[], 
           int esquerda, int meio, int direita, int tipo);
void ordenar(Sistema *s, int v[], int aux[], 
             int esquerda, int direita, int tipo);

/** CESTO */
double arredonda_centimos(double x);
double total_com_iva(Sistema *s, Produto *p, int quantidade);
void imprimir_item_cesto(Sistema *s, Produto *p);
void listar_cesto(Sistema *s);
void registar_no_cesto(Sistema *s, int pos);
int alterar_cesto(Sistema *s, char *ean, int quantidade);
void cmd_a(Sistema *s, char *linha);

/** CLIENTES */
char *copia_string(char *s);
int nif_valido(char *nif);
int nome_valido(char *nome);
int extrair_nome(char *p, char *nome);
int ler_nif_inicial(char **p, int *nif);
int ler_nome_cliente(char *p, char *nome);
int ler_dados_cliente(char *linha, int *nif, char *nome);

/** FACTURAS */
int garantir_espaco_facturas(Sistema *s);
void calcular_dados_factura(Sistema *s, int *n_items, double *total);
void transferir_cesto_para_vendidos(Sistema *s);
void preencher_factura(Factura *f, int numero, int nif, 
                       char *nome, int n_items, double total);
void cmd_f(Sistema *s, char *linha);
void imprimir_factura(Factura *f);
int listar_facturas_ativas(Sistema *s);
int listar_facturas_cliente(Sistema *s, char *nome);
void cmd_c(Sistema *s, char *linha);
int procurar_factura(Sistema *s, int numero);
void apagar_factura(Sistema *s, int numero);
void remover_stock_produto(Sistema *s, char *ean, int quantidade);
void cmd_d(Sistema *s, char *linha);

#endif