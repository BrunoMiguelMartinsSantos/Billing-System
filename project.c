/* iaed26 - ist1118057 - project */
/** 
 * @file project.c
 * @author ist1118057 (Bruno Santos)
 * project.c - Sistema de facturação.
 * Inicializa o sistema, carrega a tabela de IVA por defeito ou a partir
 * de ficheiro e processa os comandos introduzidos no standard input até
 * ao fim da execução.
*/
#include <stdio.h>
#include "sistema.h"

/**
 * Executa o ciclo principal do programa:
 * inicializa o sistema, processa os comandos e liberta a memória antes
 * de terminar.
 * Caso seja fornecido um argumento na linha de comandos, lê um ficheiro
 * com a configuração de IVA.
 * @param argc Número de argumentos da linha de comandos.
 * @param argv Lista de argumentos da linha de comandos.
 * @return 0 em caso de execução bem-sucedida.
*/
int main(int argc, char *argv[]) { 
    char linha[MAXLINHA];
    Sistema s;
    inicializar_sistema(&s);
    iva_default(&s);
    if (argc > 1) {
        ler_ficheiro_iva(&s, argv[1]);
    }
    while (fgets(linha, MAXLINHA, stdin) != NULL) {
        switch (linha[0]) {
            case 'p': cmd_p(&s, linha); break;
            case 'l': cmd_l(&s, linha); break;
            case 'a': cmd_a(&s, linha); break;
            case 'r': cmd_r(&s, linha); break;
            case 'f': cmd_f(&s, linha); break;
            case 'c': cmd_c(&s, linha); break;
            case 'd': cmd_d(&s, linha); break;
            case 'q':
                libertar_sistema(&s);
                return 0;
        }
    }
    libertar_sistema(&s);
    return 0;
}