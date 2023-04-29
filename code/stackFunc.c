#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stackFunc.h"
#include <ctype.h>

/** @brief Tamnho definido para cada char de uma string
*/
#define STRSIZE 32

// -------- aux ----------

/** @brief Função que associa cada valor ascii de um caratér à respetiva função da stack
 * @param array de pointers para as várias stackFuncs, pointer do pointer para a string de input (a ser lida) 
 * @returns nada
*/
void operHandlerDTAssign (operHandler *handle[]) {
    handle[3] = stringBuilder;
    handle[5] = twoDotsHandler; // leu ':', vai para o handler de atribuir valor a variáveis 
    handle[6] = handle[7] = handle[8] = varHandler; // leu uma variável, vai para handler de variáveis
    handle[10] = eHandler; // leu 'e', vai para eHandler
}

/** @brief Função que associa cada valor ascii de um caratér à respetiva função da stack
 * @param array de pointers para as várias stackFuncs, pointer do pointer para a string de input (a ser lida) 
 * @returns nada
*/
void stackFuncDTAssign (stackFunc *dist[]) {
    dist[0] = negate;
    dist[2] = expo;
    dist[3] = copy_nth;
    dist[4] = mod;
    dist[5] = and;
    dist[6] = simpleDebug; // == apóstrofe
    dist[7] = dec;
    dist[8] = inc;
    dist[9] = mult;
    dist[10] = add;
    dist[11] = range;
    dist[12] = sub;
    dist[13] = debug; // 13 + 33 = 46 = '.' (comando nosso para debug)
    dist[14] = divis;
    dist[26] = pop;
    dist[27] = isLesser;
    dist[28] = isEqual;
    dist[29] = isGreater;
    dist[30] = IfThenElse;
    dist[31] = rotate_top3;
    dist[40] = top2Lower;
    dist[42] = top2Greater;
    dist[43] = top2And;
    dist[44] = top2Or;
    dist[59] = swap_top2;
    dist[61] = xor;
    dist[62] = duplicate;
    dist[66] = toChar;
    dist[69] = toDouble;
    dist[72] = toInt;
    dist[75] = readl;
    dist[79] = printTop;
    dist[82] = toString;
    dist[83] = readAll;
    dist[86] = executew;
    dist[91] = or;
    dist[93] = not;
}

/** @brief Função que inicializa as variáveis da stack com os seus valores pré-definidos
 * @param pointer para o array onde cada posição fixa corresponde a uma estrutrura relativa a uma variável
 * @returns nada
*/
void varListAssign (NODE varList) {
    int I;
    for (I = 0; I < 26; I++) { //alocamos espaço mesmo para as que não usamos
        (varList + I)->value = malloc(sizeof(long double));
        (varList + I)->type = 0;
    }
    *(long double*) ((varList[0]).value) = 10; // A
    varList[0].type = INT;
    *(long double*) ((varList[1]).value) = 11; // B
    varList[1].type = INT;
    *(long double*) ((varList[2]).value) = 12; // C
    varList[2].type = INT;
    *(long double*) ((varList[3]).value) = 13; // D
    varList[3].type = INT;
    *(long double*) ((varList[4]).value) = 14; // E
    varList[4].type = INT;
    *(long double*) ((varList[5]).value) = 15; // F
    varList[5].type = INT;
    *(long double*) ((varList[13]).value) = (long double)'\n'; // N
    varList[13].type = CHAR;
    *(long double*) ((varList[18]).value) = (long double)' '; // S
    varList[18].type = CHAR;
    *(long double*) ((varList[23]).value) = 0; // X
    varList[23].type = INT;
    *(long double*) ((varList[24]).value) = 1;  // Y
    varList[24].type = INT;
    *(long double*) ((varList[25]).value) = 2; // Z
    varList[25].type = INT;
    // as restantes variáveis não são atribuidos valores, nem utilizadas
}

/**
 * @brief Retorna tamanho de um objeto usado pela stack (já em bytes) Nota: para arrays retorna size, não sp
 * @param node do qual queremos saber o tamanho **do seu valor**
 * @returns tamanho em bytes
*/
int getSize (NODE node) {
    if (IS_NUM(node)) {
        return sizeof(long double); //long double
    } else if (node->type == STRING) {
        return (strlen((char *)node->value) + 1);
    } else  if (node->type == ARRAY) { //falta caso para bloco, este é para array, retorna atributo size
        STACK stack = node->value;
        return (stack->size*(sizeof(nodeStruct)));
    }
    else return 0; // este ultimo caso funciona se for preciso mas malloc(0) deve dar asneira
}

/**
 * @brief Faz free do valor do node
 * @param node que se pretende apagar
 * @returns void
*/
void deleteNode (NODE node) {
    if (node->type == ARRAY) {
        fullDeleteStack(node->value);
    } else if (node->type == BLOCK) {
        BLOCKPTR block = node->value;
        free(block->string);
        free(block);
    } else {
        free(node->value);
    }
    node->type = 0;
}

/**
 * @param stack cujas variáveis se pretendem apagar
 * @returns void
 * @brief Apaga a varList da stack, e os valores associados a cada variável
*/
void deleteVarList (NODE varList) {
    int i;
    for (i = 0; i < 26; i ++) { // free do valor de cada variável
        deleteNode(varList + i);
    }
    free(varList);
}

/**
 * @brief Apaga a stack, porém nao faz free do valor dos nodes individualmente. Não apaga a varList da stack (senão apagaria o valor das variáveis em nested stacks)
 * @param stack que se pretende apagar
 * @returns void
*/
void deleteStack (STACK stack) {
    free(stack->array); // free do array
    free(stack); // free da estrutura da stack
}

/**
 * @brief Apaga a stack, incluindo o valor dos nodes individualmente. Apaga a varList da stack.
 * @param stack que se pretende apagar
 * @returns void
*/
void fullDeleteStack (STACK stack) {
    while (stack->sp >= 0) {
        deleteNode(stack->array + (stack->sp--));
    }
    deleteStack(stack);
}

//A stack resultante é igual mas independente da original (aloca-se a informação toda para memória nova).
/**
 * @brief Copia uma stack (array)
 * @param stack (array) que queremos copiar
 * @returns stack nova
*/
STACK copyArray (STACK stack) {
    //nao uso newStack pq depois ia precisar de realloc
    STACK newStack = malloc(sizeof(stackStruct));
    newStack->sp = stack->sp;
    newStack->array = malloc(stack->size*sizeof(nodeStruct));
    newStack->size = stack->size;
    newStack->varList = stack->varList;
    int i;
    NODE dest, src;
    for (i = 0; i <= stack->sp; i++) { //parse por todos os elementos de ambos os arrays
        dest = newStack->array + i;
        src = stack->array + i;
        // == copyNode sem free
        TYPE t = src->type;
        if (t == ARRAY) {
            dest->value = copyArray(src->value);
        } else  if (t == BLOCK) {
            dest->value = copyBlock(src->value);
        } else {
            int size = getSize(src);
            dest->value = malloc(size);
            memcpy(dest->value, src->value, size);
        }
        dest->type = src->type;
    }
    /*
    for (i = 0; i < 26; i ++) {
        src = stack->varList + i;
        dest = newStack->varList + i;
        if (src->type == 0) {
            dest->value = NULL;
        } else {
            size = getSize(src);
            dest->value = malloc(size);
            memcpy(dest->value, src->value, size);
        }
        dest->type = src->type;
    }
    */
    return newStack;
}

/**
 * @brief Aloca o espaço e cria um bloco com o mesmo conteúdo do bloco de imput
 * @param block bloco a copiar
 * @returns novo bloco
*/
BLOCKPTR copyBlock (BLOCKPTR block) {
    BLOCKPTR new = malloc(sizeof(blockStruct));
    new->handle = block->handle;
    new->dist = block->dist;
    int len = strlen(block->string);
    new->string = malloc(sizeof(char)*(len + 2));
    //strdup?? fiz assim já que tive de ir buscar o strlen
    memcpy(new->string, block->string, len + 2);
    return new;
}

