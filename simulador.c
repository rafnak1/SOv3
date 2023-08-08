#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>

typedef struct ProgramaLido
{
    char linhas[30][30];
    int n_linhas;
} tipoProgramaLido;

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
    tipoProgramaLido programa;
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
int pidmap[20] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

int pc, ax, bx;

tipoPcb pcbAtual = {.ax = 0, .bx = 0, .pc = 0, .pid = -1};

int pidDoProximoProcessoASerCriado = 0;

int roundRobinTimer = 0;

bool preemptivo;
int quantum;

// O elemento para o qual o placeholder aponta é o primeiro da fila.
// Quando o ultimoDaFila aponta para o placeholder, a fila está vazia.
tipoProcessoDaFila placeholder = {.proximo = NULL};
tipoProcessoDaFila *ultimoDaFila = &placeholder;

void configuraSimulacao(int argc, char **argv);

void mostraStatusSimulacao();
void imprimeMapaDeBits(FILE *f_tty);
void imprimeFilaDeProntos(FILE *f_tty);
void imprimeRegistradoresDaCpu(FILE *f_tty);
void imprimePcbAtual(FILE *f_tty);
void imprimeStatusPrograma(FILE *f_tty);
void imprimeSeparador(FILE *f_tty);

void interpretaComando(char comando[]);

int novoPid();
void insereCreate(int tamanhoMem);
void insereKill(int pidParaMatar);
int insereProcessoUsuario(int pid);
int criaProcessoSeForCreate();
void mataProcessoSeForKill();
void voltaProcessoAtualParaFinalDaFila();

tipoPcb *acessaIndiceFila(int i);
tipoPcb dequeueFilaProntos();

void trocaContextoSemPreempcao();
void trocaContextoComPreempcao();

void ler_instrucao(char *linha);
tipoProgramaLido carregaPrograma(char *filename);

int reservaMemoria(int pid, int tamanhoMemoria);
void liberaMemoria(int pid);
int firstFit(int tamanhoMemoria);
void executaCompactador();

int main(int argc, char **argv)
{
    char comando[100];

    configuraSimulacao(argc, argv);

    while (true)
    {
        mostraStatusSimulacao();
        fgets(comando, 100, stdin);
        interpretaComando(comando);
    }

    return 0;
}

void configuraSimulacao(int argc, char **argv)
{
    if (argc == 3)
    {
        preemptivo = atoi(argv[1]);
        quantum = atoi(argv[2]);
    }
    else
    {
        printf("É obrigadório inserir as duas opções de linha de comando.\n");
        exit(-1);
    }
}

void interpretaComando(char comando[])
{
    int tamanhoMemoria, pidParaMatar;

    if (sscanf(comando, "create -m %d", &tamanhoMemoria) == 1)
    {
        insereCreate(tamanhoMemoria);
    }
    else if (sscanf(comando, "kill %d", &pidParaMatar) == 1)
    {
        insereKill(pidParaMatar);
    }
    else if (comando[0] == '\n')
    {
        printf("clock tick\n");
        if (pcbAtual.programa.n_linhas != 0) {
            pc++;
            roundRobinTimer++;
        }
        if (pc == pcbAtual.programa.n_linhas)
        {
            // Se nem o compactador consegue abrir espaço,
            // o create vai para o final da fila
            if (criaProcessoSeForCreate() == -1)
                voltaProcessoAtualParaFinalDaFila();
            mataProcessoSeForKill();
            trocaContextoSemPreempcao();
        } else if (roundRobinTimer == quantum && preemptivo) {
            trocaContextoComPreempcao();
        } 
        
    }
    else
    {
        printf("Erro: Comando inválido!\n");
    }
}

// PC atingiu o HLT: Programa vai retornar
void trocaContextoSemPreempcao()
{
    liberaMemoria(pcbAtual.pid);
    pcbAtual = dequeueFilaProntos();
    pc = pcbAtual.pc;
    ax = pcbAtual.ax;
    bx = pcbAtual.bx;

    roundRobinTimer = 0;
}

