#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stackFunc.h"

/** tamanho default referente a cada char de uma string */
#define STRSIZE 32 

/** @brief Função verifica se um número é inteiro ou float
 * @param str recebida como imput
 * @returns tipo
*/
TYPE check_type (char *str) {
    while (*str && *str != ' ') {
        if (*str == '.') {
            return DOUBLE;
        }
        str++;
    }
    return INT;
}

/** @brief controla para quais funções enviar os operadores
 * @param ptr a avaliar 
 * @returns tipo
*/
int check_ptr_type (char * ptr) {
    return (((*ptr) > 64 && (*ptr) < 91) || (*ptr) == 101 || (*ptr) == 58 || (*ptr) == 34 || (*ptr) == 91 );
}

/** @brief Função que cada caratér do input à respetiva função na stack
 * @param linha de input; array de pointers para as várias stackFuncs (dispatch table), stack sobre a qual se vai atuar
 * @returns void
*/
void mainHandler (char * line, stackFunc *dist[], operHandler *handle[], STACK stack) {
    long double val;
    char *ptr = line;
    TYPE t;
    while (*line && *line != '\n') {
        t = check_type(line); // faz check mesmo que nao seja numero, tem de ser do line, não pode ser o ptr!! Ao sair de um array ou bloco, ele não inicia outro mainHandler, por isso o ptr não seria atualizado
        val = strtold(line, &ptr);
        // leu numero, push do número para a stack
        if (val || ptr != line) {
            ptr++; //skip espaços
            push(stack, val, t);
            line = ptr;
        } 
        // leu espaço, salta espaço à frente
        else if ((*ptr) == ' ') line++;
        // leu simbolo, vai para handler de funções stackFunc
        else if ((*ptr) == '[') { 
            line = arrayBuilder(ptr + 2, dist, handle, stack); // começa duas posições à frente para ignorar o caractér '['
        } else if ((*ptr) == '{') {
            line = blockBuilder(ptr + 2, dist, handle, stack); // começa duas posições à frente para ignorar o caractér '{'
        } else if (check_ptr_type(ptr)) {
            handle[((*ptr)-1)/10] (&ptr, dist, handle, stack);
            line = ptr;
        }
        // leu qualquer outro símbolo (só usa um caracter)
        else {
            dist[(*ptr) - 33](stack); //para dar skip ao espaço em que calhou
            line = ptr + 2; // skip ao simbolo usado e skip ao espaço
        }
    }
}

/** @brief Função que constrói um bloco
 * @param lineCrumbs string a utilzar para fazer o bloco
 * @param dist array de pointers para as stackFuncs (dispatch table)
 * @param operHandler que vai trabalhar com os diferentes operadores de funções
 * @param stack a utilizar
 * @returns bloco
*/
char * blockBuilder (char * lineCrumbs, stackFunc *dist[], operHandler *handle[], STACK stack) {
    int i, c = 1;
    for (i = 0; c != 0; i++) { // acaba no caracter a seguir a '}'
        if ((lineCrumbs[i]) == '{') c++;
        if ((lineCrumbs[i]) == '}') c--;
    }
    char *str;
    if (i == 1) {
        str = malloc(sizeof(char)*2);
        str[0] = '\0';
        str[1] = '\0';
    } else {
        str = malloc(sizeof(char)*i);
        memcpy(str, lineCrumbs, i - 2);
        str[i - 2] = '\0';
        str[i - 1] = '\0';
    }
    BLOCKPTR block = malloc(sizeof(blockStruct));
    block->string = str;
    block->handle = handle;
    block->dist = dist;
    generalPush(stack, block, BLOCK);
    return (lineCrumbs + i + 1);
}

/** @brief Função que constroi um array a partir de elementos dentro de '[' e ']' no input
 * @param lineCrumbs string recebida para fazer o array 
 * @param dist array de pointers para dispatch table de funções
 * @param stack a utilizar
 * @returns número depois do ]
*/
char * arrayBuilder (char * lineCrumbs, stackFunc *dist[], operHandler *handle[], STACK stack) {
    STACK newArray = newStack(stack->varList);
    char * init = lineCrumbs;
    int i, c = 1; // c permite resolver nested arrays contando os [ e os ]. começa em 1 pois a funçao inicia com um skip do primeiro [
    for (i = 0; c != 0; i++, lineCrumbs++) {
        if (*lineCrumbs == '[') c ++;
        if (*lineCrumbs == ']') c --;
    }
    lineCrumbs[-1] = '\0'; // na posição da chaveta fica um '\0' para terminar o processamento no mainHandler, n podia ser pos [-2] porque a mainHandler salta para a chaveta, ao processar
    mainHandler(init, dist, handle, newArray);
    lineCrumbs[-1] = ']'; // substituir de volta o '\0' por ']'
    generalPush(stack, newArray, ARRAY);
    return (lineCrumbs + 1); //devolve numero depois do ]
}

