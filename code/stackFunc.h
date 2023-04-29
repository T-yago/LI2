#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief estrutura para atribuir tipo ao valor de cada NODE
*/
typedef enum {
    /**
 * @brief o tipo char corresponde a 1
*/
    CHAR = 1,
    /**
 * @brief o tipo int corresponde a 2
*/
    INT = 2,
    /**
 * @brief o tipo double corresponde a 4
*/
    DOUBLE = 4,
    /**
 * @brief o tipo array corresponde a 8
*/
    ARRAY = 8,
    /**
 * @brief o tipo string corresponde a 16
*/
    STRING = 16,
    /**
 * @brief o tipo block corresponde a 32
*/
    BLOCK = 32,
} TYPE;

 /**
 * @brief  número pode ser inteiro, double ou char
*/
#define NUM (INT | DOUBLE | CHAR)

/**
 * @brief verifica se node é um número
*/
#define IS_NUM(n) (n->type & NUM)

/**
 * @brief verifica se algum de dois nodes tem um double
*/
#define HAS_DOUBLE(a, b) ((a | b) & DOUBLE)

/** @brief estrutura de cada nodo referente à stack*/
typedef struct { 

    /**
     * @brief void pointer referente ao valor do node
     */
    void *value;
     /**
     * @brief tipo do valor para o qual o pointer está a apontar
     */
    TYPE type;
    /** @brief pointer para cada node de forma a ser possível alterar o seu conteúdo  */
} nodeStruct,
    /** @brief pointer para cada node de forma a ser possível alterar o seu conteúdo  */
 *NODE; 
 
 
/**
     * @brief  estrutura da stack
     */
typedef struct {
    /**
     * @brief estrutura de cada STACK
     */
    NODE array; 
      /**
     * @brief stack pointer
     */
    int sp;
     /**
     * @brief tamnanho alocado para o array
     */
    int size; 
    /**
     * @brief pointer para o array onde são guardados os valores das variáveis
     * 
     */
    NODE varList;  
    /** @brief pointer para ser possível alterar o conteúdo de cada stack */
} stackStruct,
    /** @brief pointer para ser possível alterar o conteúdo de cada stack */
 *STACK; 


/**
     * @brief estrutura para organizar arrays dentro da stack
     */
typedef struct {
     /**
     * @brief posição original dentro do array 
     */
    int pos;
     /**
     * @brief  void pointer referente ao valor 
     */
    void *value;
    /** @brief pointer para cada estrutura tempOrganizer de forma a ser possível alterar o seu conteúdo */
} organizerStruct, 
    /** @brief pointer para cada estrutura tempOrganizer de forma a ser possível alterar o seu conteúdo */
* tempOrganizer; 


 /**
     *  funções da stack
     */
typedef void stackFunc (STACK stack);

/**
* funções que alteram a stack no entanto precisam de manipular a string de imput
*/
typedef void operHandler (char * * ptr, stackFunc *dist[], void *, STACK stack);

 /**
     * funções que fazem print à stack
     */
typedef void printFunc (void *);

 /**
     * estrutura do bloco
     */
typedef struct {
     /**
     * apontador para dispatch table das stackfuncs
     */
    stackFunc **dist;
     /**
     * apontador para dispatch table das operHandlersfuncs
     */
    operHandler **handle;
      /**
     * input que consta no bloco
     */
    char *string;
    /**  Pointer para cada bloco de forma a ser possível alterar o seu conteúdo */
} blockStruct,
 /**  Pointer para cada bloco de forma a ser possível alterar o seu conteúdo */
 *BLOCKPTR; 


void printInt (void *value);
void printDouble (void *value);
void printStr (void *value);
void printBlock (void *value);
void printChr (void *value);

//funções que não encaixam em stackFunc/auxiliares

void operHandlerDTAssign (operHandler *handle[]);

void varListAssign (NODE varList);

void stackFuncDTAssign (stackFunc *dist[]);

void push (STACK stack, long double val, TYPE t);

void generalPush (STACK stack, void * ptr, TYPE t);

void nodeToInt (NODE node);

void fullDeleteStack (STACK stack);

void deleteVarList (NODE varList);

void deleteStack (STACK stack);

void copyNode (NODE dest, NODE src);

void copyNodePush (STACK stack, NODE src);

int getSize (NODE node);

void deleteNode (NODE node);

void copyNode (NODE dest, NODE src);

void copyNodePush (STACK stack, NODE src);

void nodeToInt (NODE node);

STACK newStack (NODE varList);

void printInt (void *value);

void printDouble (void *value);

void printStr (void *value);

void printChr (void *value);

void debugPrintStack (STACK stack, printFunc *printDist[], int depth);

void debug (STACK stack);

BLOCKPTR copyBlock (BLOCKPTR block);

//operHandlers chamados por handle[]

void eHandler (char **ptr, stackFunc *dist[], void *, STACK stack);
void twoDotsHandler (char **ptr, stackFunc *dist[], void *, STACK stack);
void varHandler (char **ptr, stackFunc *dist[], void *, STACK stack);
void stringBuilder (char **ptr, stackFunc *dist[], void *, STACK stack);
void mainHandler (char * line, stackFunc *dist[], operHandler *handle[], STACK stack);

//Funções que constroem arrays e blocos 
char * blockBuilder (char * lineCrumbs, stackFunc *dist[], operHandler *handle[], STACK stack);
char * arrayBuilder (char * lineCrumbs, stackFunc *dist[], operHandler *handle[], STACK stack);

//stackFunctions chamadas por dist[]

//guiao 1
void add (STACK stack);
void sub (STACK stack);
void divis (STACK stack);
void mod (STACK stack);
void mult (STACK stack);
void inc (STACK stack);
void dec (STACK stack);
void expo (STACK stack);
void pop (STACK stack);
void and (STACK stack);
void or (STACK stack);
void xor (STACK stack);
void not (STACK stack);

//guiao 2
void toInt (STACK stack);
void toChar (STACK stack);
void toDouble (STACK stack);
void swap_top2 (STACK stack);
void duplicate (STACK stack);
void rotate_top3 (STACK stack);
void copy_nth (STACK stack);
void readl (STACK stack);

//guiao 3
void isEqual (STACK stack);
void isGreater (STACK stack);
void isLesser (STACK stack);
void negate (STACK stack);
void IfThenElse (STACK stack);
void top2Greater (STACK stack);
void top2Lower (STACK stack);
void top2Or (STACK stack);
void top2And (STACK stack);

//guiao 4
void range (STACK stack);
void whiteSpacer (STACK stack);
void newLineSpacer (STACK stack);
void readAll (STACK stack);

//guiao 5
void executew (STACK stack);

//etc
void debug (STACK stack);
void simpleDebug (STACK stack);
void toString (STACK stack);
void printTop (STACK stack);