// Processo foi interrompido.
// Deverá voltar ao final da fila.
void trocaContextoComPreempcao()
{
    voltaProcessoAtualParaFinalDaFila();

    pcbAtual = dequeueFilaProntos();
    pc = pcbAtual.pc;
    ax = pcbAtual.ax;
    bx = pcbAtual.bx;

    roundRobinTimer = 0;
}

void insereCreate(int tamanhoMem)
{
    char nomeArquivo[20] = "create.s";
    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));

    tipoPcb pcb = {.tipoDeProcesso = CREATE, .tamanhoMem = tamanhoMem, .pid = novoPid(), .programa = carregaPrograma(nomeArquivo)};
    novoProcesso->pcb = pcb;

    novoProcesso->proximo = NULL;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

void insereKill(int pidParaMatar)
{
    char nomeArquivo[20] = "kill.s";

    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));

    tipoPcb pcb = {.tipoDeProcesso = KILL, .pidParaMatar = pidParaMatar, .pid = novoPid(), .programa = carregaPrograma(nomeArquivo)};
    novoProcesso->pcb = pcb;

    novoProcesso->proximo = NULL;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

int insereProcessoUsuario(int pid)
{
    char nomeArquivo[20] = "programa_usuario.s";

    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));

    tipoPcb pcb = {.tipoDeProcesso = PROCESSO_USUARIO, .pid = pid, .programa = carregaPrograma(nomeArquivo)};
    novoProcesso->pcb = pcb;

    novoProcesso->proximo = NULL;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

int criaProcessoSeForCreate()
{
    int resultado;
    int pid = novoPid();

    if (pcbAtual.tipoDeProcesso == CREATE)
    {
        resultado = reservaMemoria(pid, pcbAtual.tamanhoMem);
        if (resultado == -1)
        {
            executaCompactador();
            resultado = reservaMemoria(pid, pcbAtual.tamanhoMem);
            // Não deu para reservar nem depois da compactação
            if (resultado == -1)
            {
                return -1;
            }
        }
        insereProcessoUsuario(pid);
    }
    return 0;
}

void voltaProcessoAtualParaFinalDaFila()
{
    tipoProcessoDaFila *novoProcesso = (tipoProcessoDaFila *)malloc(sizeof(tipoProcessoDaFila));
    novoProcesso->pcb = pcbAtual;
    novoProcesso->pcb.pc = pc;
    novoProcesso->pcb.ax = ax;
    novoProcesso->pcb.bx = bx;
    ultimoDaFila->proximo = novoProcesso;
    ultimoDaFila = novoProcesso;
}

void mataProcessoSeForKill()
{
    int i = 0;
    tipoProcessoDaFila *processo = &placeholder;
    tipoProcessoDaFila *processoAnterior;

    if (pcbAtual.tipoDeProcesso == KILL)
    {
        while (processo->proximo != NULL)
        {
            processoAnterior = processo;
            processo = processo->proximo;
            if (processo->pcb.pid == pcbAtual.pidParaMatar)
            {
                processoAnterior->proximo = processo->proximo;
                free(processo);
            }
        }
        liberaMemoria(pcbAtual.pidParaMatar);
    }
}

int reservaMemoria(int pid, int tamanhoMemoria)
{
    int indiceParaReservar = firstFit(tamanhoMemoria);
    if (indiceParaReservar == -1)
    {
        return -1;
    }
    for (int i = 0; i < tamanhoMemoria; i++)
    {
        bitmap[indiceParaReservar + i] = true;
        pidmap[indiceParaReservar + i] = pid;
    }
    return 0;
}

void executaCompactador()
{
    int inicio, final, pid, tamanho, primeiroIndiceLivre;
    for (int i = 0; i < 20; i++)
    {
        if (bitmap[i] == true)
        {
            pid = pidmap[i];
            inicio = i;
            final = i;
            while (final < 20 && pidmap[final] == pid)
                final++;
            tamanho = final - inicio;

            primeiroIndiceLivre = 0;
            while (bitmap[primeiroIndiceLivre] == true)
                primeiroIndiceLivre++;

            liberaMemoria(pid);
            for (int j = 0; j < tamanho; j++)
            {
                bitmap[primeiroIndiceLivre + j] = true;
                pidmap[primeiroIndiceLivre + j] = pid;
            }

            i = primeiroIndiceLivre + tamanho;
        }
    }
}