/**
 * @brief Copia o NODE src para dest. Assume-se que dest->value tinha informação e faz free**
 * @param dest NODE de destino
 * @param src NODE de origem
 * @returns void
*/
void copyNode (NODE dest, NODE src) {
    TYPE t = src->type;
    deleteNode(dest);
    if (t == BLOCK) {
        dest->value = copyBlock(src->value);
    } else if (t == ARRAY) {
        dest->value = copyArray(src->value);
    } else {
        int size = getSize(src);
        dest->value = malloc(size);
        memcpy(dest->value, src->value, size);
    }
    dest->type = t;
}

//copia elemento src para o topo da stack, incrementando sp
//assume que esse node estava vazio
/**
 * @brief Copia source para o topo da stack, fazendo push
 * @param stack para a qual fazer push
 * @param src NODE para copiar
 * @returns void
*/
void copyNodePush (STACK stack, NODE src) {
    nodeStruct node = *src; //ao fazer realloc, src pode mudar de sitio, por isso vamos já buscar o valor
    if (stack->sp == stack->size-1) {
        stack->size *= 2;
        stack->array = realloc(stack->array, stack->size*sizeof(nodeStruct));
    }
    NODE newNode = stack->array + (stack->sp + 1);
    if (node.type == ARRAY) {
        newNode->value = copyArray(node.value);
    } else if (node.type == BLOCK) {
        newNode->value = copyBlock(node.value);
    } else {
        int size = getSize(&node);
        newNode->value = malloc(size);
        memcpy(newNode->value, node.value, size);
        newNode->type = node.type;
    }
    stack->sp ++;
    newNode->type = node.type;
}

/**
 * @brief Transforma node para o tipo int. Altera o seu tipo e trunca o resultado, se necessário.
 * @param node NODE a transformar
 * @returns void
*/
void nodeToInt (NODE node) { //torna node em int (melhor que ir ao topo da stack outra vez se ja o tivermos?)
    if (node->type == STRING) { // ver depois
        char *str = node->value;
        node->value = malloc(sizeof(long double));
        *(long double *)node->value = (long double)atol((char *)str);
        free(str);
    } else {
        long int A = (long int)(*(long double *)node->value); // truncar parte fracionaria
        *(long double *)node->value = (long double)A;
    }
    node->type = INT;
}

/**
 * @brief Cria uma nova stack vazia
 * @param nada
 * @returns STACK nova criada
*/
STACK newStack (NODE varList) {
    STACK stack = malloc(sizeof(stackStruct));
    stack->sp = -1; // para começar no 0 no primeiro push
    stack->size = 32;
    stack->array = malloc(32*sizeof(nodeStruct));
    stack->varList = varList;
    return stack;
}

/**
 * Print de valores inteiros
 * @param value void* que contém o valor
 * @returns void
*/
void printInt (void *value) {
    printf("%ld", (long int)(*(long double *)value));
}

/**
 * @brief Print de valores long double
 * @param value void* que contém o valor
 * @returns void
*/
void printDouble (void *value) {
    printf("%Lg", *(long double *)value);
}

/**
 * @brief Print de string
 * @param value void* que contém a string
 * @returns void
*/
void printStr (void *value) {
    printf("%s", (char *)value);
}

/**
 * @brief Print de block, acrescenta parêntesis fora da string guardada
 * @param value void* que contém a string do bloco (sem as chavetas)
 * @returns void
*/
void printBlock (void *value) {
    printf("{ %s }", (char *) ((BLOCKPTR) value)->string); // a chaveta tem que estar colada porque já tem um espaço a mais no final a string recebida
}

/**
 * @brief Print de char
 * @param value void* que o contém
 * @returns void
*/
void printChr (void *value) {
    printf("%c", (char)*((long double *)value));
}

/**
 * @brief Usada pela debugPrintStack para tornar debug de arrays mais legível
 * @param depth indica o número de espaços consoante a "profundidade" do array
 * @returns void
*/
void printSpace (int depth) {
    for (; depth; depth--) printf("  ");
}

/**
 * @brief Faz print da lista de variáveis, indicando as letras das variáveis, o tamanho, o tipo e a sua profundidade na estrutura onde se encontram.
 * @param stack para dar print
 * @param printDist dispatch table de funções de print
 * @param depth para tornar legíveis nested arrays
 * @returns void
*/
void debugPrintVarList(STACK stack, printFunc *printDist[], int depth) {
    printSpace(depth);
    printf("lista de variáveis:\n"); //isto nao funciona se lá estiverem arrays, falta um if
    NODE node;
    int i;
    for (i = 0; i < 26; i++) {
        node = (stack->varList + i);
        if (node->type) {
            printSpace(depth);
            printf("Letra: %c\n", i + 65);
            printSpace(depth);
            printf("Tipo : %d\n", node->type);
            printSpace(depth);
            printf("Size:  %d\n", getSize(node));
            printSpace(depth);
            printf("Print:'");
            if (node->type == ARRAY) {
                STACK array = node->value;
                printSpace(depth);
                printf("depth-index:[%d]-[%d]\n", depth, i);
                printSpace(depth);
                printf("[--------------[%d]-[%d]\n", depth, i);
                debugPrintStack(array, printDist, depth + 1);
                printSpace(depth);
                printf("[--------------[%d]-[%d]\n", depth, i);
            } else {
                printDist[node->type](node->value);
            }
            printf("'\n\n");
        }
    } 
}

/**
 * @brief Faz print da stack, indicando tipos, valores, inicio e fim de arrays, varList, etc.
 * @param stack para dar print
 * @param printDist dispatch table de funções de print
 * @param depth para tornar legíveis nested arrays
 * @returns void
*/
void debugPrintStack (STACK stack, printFunc *printDist[], int depth) {
    NODE node;
    int i;
    for (i = 0; i <= stack->sp; i++) {
        node = stack->array + i;
        if (node->type == ARRAY) {
            STACK array = node->value;
            printSpace(depth);
            printf("depth-index:[%d]-[%d]\n", depth, i);
            printSpace(depth);
            printf("[--------------[%d]-[%d]\n", depth, i);
            debugPrintStack(array, printDist, depth + 1);
            printSpace(depth);
            printf("[--------------[%d]-[%d]\n", depth, i);
        } else {
            printSpace(depth);
            printf("depth-index:[%d]-[%d]\n", depth, i);
            printSpace(depth);
            printf("Tipo: %d\n", node->type);
            printSpace(depth);
            printf("Size: %d\n", getSize(node));
            printSpace(depth);
            printf("Print:'");
            if (node->type == ARRAY) {
                STACK array = node->value;
                printSpace(depth);
                printf("[--------------[%d]-[%d]\n", depth, i);
                debugPrintStack(array, printDist, depth + 1);
                printSpace(depth);
                printf("[--------------[%d]-[%d]\n", depth, i);
            } else {
                printDist[node->type](node->value);
            }
            printf("'\n\n");
        }
    }
    debugPrintVarList(stack, printDist, depth);
}

/**
 * @brief Semelhante a debugPrintStack(), mas não indica a varList
 * @param stack para dar print
 * @param printDist dispatch table de funções de print
 * @param depth para tornar legíveis nested arrays
 * @returns void
*/
void simpleDebugPrint (STACK stack, printFunc *printDist[], int depth) {
    NODE node;
    int i;
    for (i = 0; i <= stack->sp; i++) {
        node = stack->array + i;
        if (node->type == ARRAY) {
            STACK array = node->value;
            printSpace(depth);
            printf("[--------------[%d]-[%d]\n", depth, i);
            simpleDebugPrint(array, printDist, depth + 1);
            printSpace(depth);
            printf("[--------------[%d]-[%d]\n", depth, i);
        } else {
            printSpace(depth);
            printf("depth-index:[%d]-[%d]\n", depth, i);
            printSpace(depth);
            printf("Tipo: %d\n", node->type);
            printSpace(depth);
            printf("Size: %d\n", getSize(node));
            printSpace(depth);
            printf("Print:'");
            if (node->type == ARRAY) {
                STACK array = node->value;
                printSpace(depth);
                printf("[--------------[%d]-[%d]\n", depth, i);
                simpleDebugPrint(array, printDist, depth + 1);
                printSpace(depth);
                printf("[--------------[%d]-[%d]\n", depth, i);
            } else {
                printDist[node->type](node->value);
            }
            printf("'\n\n");
        }
    }
}

