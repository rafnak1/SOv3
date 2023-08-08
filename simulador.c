#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

enum TipoDeProcesso
{
    PROCESSO_USUARIO,
    CREATE,
    KILL
};

/** Process Control Block
 *  Armazena as informações do processo
 */
typedef struct PCB
{
    int pid;
    int pc, ax, bx;
    enum TipoDeProcesso tipoDeProcesso;
    int tamanhoMem;   // só é útil se o processo for create
    int pidParaMatar; // só é útil se o processo for kill
} tipoPcb;

// Fila de prontos é uma lista ligada.
typedef struct processo
{
    tipoPcb pcb;
    struct processo *proximo;
} tipoProcessoDaFila;

bool bitmap[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

int pc, ax, bx;

tipoPcb pcbAtual = {.ax = 1, .bx = 3, .pc = 3, .pid = 8};

// O elemento para o qual o placeholder aponta é o primeiro da fila.
// Quando o ultimoDaFila aponta para o placeholder, a fila está vazia.
tipoProcessoDaFila placeholder = {.proximo = NULL};
tipoProcessoDaFila *ultimoDaFila = &placeholder;

void mostraStatusSimulacao();
void imprimeMapaDeBits(FILE *f_tty);
void imprimeFilaDeProntos(FILE *f_tty);
void imprimeRegistradoresDaCpu(FILE *f_tty);
void imprimePcbAtual(FILE *f_tty);
void imprimeStatusPrograma(FILE *f_tty);
void imprimeSeparador(FILE *f_tty);

void interpretaComando(char comando[]);

void insereCreate();
void insereKill();
void insereProcessoUsuario();
tipoPcb *acessaIndiceFila(int i);

int main()
{
    char comando[100];

    insereCreate();
    insereKill();
    insereProcessoUsuario();

    while (true)
    {

        mostraStatusSimulacao();
        fgets(comando, 100, stdin);
        interpretaComando(comando);
    }

    return 0;
}

void interpretaComando(char comando[])
{
    int tamanhoMemoria, pidParaMatar;

    if (sscanf(comando, "create -m %d", &tamanhoMemoria) == 1)
    {
        printf("create -m %d\n", tamanhoMemoria);
    }
    else if (sscanf(comando, "kill %d", &pidParaMatar) == 1)
    {
        printf("kill %d\n", pidParaMatar);
    }
    else if (comando[0] == 'a')
    {
        printf("clock tick\n");
    }
    else
    {
        printf("Erro: Comando inválido!\n");
    }
}

tipoPcb *acessaIndiceFila(int i)
{
    tipoProcessoDaFila *p = &placeholder;
    for (int j = 0; j <= i; j++)
    {
        if (p->proximo == NULL)
            return NULL;
        p = p->proximo;
    }
    return &p->pcb;
}

void insereCreate()
{
    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));

    tipoPcb pcb = {.tipoDeProcesso = CREATE, .tamanhoMem = 4};
    novoProcesso->pcb = pcb;

    novoProcesso->proximo = NULL;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

void insereKill()
{
    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));

    tipoPcb pcb = {.tipoDeProcesso = KILL, .pidParaMatar = 2};
    novoProcesso->pcb = pcb;

    novoProcesso->proximo = NULL;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

void insereProcessoUsuario()
{
    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));

    tipoPcb pcb = {.tipoDeProcesso = PROCESSO_USUARIO, .pid = 9};
    novoProcesso->pcb = pcb;

    novoProcesso->proximo = NULL;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

void mostraStatusSimulacao()
{

    FILE *f_tty = fopen("/dev/pts/8", "w");

    imprimeStatusPrograma(f_tty);
    imprimeMapaDeBits(f_tty);
    imprimeFilaDeProntos(f_tty);
    imprimeRegistradoresDaCpu(f_tty);
    imprimePcbAtual(f_tty);
    imprimeSeparador(f_tty);

    fclose(f_tty);
}

void imprimeMapaDeBits(FILE *f_tty)
{
    fprintf(f_tty, "+---------------+\n");
    fprintf(f_tty, "| MAPA DE BITS  |\n");
    fprintf(f_tty, "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n");
    fprintf(f_tty, "|");
    for (int i = 0; i < 20; i++)
    {
        fprintf(f_tty, " %d |", bitmap[i]);
    }
    fprintf(f_tty, "\n");
    fprintf(f_tty, "+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+\n");
}

void imprimeFilaDeProntos(FILE *f_tty)
{
    int i = 0;
    tipoPcb *pcb;

    fprintf(f_tty, "+-----------------+\n");
    fprintf(f_tty, "| FILA DE PRONTOS |\n");
    fprintf(f_tty, "+-----------------+\n");
    while (pcb = acessaIndiceFila(i++))
    {
        if (pcb->tipoDeProcesso == CREATE)
            fprintf(f_tty, "| CREATE -m  %d    |\n", pcb->tamanhoMem);
        else if (pcb->tipoDeProcesso == KILL)
            fprintf(f_tty, "| KILL %d          |\n", pcb->pidParaMatar);
        else if (pcb->tipoDeProcesso == PROCESSO_USUARIO)
            fprintf(f_tty, "| PID %d           |\n", pcb->pid);
        fprintf(f_tty, "+-----------------+\n");
    }
}

void imprimeRegistradoresDaCpu(FILE *f_tty)
{
    fprintf(f_tty, "+----------------------+--------+-------+--------+\n");
    fprintf(f_tty, "| REGISTRADORES DA CPU | PC: %d  | AX: 8 | BX: 2  |\n", 5);
    fprintf(f_tty, "+----------------------+--------+-------+--------+\n");
}

void imprimePcbAtual(FILE *f_tty)
{
    fprintf(f_tty, "+-----------+--------+--------+--------+--------+\n");
    fprintf(f_tty, "| PCB ATUAL | PID: %d | PC: %d  | AX: %d  | BX: %d  |\n", pcbAtual.pid, pcbAtual.pc, pcbAtual.ax, pcbAtual.bx);
    fprintf(f_tty, "+-----------+--------+--------+--------+--------+\n");
}

void imprimeStatusPrograma(FILE *f_tty)
{
    fprintf(f_tty, "+------------+\n");

    fprintf(f_tty, "| MOV AX, 10   <--\n");
    fprintf(f_tty, "| MOV BX, AX\n");
    fprintf(f_tty, "| JMP 0\n");
    fprintf(f_tty, "| HLT\n");

    fprintf(f_tty, "+------------+\n");
}

void imprimeSeparador(FILE *f_tty)
{
    fprintf(f_tty, "\n\n========================================================================================\n\n");
}