/** 
 * @brief Função de print do estado final da stack
 * @param NODE topNode
 * @returns 0 se tudo correr bem
*/
void printStack (STACK stack, printFunc *printDist[]) {
    NODE node;
    int i;
    for (i = 0; i <= stack->sp; i++) {
        node = stack->array+i;
        if (node->type == ARRAY) {
            printStack(node->value, printDist);
        } else {
            printDist[node->type](node->value);
            deleteNode(stack->array+i);
        }
    }
    deleteStack(stack);
}

/**
 * @brief - Função principal do programa
 * @returns 0 se o programa executar corretamente
 */
int main () {
    //inicializar array de variáveis
    NODE varList = malloc(26*sizeof(nodeStruct));
    varListAssign(varList);
    STACK mainStack = newStack(varList);

    //inicialização da dispatch table para StackFunc
    stackFunc *dist[94]; 
    stackFuncDTAssign (dist);

    //incialização da dispatch table para operadores que redirecionam para StackFunc
    operHandler *handle[11];
    operHandlerDTAssign (handle);

    //receber input de ficheiro
    char *line = malloc(STRSIZE*sizeof(char));
    int i, c, chr, size = 32;
    for (i = c = 0; (chr = getc(stdin)) && chr != '\n'; i++, c++) {
        if (c == size - 2) {
            size *= 2;
            line = realloc(line, size*sizeof(char));
        }
        line[i] = chr;
    }
    line[i] = '\n'; // e se nao tiver espaço para ele???????????????????????????????????????????????????????????????????
    line[i+1] = '\0';
    mainHandler(line, dist, handle, mainStack);

    //print final
    // meter numa função auxiliar para organizar?
    printFunc *printDist[33] = {printDouble, printChr, printInt, NULL, printDouble}; //printDist[0]: default é ser double
    printDist[16] = printStr;
    printDist[32] = printBlock;

    printStack(mainStack, printDist);
    deleteVarList(varList); // free da lista de variáveis, tem de se realizar no final porque são variáveis globais
    putchar('\n');
    free(line);
    return 0;
}

/**
 * @brief  Funções de Handle dos operadores / manipulação de variávei
 * @param ptr operador a receber
 * @param dist  array de pointers para dispatch table de funções
 * @param stack a utilizar
 */
void eHandler (char * *ptr, stackFunc *dist[], void * handle, STACK stack) {
    dist[((int) (*((*ptr)+1)%5) + 40)] (stack);
    (*ptr) += 3;
    handle = handle;
}

/** @brief Função que coloca na stack o valor correspondente à variável que se deu input
 * @param array de pointers para as várias stackFuncs, pointer de pointer para a string (a ser lida); é preciso pointer de pointer para poder alterar o valor de ptr dentro desta função
 * @param dist  array de pointers para dispatch table de funções
 * @param stack a utilizar
 * @returns void
*/
void varHandler (char * *ptr, stackFunc *dist[], void * handle, STACK stack) {
    char letra = **ptr;
    if (*(*ptr+1) == '/') { // casos das funções de separar por whitespace e newline
        if ((**ptr) == 83) whiteSpacer (stack);
        else newLineSpacer (stack);
    } else {
        copyNodePush(stack, stack->varList + (letra - 65));
        dist = dist;
        handle = handle;
    }
    (*ptr) += 2;
}

/**
 * @brief Função que associa cada valor ascii de um caratér à respetiva função da stack
 * @param ptr pointer de pointer para a string (a ser lida); é preciso pointer de pointer para poder alterar o valor de ptr dentro desta função
 * @param dist  array de pointers para dispatch table de funções
 * @param stack a utilizar
 * @returns void
*/
void twoDotsHandler (char * *ptr, stackFunc *dist[], void * handle, STACK stack) {
    char letra = *(*ptr+1);
    NODE topNode = stack->sp + stack->array;
    copyNode(stack->varList + (letra - 65), topNode);
    (*ptr) += 3;
    dist = dist;
    handle = handle;
}

// -------- guiao 4 ----------

// talvez meter estas funções noutra dispatch table, já que não usam dist.
// ou então meter ehandler com 4 ifs e tirar o dist de operHandler !!!
//constroi um array a partir de uma string e dá push para a stack
/**
* @brief constroi um array a partir de uma string e dá push para a stack
* @param ptr pointer de pointer para a string (a ser lida); é preciso pointer de pointer para poder alterar o valor de ptr dentro desta função
* @param dist  array de pointers para dispatch table de funções
* @param stack a utilizar
*/
void stringBuilder (char * *ptr, stackFunc *dist[], void * handle, STACK stack) {
    int I;
    char caracter = *(++(*ptr)); // pega no caractér à direita das primeiras aspas
    char *str = malloc(sizeof(char)*(strlen(*ptr) + 1));
    for(I = 0; **ptr != '"'; I++) {
        str[I] = caracter;
        (*ptr)++;
        caracter = **ptr;
    }
    str[I] = '\0';
    generalPush(stack, str, STRING);
    (*ptr)+=2;
    dist = dist;
    handle = handle;
}