/**
 * @brief Faz debug da stack, chamando simpleDebugPrint()
 * @param stack para a qual fazer debug
 * @returns void
*/
void simpleDebug (STACK stack) {
    printFunc *printDist[33] = {printDouble, printChr, printInt, NULL, printDouble}; //printDist[0]: default é ser long double
    printDist[16] = printStr;
    printDist[32] = printBlock;
    simpleDebugPrint(stack, printDist, 0);
    printf("--------------------------- debug end ---------------------------\n");
}

/**
 * @brief Faz debug da stack, chamando debugPrintStack()
 * @param stack para a qual fazer debug
 * @returns void
*/
void debug (STACK stack) {
    printFunc *printDist[33] = {printDouble, printChr, printInt, NULL, printDouble}; //printDist[0]: default é ser long double
    printDist[16] = printStr;
    printDist[32] = printBlock;
    debugPrintStack(stack, printDist, 0);
    printf("--------------------------- debug end ---------------------------\n");
}

/**
 * @brief Faz push para a stack de um long double. Incrementa stack pointer
 * @param stack para a qual dar push
 * @param val long double a ser usada
 * @param t TYPE para o qual dar assign do node novo
 * @returns void
*/
void push (STACK stack, long double val, TYPE t) {
    if (stack->sp == stack->size-1) {
        stack->size *= 2;
        int size = stack->size;
        stack->array = realloc(stack->array, size*sizeof(nodeStruct));
    }
    NODE node = (stack->array) + (++(stack->sp)); // == &stack->array[...]
    node->type = t;
    node->value = malloc(sizeof(long double));
    *(long double *)node->value = val;
}

// talvez eliminar push no futuro e juntar caso para long doubles aqui?
/**
 * @brief Semelhante a push(), mas o valor é um void* e não long double
 * @param stack para a qual dar push
 * @param ptr void* que será o valor usado (é usado o pointer, não o seu valor)
 * @param t TYPE para o qual dar assign do node novo
 * @returns void
*/
void generalPush (STACK stack, void * ptr, TYPE t) {
    if (stack->sp == stack->size-1) {
        stack->size *= 2;
        int size = stack->size;
        stack->array = realloc(stack->array, size*sizeof(nodeStruct));
    }
    NODE node = (stack->array) + (++(stack->sp)); // == &stack->array[...]
    node->type = t;
    node->value = ptr;
}

/**
 * @brief recebe uma string e copia n vezes o seu conteúdo, concatenando a string resultante.
 * @param n multiplicador
 * @param s string a multiplicar
 * @returns string resultante concatenada
*/
char *str_rep(int n, char *s)
{
    size_t slen = strlen(s);
    char *dest = malloc(n * slen + 1);

    int i;
    char *p;
    for (i = 0, p = dest; i < n; ++i, p += slen)
    {
        memcpy(p, s, slen);
    }
    free(s);
    *p = '\0';
    return dest;
}

/**
 * @brief opera o valor lógico para números, strings e arrays
 * @param node sobre o qual operar
 * @returns ret - valor lógico
*/
int getLogicalValue (NODE node) {
    int ret;
    if (node->type == ARRAY) {
        ret = ((STACK) node->value)->sp + 1; //começa em -1
    } else if (node->type == STRING) {
        ret = strlen(node->value);
    } else {
        ret = (int)*(long double *)node->value;
    }
    return ret;
}

/**
 * @brief aplica a operação filter em arrays da stack
 * @param stack a utilizar
 * @param topNode útlimo node da stack
 * @param sndNode penúltimo node da stack
 * @returns ret - valor lógico
*/
void filterArray (STACK stack, NODE topNode, NODE sndNode) {
    int i, spArr, spAfter, spBefore;
    NODE node, base;
    BLOCKPTR block = topNode->value;
    STACK arr = sndNode->value,
    newArr = newStack(stack->varList);
    spArr = arr->sp;
    base = arr->array;
    for (i = 0; i <= spArr; i++) {
        node = base + i;
        copyNodePush(newArr, node);
        spBefore = newArr->sp;
        mainHandler(block->string, block->dist, block->handle, newArr);
        spAfter = newArr->sp;
        if (getLogicalValue(newArr->array + newArr->sp)) {
            for (; spAfter >= spBefore; spAfter--) {
                pop(newArr);
            }
            generalPush(newArr, node->value, node->type);
        } else {
            deleteNode(node);
            pop(newArr);
        }
    }
    pop(stack);
    deleteStack(arr);
    sndNode->type = 0;
    stack->sp --;
    generalPush(stack, newArr, ARRAY);
}

/**
 * @brief aplica a operação filter para strings na stack
 * @param stack a utilizar
 * @param topNode útlimo node da stack
 * @param sndNode penúltimo node da stack
 * @returns ret - valor lógico
*/
void filterString (STACK stack, NODE topNode, NODE sndNode) {
    int i, spArr, spAfter, spBefore;
    NODE node;
    BLOCKPTR block = topNode->value;
    char *str = (char *)sndNode->value;
    STACK newArr = newStack(stack->varList);
    for (i = 0; str[i]; i++) {
        push(newArr, (long double)str[i], CHAR);
        spBefore = newArr->sp;
        mainHandler(block->string, block->dist, block->handle, newArr);
        spAfter = newArr->sp;
        if (getLogicalValue(newArr->array + newArr->sp)) {
            for (; spAfter >= spBefore; spAfter--) {
                pop(newArr);
            }
            push(newArr, (long double)str[i], CHAR);
        } else {
            pop(newArr);
        }
    }
    pop(stack);
    pop(stack);
    spArr = newArr->sp;
    node = newArr->array;
    str = malloc(sizeof(char)*(spArr + 2));
    for (i = 0; i <= spArr; i++) {
        str[i] = (char)*(long double *)(node + i)->value;
        deleteNode(node + i);
    }
    str[i] = '\0';
    deleteStack(newArr);
    generalPush(stack, str, STRING);
}

/**
 * @brief utiliza as funções filterArray e filterString para filtrar todos os tipos da stack
 * @param stack a utilizar
 * @param topNode útlimo node da stack
 * @param sndNode penúltimo node da stack
 * @returns ret - valor lógico
*/
void filter (STACK stack, NODE topNode, NODE sndNode) {
    if (sndNode->type == ARRAY)
        filterArray (stack,topNode,sndNode);
    else // string
        filterString (stack,topNode,sndNode);
}

/*
void filter (STACK stack, NODE topNode, NODE sndNode) {
    int i, spArr, spAfter, spBefore;
    NODE node, base;
    BLOCKPTR block = topNode->value;
    if (sndNode->type == ARRAY) {
        STACK arr = sndNode->value,
        newArr = newStack(stack->varList);
        spArr = arr->sp;
        base = arr->array;
        for (i = 0; i <= spArr; i++) {
            node = base + i;
            copyNodePush(newArr, node);
            spBefore = newArr->sp;
            mainHandler(block->string, block->dist, block->handle, newArr);
            spAfter = newArr->sp;
            if (getLogicalValue(newArr->array + newArr->sp)) {
                for (; spAfter >= spBefore; spAfter--) {
                    pop(newArr);
                }
                generalPush(newArr, node->value, node->type);
            } else {
                deleteNode(node);
                pop(newArr);
            }
        }
        pop(stack);
        deleteStack(arr);
        sndNode->type = 0;
        stack->sp --;
        generalPush(stack, newArr, ARRAY);
    } else { //string
        char *str = (char *)sndNode->value;
        STACK newArr = newStack(stack->varList);
        for (i = 0; str[i]; i++) {
            push(newArr, (long double)str[i], CHAR);
            spBefore = newArr->sp;
            mainHandler(block->string, block->dist, block->handle, newArr);
            spAfter = newArr->sp;
            if (getLogicalValue(newArr->array + newArr->sp)) {
                for (; spAfter >= spBefore; spAfter--) {
                    pop(newArr);
                }
                push(newArr, (long double)str[i], CHAR);
            } else {
                pop(newArr);
            }
        }
        pop(stack);
        pop(stack);
        spArr = newArr->sp;
        node = newArr->array;
        str = malloc(sizeof(char)*(spArr + 2));
        for (i = 0; i <= spArr; i++) {
            str[i] = (char)*(long double *)(node + i)->value;
            deleteNode(node + i);
        }
        str[i] = '\0';
        deleteStack(newArr);
        generalPush(stack, str, STRING);
    }
}*/

