# Trabalho de Sistemas Operacionais (PCS3746)

Trabalho da disciplina PCS3746 - Sistemas Operacionais, do 3º módulo Acadêmico do Curso de Engeharia da Computação da Escola Politécnica da Universidade de São Paulo

## Membros do Grupo:

- [@AugustoVaccarelli](https://github.com/AugustoVaccarelli) - Augusto Vaccarelli Costa  10770197
- [@Flute-Gamer](https://github.com/Flute-Gamer) - Igor Souza Lima e Silva Caixeta 11918564
- [@luimot](https://github.com/luimot) - Luiz Fernando de Almeida Mota 11807790
- [@rafnak1](https://github.com/rafnak1) - Rafael Katsuo Nakata 11803819

## Como usar

Este programa é feito para ser rodado num ambiente linux. Dois exemplos de ambientes onde ele deveria funcionar são: WSL e Ubuntu. Segue um passo a passo para o uso do programa:

- Abra dois terminais, sendo que um dele deve estar no diretório deste repositório. Este terminal será o de entrada, e o outro, de saída.
- Execute o comando de bash "tty" no no terminal de saída.
- Copie a saída deste comando (será alguma coisa parecida com "/dev/pts/2", sendo que o "2" pode ser qualquer outro número) para o arquivo pts.txt.
- Execute o comando "make" no terminal de entrada. Isso irá compilar o programa.
- O executável "simulador" será gerado. Execute-o com "./simulador" no terminal de entrada.

### Opções de linha de comando

O simulador tem duas opções de linha de comando, sendo que ambas são números inteiros. A primeira é 0 (escalonamento FIFO) ou 1 (escalonamento round-robin). A segunda é qualquer número inteiro positivo, e designa o número de ticks de clock que duram um quantum do round-robin. Por exemplo:

```./simulador 1 4```

Irá executar o simulador no modo round-robin com quantum igual a 4 ciclos de clock de processador.

Não é necessário especificara segunda opção caso a primeira seja 0. Exemplo:

```./simulador 0```

Irá executar o simulador no modo FIFO.

### Comandos do simulador

O simulador tem 3 comandos principais:
- `create -m [inteiro]` insere um processo de sistema na fila de prontos, que, quando termina de ser executado, cria um novo processo de usuário. O número fornecido como argumento corresponde a quantos slots o processo de usuário irá precisar.

- `kill [inteiro]` insere um processo de sistema na fila de prontos, que, quando termina de ser executado, mata um processo da fila de prontos com o PID igual ao inteiro fornecido.

- Newline (apenas pressionando Enter): esse comando "vazio" simula um novo sinal de clock do processador, fazendo com que o Program Counter avance a execução do programa.

### Programas em (pseudo) assembly

Os arquivos .s contidos neste repositório podem ser executados na simulação. As instruções são meramente ilustrativas, ou seja, não fazem nada (apenas a HLT, ou halt, é útil para o simulador saber onde o programa acaba). São eles:
- `create.s`: o programa de sistema que cria processos de usuário.
- `kill.s`: o programa de sistema que mata processos de usuário.
- `programa_usuario.s`: esse programa é "executado" toda vez que um processo de usuário é escalonado.
- `4instrucoes.s` e `11instrucoes.s`: o propósito desses arquivos é o usuário copiar o conteúdo de um deles para `programa_usuario.s`, a fim de poder executar programas de usuário maiores (11 instruções) ou menores (4 instruções). 