// Retorna -1 se não há slots contíguos o suficiente de memória
int firstFit(int tamanhoMemoria)
{
    int contadorPosicoesContiguas = 0;
    for (int i = 0; i < 20; i++)
    {
        if (bitmap[i] == false)
        {
            contadorPosicoesContiguas++;
        }
        else
        {
            contadorPosicoesContiguas = 0;
        }
        if (contadorPosicoesContiguas == tamanhoMemoria)
            return i - tamanhoMemoria + 1;
    }
    return -1;
}

void liberaMemoria(int pid)
{
    for (int i = 0; i < 20; i++)
    {
        if (pidmap[i] == pid)
        {
            pidmap[i] = -1;
            bitmap[i] = false;
        }
    }
}

int novoPid()
{
    return pidDoProximoProcessoASerCriado++;
}

/* Carregador de Programas *.s */
tipoProgramaLido carregaPrograma(char *filename)
{
    tipoProgramaLido programa;
    FILE *f = fopen(filename, "r");
    char linha[100];
    int n_linhas = 0;
    while (fgets(linha, sizeof(linha), f))
    {
        for (int i = 0; linha[i]; i++)
        {
            if (linha[i] == '\n')
            {
                linha[i] = '\0';
                break;
            }
            linha[i] = toupper(linha[i]);
        }
        strcpy(programa.linhas[n_linhas], linha);
        n_linhas++;
    }
    programa.n_linhas = n_linhas;
    return programa;
}

tipoPcb dequeueFilaProntos()
{
    tipoPcb pcb;
    if (placeholder.proximo == NULL)
    {
        pcb.programa.n_linhas = 0; // Para saber que nenhum programa está executando
        pcb.pc = 0;
        pcb.ax = 0;
        pcb.bx = 0;
        pcb.pid = -1;
        return pcb;
    }
    pcb = placeholder.proximo->pcb;
    placeholder.proximo = placeholder.proximo->proximo;
    if (placeholder.proximo == NULL)
        ultimoDaFila = &placeholder;
    return pcb;
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

void mostraStatusSimulacao()
{

    FILE *f_tty = fopen("/dev/pts/7", "w");

    imprimeFilaDeProntos(f_tty);
    imprimeStatusPrograma(f_tty);
    imprimeMapaDeBits(f_tty);
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
    fprintf(f_tty, "| REGISTRADORES DA CPU | PC: %d  | AX: %d | BX: %d  |\n", pc, ax, bx);
    fprintf(f_tty, "+----------------------+--------+-------+--------+\n");
}

void imprimePcbAtual(FILE *f_tty)
{
    fprintf(f_tty, "+-----------+--------+--------+--------+--------+\n");
    fprintf(f_tty, "| PCB ATUAL | PID: %d | PC: %d  | AX: %d  | BX: %d  |\n", pcbAtual.pid, pcbAtual.pc, pcbAtual.ax, pcbAtual.bx);
    fprintf(f_tty, "+-----------+--------+--------+--------+--------+\n");
}

void ler_instrucao(char *linha)
{
    if (strcmp(linha, "HLT\n") == 0)
        printf("Programa em execução entrou em halt \n");
}

void imprimeStatusPrograma(FILE *f_tty)
{
    tipoProgramaLido programaExecutando = pcbAtual.programa;
    int pcAtual = pcbAtual.pc;

    fprintf(f_tty, "+------------+\n");
    for (int i = 0; i < programaExecutando.n_linhas; i++)
    {
        fprintf(f_tty, "| ");
        fprintf(f_tty, "%s", programaExecutando.linhas[i]);
        if (pc == i)
        {
            fprintf(f_tty, "\t<-- \n");
            if (i == programaExecutando.n_linhas - 1)
                printf("Última instruçao \n");
            ler_instrucao(programaExecutando.linhas[i]);
        }
        else
            fprintf(f_tty, "\n");
    }
    fprintf(f_tty, "+------------+\n");
}

void imprimeSeparador(FILE *f_tty)
{
    fprintf(f_tty, "\n\n========================================================================================\n\n");
}