// -------- guiao 1 ----------

/**
 * @brief Faz pop do elemento no topo da stack. Decrementa sp e chama deleteNode()
 * @param stack para a qual dar pop
 * @returns void
*/
void pop (STACK stack) {
    NODE node = (stack->array) + ((stack->sp)--);
    deleteNode(node);
}

/**
 * @brief concatenaçao string/char no topo da stack
 * @param stack a utilizar
 * @param topNode último valor colocado na stack
 * @param sndNode penúltimo valor colocado na stack
 * @returns void
*/
void addStr (STACK stack, NODE topNode, NODE sndNode) {
    if (sndNode->type == STRING && topNode->type == STRING) {
        int length = strlen (topNode->value) + strlen (sndNode->value) + 1;
        sndNode->value = realloc (sndNode->value,length);
        sndNode->value = strcat (sndNode->value,topNode->value);
        pop (stack);
    } else if (sndNode->type == STRING) { // && topNode->type == CHAR) {
        int len = strlen(sndNode->value);
        sndNode->value = realloc(sndNode->value, sizeof(char)*(len + 2));
        ((char *)sndNode->value)[len] = (char)*(long double *)topNode->value;
        ((char *)sndNode->value)[len + 1] = '\0';
        pop(stack);
    } else {
        int len = strlen(topNode->value);
        char *str = malloc(sizeof(char)*(len + 2));
        memcpy(str+1, topNode->value, len + 1);
        str[0] = (char)*(long double *)sndNode->value;
        pop(stack);
        pop(stack);
        generalPush(stack, str, STRING);
    }
}

/**
 * @brief concatenaçao array/número no topo da stack
 * @param stack a utilizar
 * @param topNode último valor colocado na stack
 * @param sndNode penúltimo valor colocado na stack
 * @returns void
*/
void addArr (STACK stack, NODE topNode, NODE sndNode) {
    int i;
    NODE node;
    if (sndNode->type == ARRAY && topNode->type == ARRAY) {
        STACK fstStack = topNode->value,
        sndStack = sndNode->value;
        for (i = 0; i <= fstStack->sp; i++) { // otimizar acessos a memoria??
            node = fstStack->array + i;
            generalPush(sndStack, node->value, node->type);
        }
        // fazer pop iria chamar a fullDeleteStack, criando memory leak. deleteStack não faz free a cada node->value
        deleteStack(fstStack);
    } else if (sndNode->type == ARRAY) {
        STACK sndStack = sndNode->value;
        generalPush(sndStack, topNode->value, topNode->type);
    } else { // enorme confusão de nomes de vars // elemento [ array ] +
        STACK topStack = topNode->value;
        if (topStack->sp == -1) { //sarray vazio
            generalPush(topStack, sndNode->value, sndNode->type);
            sndNode->type = ARRAY;
            sndNode->value = topStack;
        } else { //isto funcemina quando array fica sem espaço???
            NODE tnode = topStack->array + topStack->sp;
            generalPush(topStack, tnode->value, tnode->type);
            NODE snode = tnode - 1;
            for (i = topStack->sp - 1; i > 0; i --) {
                tnode->value = snode->value;
                tnode->type = snode->type;
                tnode -= 1;
                snode -= 1;
            }
            tnode->value = sndNode->value;
            tnode->type = sndNode->type;
            sndNode->value = topNode->value;
            sndNode->type = topNode->type;
        }
    }
    topNode->type = 0;
    topNode->value = NULL;
    stack->sp --;
}

/**
 * @brief Soma os dois valores no topo da stack
 * @param stack a utilizar
 * @returns void
*/
void add (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    if (sndNode->type == ARRAY) {
        STACK sndStack = sndNode->value;
        if (topNode->type == ARRAY) {
            int i;
            STACK fstStack = topNode->value;
            NODE node;
            for (i = 0; i <= fstStack->sp; i++) {
                node = fstStack->array + i;
                generalPush(sndStack, node->value, node->type);
            }
            // fazer pop iria chamar a fullDeleteStack, criando memory leak. deleteStack não faz free a cada node->value
            deleteStack(fstStack);           
        } else {
            generalPush(sndStack, topNode->value, topNode->type);
            topNode->value = NULL;
        }
        topNode->type = 0;
        stack->sp --;
    } else if (topNode->type == ARRAY || sndNode->type == ARRAY) {
        addArr(stack, topNode, sndNode);
    } else if (topNode->type == STRING || sndNode->type == STRING) {
        addStr(stack, topNode, sndNode);
    } else {
        *(long double *)sndNode->value += *(long double *)topNode->value;
        if (!(HAS_DOUBLE(topNode->type, sndNode->type))) {
            nodeToInt(sndNode);
        } else {
            sndNode->type = DOUBLE;
        }
        pop(stack);
    }
}

/**
 * @brief Função que subtrai os dois valores no topo da stack
 * @param stack a utilizar
 * @returns void
*/
void sub (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    *(long double *)sndNode->value -= *(long double *)topNode->value;
    if (!(HAS_DOUBLE(topNode->type, sndNode->type))) {
        nodeToInt(sndNode);
    } else {
        sndNode->type = DOUBLE;
    }
    pop(stack);
}

/**
 * @brief divide string por substrings
 * @param stack a utilizar
 * @param topNode último valor da stack
 * @param sndNode penútlimo valor da stack
 * @returns void
*/
STACK getSubstrings (STACK stack, NODE topNode, NODE sndNode) {
    if (topNode->type == CHAR) {
        char chr = (char)*(long double *)topNode->value;
        free(topNode->value);
        topNode->value = malloc(sizeof(char)*2);
        *(char *)(topNode->value) = chr;
        *((char *)(topNode->value) + 1) = '\0';
    }
    char *str = sndNode->value,
    *topstr = topNode->value, 
    *r,
    *temp;
    int length = strlen(topstr),
    tam;
    STACK newstack = newStack(stack->varList);
    if (length) { //se for 0 temos de separar string para array de chars
        for (tam = 0; (r = strstr(str, topstr)) ; str += tam + length) {
            tam = r - str;
            if (tam) {
                temp = malloc(sizeof(char)*(tam + 1));
                memcpy(temp, str, tam);
                *(temp + tam) = '\0';
                generalPush(newstack, temp, STRING);
            }
        }
        if (*str) {
            tam = strlen(str);
            if (tam) {
                temp = malloc(sizeof(char)*(tam + 1));
                memcpy(temp, str, tam);
                *(temp + tam) = '\0';
                generalPush(newstack, temp, STRING);
            }
        }
    } else {
        for (; *str; str++) {
            push(newstack, (long double)*(str), CHAR);
        }
    }
    return newstack;

}

/**
 * @brief Função que divide os dois valores dno topo da stack
 * @param stack a utilizar
 * @returns void
*/
void divis (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    if (sndNode->type == STRING) { //separar string por substrings
        STACK newstack = getSubstrings(stack, topNode, sndNode);
        pop(stack);
        pop(stack);
        generalPush(stack, newstack, ARRAY);
    } else {
        *(long double *)sndNode->value /= *(long double *)topNode->value;
        if (!(HAS_DOUBLE(topNode->type, sndNode->type))) {
            nodeToInt(sndNode);
        } else {
            sndNode->type = DOUBLE;
        }
    pop(stack);
    }   
}


