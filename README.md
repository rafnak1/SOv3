# Trabalho de Sistemas Operacionais (PCS3746)

Trabalho da disciplina PCS3746 - Sistemas Operacionais, do 3º módulo Acadêmico do Curso de Engeharia da Computação da Escola Politécnica da Universidade de São Paulo

## Membros do Grupo:

- [@AugustoVaccarelli](https://github.com/AugustoVaccarelli) - Augusto Vaccarelli Costa  10770197
- [@Flute-Gamer](https://github.com/Flute-Gamer) - Igor Souza Lima e Silva Caixeta 11918564
- [@luimot](https://github.com/luimot) - Luiz Fernando de Almeida Mota 11807790
- [@rafnak1](https://github.com/rafnak1) - Rafael Katsuo Nakata 11803819

## Escopo do projeto

A proposta deste projeto é simular o gerenciamento de processos e de memória de um sistema operacional. A interface com o usuário é separada em dois terminais: um deles só é usado para digitar comandos para o simulador; no outro, é exibido o status da simulação, com informações como a fila de processos e o bitmap da memória de usuário.

## Como usar

Este programa é feito para ser rodado num ambiente linux. Dois exemplos de ambientes onde ele deveria funcionar são: WSL e Ubuntu. Segue um passo a passo para o uso do programa:

- Abra dois terminais, sendo que um deles deve estar no diretório deste repositório. Este terminal será o de entrada, e o outro, de saída.
- Execute o comando de bash "tty" no terminal de saída.
- Copie a saída deste comando (será alguma coisa parecida com "/dev/pts/2", sendo que o "2" pode ser qualquer outro número) para o arquivo pts.txt.
- Execute o comando "make" no terminal de entrada. Isso irá compilar o programa.
- O executável "simulador" será gerado. Execute-o com "./simulador" no terminal de entrada.

### Opções de linha de comando

O simulador tem duas opções de linha de comando, sendo que ambas são números inteiros. A primeira é 0 (escalonamento FIFO) ou 1 (escalonamento round-robin). A segunda é qualquer número inteiro positivo, e designa o número de ticks de clock que duram um quantum do round-robin. Por exemplo:

```./simulador 1 4```

Irá executar o simulador no modo round-robin com quantum igual a 4 ciclos de clock de processador.

Não é necessário especificar a segunda opção caso a primeira seja 0. Exemplo:

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

## Comentários gerais

O nome deste repositório é "SOv3" porque esta é a terceira tentativa (e a única bem-sucedida) de realizar este trabalho. Foram feitos outros dois repositórios antes deste, mas ambos não foram para frente por causa de "refatoração prematura": o código foi dividido em arquivos do jeito que o grupo achou que faria sentido, mas no final essa organização tornou o código macarrônico. A proposta deste repositório é refatoração conforme necessidade, a fim de evitar organizações arbitrárias que preveem erroneamente as necessidades futuras do projeto. 

Como pode ser verificado pelo histórico de commits, o projeto foi feito praticamente durante um dia inteiro. Isso explica o resultado de apenas um grande arquivo-fonte. Como o projeto não prevê mudanças para além da entrega para o professor da disciplina, e ele já está funcionando corretamente, optamos por deixar o programa organizado em monolito. 
