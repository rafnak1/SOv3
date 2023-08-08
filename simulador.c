#include <stdbool.h>
#include <stdio.h>

bool bitmap[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

void mostraStatusSimulacao();
void imprimeMapaDeBits(FILE *f_tty);
void imprimeFilaDeProntos(FILE *f_tty);
void imprimeRegistradoresDaCpu(FILE *f_tty);
void imprimePcbAtual(FILE *f_tty);
void imprimeStatusPrograma(FILE *f_tty);
void imprimeSeparador(FILE *f_tty);

void interpretaComando(char comando[]);

int main()
{
    char comando[100];

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
    fprintf(f_tty, "+-----------------+\n");
    fprintf(f_tty, "| FILA DE PRONTOS |\n");
    fprintf(f_tty, "+-----------------+\n");
    fprintf(f_tty, "| PID %d           |\n", 4);
    fprintf(f_tty, "+-----------------+\n");
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
    fprintf(f_tty, "| PCB ATUAL | PID: %d | PC: 0  | AX: 2  | BX: 7  |\n", 8);
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