/**
 * @brief fold sobre um array usando um bloco 
 * @param stack a utilizar 
 * @param topNode último valor colocado na stack
 * @param sndNode penúltimo valor colocado na stack
 * @returns void
 */
void foldArray(STACK stack, NODE topNode, NODE sndNode) {
    BLOCKPTR block = topNode->value;
    STACK arr = sndNode->value,
    newArr = newStack(stack->varList);
    NODE node = arr->array;
    int i, sp = arr->sp;
    generalPush(newArr, node->value, node->type);
    for (i = 1; i <= sp; i++) {
        generalPush(newArr, (node + i)->value, (node + i)->type);
        mainHandler(block->string, block->dist, block->handle, newArr);
    }
    deleteStack(arr);
    pop(stack);
    stack->sp --;
    generalPush(stack, newArr, ARRAY);
}

/**
 * @brief Função que multiplica os dois valores no topo da stack. Caso se trate de uma string ou de um array, a função devolve o resultado da concatenação da string n vezes, sendo n o útlimo valor colocado na stack.
 * @param stack a utilizar
 * @returns void
*/
void mult (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    if (topNode->type == BLOCK) {
        foldArray(stack, topNode, sndNode);
    }
    else if (IS_NUM(sndNode)) {
        *(long double *)sndNode->value *= *(long double *)topNode->value;
        if (!(HAS_DOUBLE(topNode->type, sndNode->type))) {
            nodeToInt(sndNode);
        } else {
            sndNode->type = DOUBLE;
        }
        pop(stack);
    }
    else if (sndNode->type == STRING) { 
        int multi = (long int)*(long double *)topNode->value - 1;
        sndNode->value = str_rep(multi+1,sndNode->value); //dar free depois do alloc da str_rep
        pop(stack);
    }
    
    else if (sndNode->type == ARRAY) {
        STACK arr = sndNode->value;
        int rep = (long int)*(long double *)topNode->value - 1,
        sp = arr->sp, i;
        for (; rep > 0; rep --) {
            for (i = 0; i <= sp; i++) {
                copyNodePush(arr, arr->array + i);
            }
        }
        pop(stack); //remove multiplicador da stack
    }
}

/** 
* @brief faz map dos elementos do array em sndNode aplicado o bloco em topNode
 @param stack a utilizar
 @param topNode o valor no topo da stack
 @param sndNode o penúltimo valor da stack
 @return void

*/void mapArray (STACK stack, NODE topNode, NODE sndNode) {
    BLOCKPTR block = topNode->value;
    STACK newArr = newStack(stack->varList);
    int i, sp;
    NODE node;
    char *str = block->string;
    if (sndNode->type == ARRAY) { //array
        STACK arr = sndNode->value;
        sp = arr->sp;
        node = arr->array;
        for (i = 0; i <= sp; i ++) {
            copyNodePush(newArr, node + i);
            mainHandler(str, block->dist, block->handle, newArr);
        }
        pop(stack);
        pop(stack);
        generalPush(stack, newArr, ARRAY);
    } else { // string
        char *sndStr = sndNode->value;
        for (i = 0; sndStr[i]; i++) {
            push(newArr, (long double)sndStr[i], CHAR);
            mainHandler(str, block->dist, block->handle, newArr);
        }
        pop(stack);
        pop(stack);
        sp = newArr->sp;
        char *str = malloc(sizeof(char)*(sp + 2));
        node = newArr->array;
        for (i = 0; i <= sp; i++) {
            str[i] = (char)*(long double *)(node + i)->value;
            deleteNode(node + i);
        }
        str[i] = '\0';
        deleteStack(newArr);
        generalPush(stack, str, STRING);
    }
}

/**
 * @brief Devolve o resto da divisão dos 2 valores no topo da stack, ou executa um map sobre um array usando o bloco
 * @param stack onde executar operação
 * @returns void
*/
void mod (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    if (topNode->type == BLOCK) {
        mapArray(stack,topNode,sndNode);
    } else {
        *(long double *)sndNode->value = fmod(*(long double *)sndNode->value, *(long double *)topNode->value);
        if (!(HAS_DOUBLE(topNode->type, sndNode->type))) {
            nodeToInt(sndNode);
        } else {
            sndNode->type = DOUBLE;
        }
        pop(stack);
    }
}

/**
 * @brief Devolve o resultado da exponenciação do 2º valor da stack pelo 1º (a contar do topo) 
 * @param stack onde executar operação
 * @returns void
*/
void expo (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    if (sndNode->type == STRING) {
        int pos=-1;
        if (topNode->type == STRING) {
            char *str = sndNode->value, *r;
            r = strstr(str,topNode->value);
            if (r!= NULL) {   
                pos = (r-str);
            }
        } else { // string char #
            char chr = (char)*(long double*)topNode->value,
            *str = (char *)sndNode->value;
            int i;
            for (i = 0; str[i] && str[i] != chr; i++);
            if (str[i]) pos = i;
        }
        pop(stack);
        deleteNode(sndNode);
        sndNode->value = malloc(sizeof(long double));  
        sndNode->type = INT;
        *(long double*) (sndNode->value) = (long double)pos;
    } else {
        *(long double *)sndNode->value = pow(*(long double *)(sndNode->value), *(long double *)topNode->value);
        if (!(HAS_DOUBLE(topNode->type, sndNode->type))) {
            nodeToInt(sndNode);
        } else {
            sndNode->type = DOUBLE;
        }
    pop(stack);
    }
}

/**
 * @brief Incrementa número no topo da stack/remove último elemento de array ou string e coloca após o mesmo
 * @param stack onde executar operação
 * @returns void
*/
void inc (STACK stack) {
    NODE node = stack->array + stack->sp; 
    if (node->type == STRING) {
        int i = strlen(node->value) - 1;
        char c = ((char *)node->value)[i];
        ((char *)node->value)[i] = '\0';
        push(stack, (long double)c, CHAR);
    } else if (node->type == ARRAY) {
        STACK arr = node->value;
        NODE arrTopNode = arr->array + arr->sp;
        // mais eficiente que copiar o node? nao posso fazer pops porque isso implica free()
        arr->sp --;
        generalPush(stack, arrTopNode->value, arrTopNode->type);
        arrTopNode->value = NULL;
        arrTopNode->type = 0;
        /*
        copyNodePush(stack, arrTopNode);
        pop(arr);
        */
    } else {
        (*(long double *)(stack->array + stack->sp)->value) ++;
    }
}

/**
 * @brief Decrementa número no topo da stack/remove primeiro elemento de array ou string e coloca após o mesmo
 * @param stack onde executar operação
 * @returns void
*/
void dec (STACK stack) {
    NODE node = stack->array + stack->sp;
    if (node->type == STRING) {
        char *str =(char *)node->value,
        c = *str;
        int i;
        for (i = 0; str[i+1]; i++) {
            str[i] = str[i + 1];
        }
        str[i] = '\0';
        push(stack, (long double)c, CHAR);
    } else if (node->type == ARRAY) {
        STACK arr = node->value; int i;
        NODE arrBotNode = arr->array, 
        sndNode = arrBotNode + 1;
        //igual acima
        generalPush(stack, arrBotNode->value, arrBotNode->type);
        for (i = 0; i < arr->sp; i ++) {
            arrBotNode->type = sndNode->type;
            arrBotNode->value = sndNode->value;
            arrBotNode ++; sndNode ++;
        }
        //nao posso fazer pop porque vai dar free do top, e o que esta antes desse passa a apontar para memoria que levou free
        *sndNode = (nodeStruct) {NULL, 0};
        arr->sp --;
    } else {
        (*(long double *)(stack->array + stack->sp)->value) --;
    }
}

/**
 * @brief Bitwise and entre 2 valores no topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void and (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    *(long double *)sndNode->value = (long double) ((long int)*(long double *)sndNode->value & (long int)*(long double *)topNode->value);
    pop(stack);
}

/**
 * @brief Bitwise or entre 2 valores no topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void or (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    *(long double *)sndNode->value = (long double) ((long int)*(long double *)sndNode->value | (long int)*(long double *)topNode->value);
    pop(stack);
}

/**
 * @brief Bitwise xor entre 2 valores no topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void xor (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp),
    sndNode = topNode - 1;
    *(long double *)sndNode->value = (long double) ((long int)*(long double *)sndNode->value ^ (long int)*(long double *)topNode->value);
    pop(stack);
}

/**
 * @brief Bitwise not ao topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void not (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    if (topNode->type == BLOCK) { // se for a função para o caso dos blocos
        BLOCKPTR block = topNode->value;
        stack->sp--;
        topNode->value = NULL;
        topNode->type = 0;
        char * str = block->string; 
        mainHandler(str, block->dist, block->handle, stack);
        free(block->string);
        free(block);
    } else if (IS_NUM(topNode)) { // caso para numeros
        *(long double *)topNode->value = (long double)(~(long int)*(long double *)topNode->value);
    } else { // caso para arrays
        STACK arr = topNode->value;
        NODE arrBotNode = arr->array,
        currentNode;
        int i;
        stack->sp --;
        for (i = 0; i <= arr->sp; i++) {
            currentNode = arrBotNode + i;
            generalPush(stack, currentNode->value, currentNode->type);
        }
        deleteStack(arr);
    }
}

// -------- guiao 2 ----------

/**
 * @brief Passa topo da stack para char
 * @param stack onde executar operação
 * @returns void
*/
void toChar (STACK stack) {
    NODE node = stack->array + stack->sp;
    if (node->type == STRING) {
        char *str = node->value, chr = str[0];
        free(str);
        node->value = malloc(sizeof(long double));
        *(long double *)node->value = (long double)chr;
    } else if (IS_NUM(node)) {
        *(long double *)node->value = (long double)((char)*(long double *)node->value); //truncar resultado
    }
    node->type = CHAR;
}

/**
 * @brief Passa topo da stack para long double
 * @param stack onde executar operação
 * @returns void
*/
void toDouble (STACK stack) {
    NODE topNode = (stack->array + stack->sp);
    if (topNode->type == STRING) {
        long double A = strtold((char *)topNode->value, NULL);
        free(topNode->value);
        topNode->value = malloc(sizeof(long double));
        *(long double *)topNode->value = A;
    }
    topNode->type = DOUBLE;
}

/**
 * @brief Passa topo da stack para int, chamando nodeToInt() (para truncar resultado)
 * @param stack onde executar operação
 * @returns void
*/
void toInt (STACK stack) {
    nodeToInt(stack->array + stack->sp);
}

/**
 * @brief Troca os dois elementos do topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void swap_top2 (STACK stack) {
    NODE topNode = stack->array + stack->sp,
    sndNode = topNode - 1;
    nodeStruct temp = *topNode;
    topNode->value = sndNode->value;
    topNode->type = sndNode->type;
    sndNode->value = temp.value;
    sndNode->type = temp.type;
}

/**
 * @brief Duplica o elemento do topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void duplicate (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    copyNodePush(stack, topNode);
}

/**
 * @brief Roda os três elementos do topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void rotate_top3 (STACK stack) {
    NODE topNode = stack->array + stack->sp,
    sndNode = topNode - 1,
    thrdNode = topNode - 2;
    nodeStruct tempTop = *topNode;

    topNode->value = thrdNode->value;
    topNode->type = thrdNode->type;
    thrdNode->value = sndNode->value;
    thrdNode->type = sndNode->type;
    sndNode->value = tempTop.value;
    sndNode->type = tempTop.type;
}

/**
 * @brief insere um elemento numa lista tempOrganizer 
 * @param x long double a inserir 
 * @param arr lista com os valores e posições correspondentes a aplicar o bloco
 * @param N tamanho da lista
 * @returns void
*/
void insere (long double x, tempOrganizer arr, int N) {
    int i;
    if (N > 0) {
        for (i = N-1; i >= 0 && *(int *)arr[i].value > x; i--) {
            *(int *)arr[i+1].value = *(int *)arr[i].value;
            arr[i+1].pos = arr[i].pos;
        }
        *(int *)arr[i+1].value = x;
        arr[i+1].pos = N;
    } else {
        *(int *)arr[0].value = x;
        arr[0].pos = N;
    }
}

/**
 * @brief insere uma string numa lista tempOrganizer 
 * @param x string a inserir 
 * @param arr lista com os valores e posições correspondentes a aplicar o bloco
 * @param N tamanho da lista
 * @returns void
*/
void insereString (char *x, tempOrganizer arr, int N) {
    int i;
    if (N > 0) {
        for (i = N-1; i >= 0 && strcmp((char *)arr[i].value, x) > 0; i--) {
            arr[i+1].value = arr[i].value;
            arr[i+1].pos = arr[i].pos;
        }
        arr[i+1].value = x;
        arr[i+1].pos = N;
    } else {
        arr[0].value = x;
        arr[0].pos = N;
    }
}

/**
* @brief cria uma lista tempOrganizer ordenada
* @param array com os valores resultantes de aplicar o bloco
* @return void
*/
tempOrganizer makeList (STACK newArray) {
    NODE node = newArray->array;
    int i, sp = newArray->sp;
    tempOrganizer list = malloc(sizeof(organizerStruct)*(sp + 1));
    if (node->type == STRING) {
        char *str;
        for (i = 0; i <= sp; i++) {
            str = (node +i)->value;
            //deleteNode(node + i);
            insereString(str, list, i);
        }
    } else {
        int val;
        for (i = 0; i <= sp; i++) {
            list[i].value = malloc(sizeof(int));
        }
        for (i = 0; i <= sp; i++) {
            val = getLogicalValue(node + i);
            deleteNode(node + i);
            insere(val, list, i);
        }
    }
    return list;
}

/**
 * @brief executa o bloco para criar um array com os valores que sao usados para ordenação
 * @param newArray array com os valores já processados pelo bloco
 * @param arr array original
 * @param block a ser utilzado para execução
 * @return lista tempOrganizer já ordenada
*/tempOrganizer getArrayValues (STACK newArray, STACK arr, BLOCKPTR block) {
    int i, spBefore, spAfter, sp = arr->sp;
    NODE node = arr->array, temp;
    for (i = 0; i <= sp; i++) {
        spBefore = newArray->sp;
        copyNodePush(newArray, node + i);
        mainHandler(block->string, block->dist, block->handle, newArray);
        spAfter = newArray->sp - 1;
        temp = newArray->array + newArray->sp;
        newArray->sp--;
        //deixar o valor resultante na posicao i
        while (spAfter > spBefore) {
            pop(newArray);
            spAfter --;
        }
        generalPush(newArray, temp->value, temp->type);
    }
    return makeList(newArray);
}

/**
 * @brief Obtém os valores das strings antes de as organizar
 * @param newArray onde são guardados
 * @param str recebida
 * @param block usado para ordenar
 * @returns lista tempOrganizer já ordenada
*/
tempOrganizer getStringValues (STACK newArray, char *str, BLOCKPTR block) {
    int i, spBefore, spAfter;
    NODE temp;
    for (i = 0; str[i]; i++) {
        spBefore = newArray->sp;
        push(newArray, (long double)str[i], CHAR);
        mainHandler(block->string, block->dist, block->handle, newArray);
        spAfter = newArray->sp - 1;
        temp = newArray->array + newArray->sp;
        newArray->sp--;
        //deixar o valor resultante na posicao i
        while (spAfter > spBefore) {
            pop(newArray);
            spAfter --;
        }
        generalPush(newArray, temp->value, temp->type);
    }
    return makeList(newArray);
}

/**
 * @brief Copia o n-ésimo elemento da stack e coloca-o no topo
 * @param stack onde executar operação
 * @returns void
*/
void copy_nth (STACK stack) {
    NODE topNode = stack->array + stack->sp,
    sndNode = topNode-1,
    node;
    int i, sp; // val long double???
    if (topNode->type == BLOCK) {
        STACK newArray = newStack(stack->varList); //array que fica com valores a serem comparados
        BLOCKPTR block = topNode->value;
        if (sndNode->type == ARRAY) { //ordenar array
            tempOrganizer list = getArrayValues(newArray, sndNode->value, block);
            node = ((STACK) sndNode->value)->array;
            sp = newArray->sp;
            newArray->sp = -1;
            for (i = 0; i <= sp; i++) {
                generalPush(newArray, (node + list[i].pos)->value, (node + list[i].pos)->type);
                free(list[i].value);
            }
            pop(stack); //tirar o bloco
            deleteStack(sndNode->value);
            stack->sp--;
            generalPush(stack, newArray, ARRAY);
            free(list);
        } else {
            tempOrganizer list = getStringValues(newArray, sndNode->value, block);
            sp = newArray->sp;
            char *str = malloc(sizeof(char)*(sp + 2)),
            *original = sndNode->value;
            for (i = 0; i <= sp; i++) {
                str[i] = original[list[i].pos];
            }
            str[i] = '\0';
            pop(stack); //tirar o bloco
            pop(stack);
            generalPush(stack, str, STRING);
            deleteStack(newArray);
            free(list);
        }
    } else {
        long int n = (long int)(*(long double*)topNode->value);
        NODE node = topNode - n - 1;
        copyNode(topNode, node); //faz free do topNode->value
    }
}

/**
 * @brief Lê uma linha de input e coloca no topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void readl (STACK stack) {
    int i, c, chr, size = 32;
    char *line = malloc(32*sizeof(char));
    for (i = c = 0; (chr = getc(stdin)) && chr != '\n'; i++, c++) {
        if (c == size - 2) {
            size *= 2;
            line = realloc(line, size*sizeof(char));
        }
        line[i] = chr;
    }
    line[i] = '\0';
    generalPush(stack, line, STRING);
}

// -------- guiao 3 ----------

/**
 * @brief Efetua operação de igualdade entre 2 valores no topo da stack
 * @param stack onde executar operação
 * @returns void
*/
void isEqual (STACK stack) {
    int I;
    NODE topNode = stack->array + stack->sp,
    sndNode = topNode - 1;
    if (sndNode->type == ARRAY) {
        int pos = (long int) *(long double *) topNode->value;
        pop(stack);
        STACK arr = sndNode->value; // pointer para a nested stack
        NODE arrBotNode = arr->array,
        currentNode;
        for (I = 0; I < pos; I++) {
            deleteNode(arrBotNode + I);
        }
        stack->sp --;
        currentNode = arrBotNode + I;
        generalPush (stack, currentNode->value, currentNode->type);
        for (++I; I <= arr->sp; I++) {
            deleteNode(arrBotNode + I);
        }
        deleteStack(arr);
    } else if (sndNode->type == STRING) {
        if (topNode->type == STRING) {
            I = !(strcmp(sndNode->value, topNode->value));
            pop(stack);
            pop(stack);
            push(stack, (long double)I, INT);
        } else {
            int pos = (long int)*(long double *)topNode->value;
            pop(stack);
            char c = *((char *)sndNode->value + pos);
            pop(stack);
            push(stack, (long double)c, CHAR);
        }
    } else {
        *(long double *)sndNode->value = *(long double *)sndNode->value == *(long double *)topNode->value;
        nodeToInt(sndNode);
        pop(stack);
    }
}

/**
 * @brief Verifica se o penúltimo elemento colocado na stack é maior do que o último. 
 * @param stack onde executar operação
 * @returns void
*/
void isGreater (STACK stack) {
    NODE topNode = stack->array + stack->sp,
    sndNode = topNode - 1;
    if (sndNode->type == ARRAY) {
        STACK arr = sndNode->value;
        NODE currentNode, tempNode;
        int i, interval = (long int)*(long double *)topNode->value, s = arr->sp - interval;
        pop(stack);
        if (s != -1) {
            for (i = 0; i <= s; i++) {
                currentNode = arr->array + i; 
                deleteNode(currentNode);
                arr->sp --; // para evitar free aos valores que saíram do array
            }
            for (i = 0; i < interval; i ++) {
                currentNode = arr->array + i;
                tempNode = currentNode + s + 1;
                currentNode->value = tempNode->value;
                currentNode->type = tempNode->type;
                tempNode->value = NULL;
                tempNode->type = 0;
            }
        }
    } else if (topNode->type == STRING) {
        int res = strcmp(sndNode->value, topNode->value);
        if (res > 0) res = 1;
        else res = 0;
        pop(stack);
        pop(stack);
        push(stack, (long double)res, INT);
    } else if (sndNode->type == STRING) {
        char * temp = sndNode->value;
        int interval = (long int)*(long double *)topNode->value,
        s = strlen(temp) - interval;
        pop(stack);
        sndNode->value = malloc (sizeof(char)*interval+1);
        memcpy(sndNode->value,temp+s,interval+1);
        free (temp);
    } else {
        *(long double *)sndNode->value = (*(long double *)sndNode->value) > (*(long double *)topNode->value);
        nodeToInt(sndNode);
        pop(stack);
    }
}

/**
 * @brief Verifica se o penúltimo elemento colocado na stack é menor do que o último. 
 * @param stack onde executar operação
 * @returns void
*/
void isLesser (STACK stack) {
    NODE topNode = stack->array + stack->sp,
    sndNode = topNode - 1;
    if (sndNode->type == ARRAY) { // é para meter num array, ou pushes separados???
        int valor = (long int) *(long double *) topNode->value,
        I;
        pop(stack);
        STACK arr = sndNode->value;
        for (I = arr->sp - valor + 1 ;  I > 0; I--) {
            pop(arr);
        }
    } else if (topNode->type == STRING) {
        int res = strcmp(sndNode->value, topNode->value);
        if (res < 0) res = 1;
        else res = 0;
        pop(stack);
        pop(stack);
        push(stack, (long double)res, INT);
    } else if (sndNode->type == STRING) {
        int valor = (long int) *(long double *) topNode->value;
        pop(stack);
        ((char *) sndNode->value)[valor] = '\0';

    } else {
        *(long double *)sndNode->value = *(long double *)sndNode->value < *(long double *)topNode->value;
        nodeToInt(sndNode);
        pop(stack);
    }
}

/**
 * @brief Negar número inteiro/ coloacar na stack todos os elementos de um array
 * @param stack a stack onde executar a operação
 * @returns void
*/
void negate (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    int A = (long int)*(long double *)topNode->value;
    *(long double *)topNode->value = (long double)(!A); //ja trunca para int, basta mudar o type
    topNode->type = INT;
}

/**
 * @brief função que verifica o valor do terceiro operador na stack (condição) e retorna o segundo (then) se for verdadeira ou o primeiro (else) se for falsa.
 * @param stack onde executar a operação
 * @returns void
*/
void IfThenElse (STACK stack) {
    NODE Else = stack->array + stack->sp,
    Then = Else - 1,
    condition = Then - 1; //vai acabar por ser o final
    if (getLogicalValue(condition)) {
        copyNode(condition,Then);
    } else {
        copyNode(condition,Else);
    }
    pop(stack);
    pop(stack);
}

/**
 * @brief compara os dois valores no topo da stack e mantêm no topo apenas o maior, alterando o array
 * @param stack a utilizar
 * @return void
*/
void top2Greater (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp);
    NODE sndNode = topNode - 1;
    if (topNode->type == STRING) {
        int length = strlen (topNode->value);
        int length1 = strlen (sndNode->value);
        if (length > length1) {
            copyNode (sndNode, topNode);            
        }
    } else {
    long double fst = *(long double *) topNode->value;
    long double snd = *(long double *)sndNode->value;
    if (fst > snd) {
        copyNode(sndNode, topNode);
    }
   }
   pop(stack);
}

/**
 * @brief compara os dois valores no topo da stack e mantêm no topo apenas o menor, alterando o array
 * @param stack a ser utilizada
 * @return não retorna nada
*/
void top2Lower (STACK stack) {
    NODE topNode = (stack->array) + (stack->sp);
    NODE sndNode = topNode - 1;
    if (topNode->type == STRING) {
        int length = strlen(topNode->value);
        int length1 = strlen(sndNode->value);
        if (length <= length1) {
            copyNode (sndNode, topNode);            
        }
    } else {
        long double fst = *(long double *) topNode->value;
        long double snd = *(long double *)sndNode->value;
        if (fst < snd) {
            copyNode(sndNode, topNode);
        }
   }
   pop(stack);
}

/**
 * @brief compara os dois valores no topo da stack, segundo a operação "or", parando se o primeiro for logo verdade (isto é, tem shortcut), manipulando a stack
 * @param stack a ser utilizada
 * @return não retorna nada
*/
void top2Or (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    NODE sndNode = topNode - 1;
    if (!getLogicalValue(sndNode)) {
        copyNode(sndNode, topNode);
    } //se  for verdadeiro, faz pop e está feito
    pop(stack);
}

/**
 * @brief compara os dois valores no topo da stack, segundo a operação "and", parando se o primeiro for logo falso (isto é, tem shortcut), manipulando a stack
 * @param stack a ser utilizada
 * @return não retorna nada
*/
void top2And (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    NODE sndNode = topNode - 1;
    if (getLogicalValue(sndNode)) {
        copyNode(sndNode, topNode);
    } // se o segundo for falso, faz pop e está feito
    pop(stack);
}

// -------- guiao 4 ----------

/**
 * @brief retorna o tamanho da string ou array colocado no topo da stack.
 * @param stack a utilizar
 * @return não retorna nada
*/
void range (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    if (topNode->type == STRING) {
        int len = strlen(topNode->value);
        pop(stack);
        push(stack, (long double)len, INT);
    } else if (IS_NUM(topNode)) {
        int I;
        long double valorNodo = *(long double *)topNode->value;
        STACK newArray = newStack(stack->varList);
        for (I = 0; I < valorNodo; I++) {
            push(newArray, (long double)I, topNode->type);
        }
        pop(stack);
        generalPush(stack, newArray, ARRAY);
    } else if (topNode->type == ARRAY) {
        STACK array = topNode->value;
        int size = array->sp + 1;
        // com pop faz free duas vezes e dava erro
        fullDeleteStack(array);
        topNode->type = INT;
        topNode->value = malloc(sizeof(long double));
        *(long double *)topNode->value = (long double)size;
    } else {
        filter(stack, topNode, topNode-1);
    }
}

/**
 * @brief separa por espaços em branco uma string e coloca o conteúdo resultante num array alocado
 * @param stack a ser utilizada
 * @return void
*/
void whiteSpacer (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    char * str = topNode->value, *new;
    int beg, i;
    STACK newArray = newStack(stack->varList);
    for (beg = i = 0; str[i]; i++) {
        if (isspace(str[i])) {
            str[i] = '\0';
            new = malloc(sizeof(char) * (i - beg + 2));
            memcpy(new, str + beg, i - beg + 2);
            generalPush(newArray, new, STRING);
            i++; // skip do espaço atual
            while (str[i] && isspace(str[i])) i++;
            if (str[i] == '\0') i--;
            beg = i;
        }
    }
    if (i - beg > 1) {//chegou ao fim
        new = malloc(sizeof(char) * (i - beg + 2));
        memcpy(new, str + beg, i - beg + 2);
        generalPush(newArray, new, STRING);
    }
    pop(stack);
    generalPush(stack, newArray, ARRAY);
}

/**
 * @brief separa por newlines uma string e coloca o conteúdo resultante num array alocado
 * @param stack a ser utilizada
 * @return void
*/
void newLineSpacer (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    char * str = topNode->value, *new;
    int beg, i;
    STACK newArray = newStack(stack->varList);
    for (beg = i = 0; str[i]; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            new = malloc(sizeof(char) *(i - beg + 2));
            memcpy(new, str + beg, i - beg + 2);
            generalPush(newArray, new, STRING);
            i++; // skip do espaço atual
            while (str[i] && str[i] == '\n') i++;
            if (str[i] == '\0') i--;
            beg = i;
        }
    }
    if (i - beg > 1) {//chegou ao fim
        new = malloc(sizeof(char) * (i - beg + 2));
        memcpy(new, str + beg, i - beg + 2);
        generalPush(newArray, new, STRING);
    }
    pop(stack);
    generalPush(stack, newArray, ARRAY);
}

/**
 * @brief lê todo o imput do utilizador e coloca numa string alocada.
 * @param stack a ser utilizada
 * @return void
*/
void readAll (STACK stack) {
    int i, c, chr, size = 32;
    char *line = malloc(32*sizeof(char));
    for (i = c = 0; (chr = getc(stdin)) && chr != EOF; i++, c++) {
        if (c == size - 2) {
            size *= 2;
            line = realloc(line, size*sizeof(char));
        }
        line[i] = chr;
    }
    line[i] = '\0';
    generalPush(stack, line, STRING);
}

/**
 * @brief executa um bloco em ciclo enquanto o resultado da sua execução na stack (o valor do topNode) for verdadeiro.
 * @param stack a ser utilizada
 * @return void
*/
void executew (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    BLOCKPTR block = topNode->value;
    topNode->value = NULL;
    stack->sp--;
    topNode->type = 0;
    int i = getLogicalValue(stack->array + stack->sp);
    while (i && stack->sp != -1) {
        mainHandler(block->string,block->dist, block->handle, stack);
        if ((i = getLogicalValue(stack->array + stack->sp)))
            pop(stack);
    }
    pop(stack); //pop bloco
    free(block->string);
    free(block);
}

/**
 * @brief transforma o último elemento da stack em uma string
 * @param stack a ser utilizada
 * @return void
*/
void toString (STACK stack) {
    NODE topNode = stack->array + stack->sp;
    char *str;
    if (topNode->type == DOUBLE) {
        long double val = *(long double *)topNode->value;
        int length = snprintf(NULL, 0, "%Lg", val);
        str = malloc(sizeof(char)*(length + 1));
        snprintf(str, length + 1, "%Lg", val);
    } else {
        long int val = (long int)*(long double *)topNode->value;
        int length = snprintf(NULL, 0, "%ld", val);
        str = malloc(sizeof(char)*(length + 1));
        snprintf(str, length + 1, "%ld", val);
    }
    pop(stack);
    generalPush(stack, str, STRING);
}

/**
 * @brief faz print de um array
 * @param arr a ser utilizado
 * @param prindist pointer para a dispatch table de funções de print
 * @return void
*/
void printArray (STACK arr, printFunc *printDist[]) {
    NODE bpnode = arr->array, node;
    int i, sp = arr->sp;
    for (i = 0; i <= sp; i++) {
        node = bpnode + i;
        ///printf("TYPE:%d\n", node->type);
        if (node->type == ARRAY) printArray(node->value, printDist);
        else printDist[node->type](node->value);
    }
}

/**
 * @brief faz print do topo da stack
 * @param stack a ser utilizada
 * @return void
*/
void printTop (STACK stack) {
    ///simpleDebug(stack);
    NODE topNode = stack->array + stack->sp;
    printFunc *printDist[33] = {printDouble, printChr, printInt, NULL, printDouble}; //printDist[0]: default é ser long double
    printDist[16] = printStr;
    printDist[32] = printBlock;
    if (topNode->type == ARRAY) {
        printArray(topNode->value, printDist);
    } else {
        printDist[topNode->type](topNode->value);
    }
    putchar('\n');
}
