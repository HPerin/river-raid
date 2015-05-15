/* ------------------------------------------------------------------------
Trabalho Final - River Raid
Alunos: Lucas Perin & Lucas Weigel
Cadeira: Algoritmos e Programacao - 2013/1 (Turmas A/B)
Professor: Anderson Maciel

Compilando:
Windows - E necessaria a biblioteca PDCURSES
Linux - E necessaria a biblioteca NCURSES
------------------------------------------------------------------------ */

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <ctype.h>
#include <time.h>

#include "cursesWindows.h"
#include "LevelCreator.h"

// Diretorios
#define HIGHSCORE_FILE "highscore.bin"
// Fim

#define MOV_ENEMIES 1 // 1 para ligado, 0 para desligado

// Definicoes gerais
#define SCREEN_SPEED 6 // 1000/SPEED ms, velocidade do cenario
#define SHOT_SPEED 12 // 1000/SPEED ms, velocidade dos tiros
#define MOV_NAVIO 2 // Movimentos dos navios (verticais): 0 -> Desligado, 1 -> Horizontalmente, 2 -> Verticalmente
#define NAVIO_SPEED 1 // 1000/SPEED ms, velocidade dos navios
#define MOV_HELI 1 // Movimento dos helicopteros: 0 -> Desligado, 1 -> Ligado
#define HELI_SPEED 1.5 // 1000/SPEED ms, velocidade dos helicopteros

#define COUNTDOWN_GAME_START 3

#define LARG 85 // Largura da tela
#define ALT 27 // (Altura do cenario)+5

// Fases
#define ALT_MAX 10000
#define MAX_FASES 100

#define NAVE_Y 24 // Posicao da nave (5-24)
#define NAVE_SIZE 1 // Tamanho da nave

#define MAX_SHOTS 5 // Numero max de tiros presentes na tela

#define FUEL_MAX 100 // Default: 100
#define FUEL_PER_25_LINES 10 // Default: 10

#define LIVES_START 3
#define SCORE_PLUS_LIFE 10000

// MENSAGENS
#define EXIT_MSG "Tem certeza que deseja sair? (S/N)"
#define RESET_MSG "Tem certeza que deseja reiniciar? (S/N)"
#define PAUSE_MSG "Jogo Pausado"

// Pontuacoes
#define NAVIO_SCORE 30
#define HELI_SCORE 60
#define BRIDGE_SCORE 200

// Coordenadas info score na tela
#define SCORE_Y 22

// Cores
#define FLORESTA 1
#define RIVER 2
#define SHIP 3
#define SHOT 4
#define COMBUSTIVEL 5
#define ENEMY 6
#define FLORESTA_ENFEITE 7
#define BRIDGE 8

// Highscores
#define MAX_NAME_CHAR 50

typedef struct
{
    int x;
    int y;
} COORDENADAS;
// Estrutura para coordenadas x,y;

typedef struct
{
    char name[MAX_NAME_CHAR];
    int score;
} HIGHSCORE;

/*
MENUS
*/

int confirmAction(char *msg);
// Confirma a vontade do usuario, retorna 1 se ele quer, e 0 se nao

void showInstructions();
// Mostra as instrucoes

void endGame(int score, int how_end);
// Da a tela de morte, printando o score q o usuario fez

void Countdown(int counter, int level_larg);
// Mostra uma janela com uma contagem regressiva, dados os CLOCKS_PER_SECs

void printMenu(int cur_choice, char choices[20][60], int n_choices, int nmr, int linhas);
// Prints the menu, highlighting the cur_choice

int startMenu(int *fases_encontradas, int *linhas, int alt_fases[MAX_FASES], char level[ALT_MAX][LARG]);
// Start the Menu, return the option selected by the user

void exitGame();
// Fecha o programa de forma segura

/*
FILES
*/

int testFiles();
// Testa se os arquivos usados durante o programa existem, retorna o numero de fases disponiveis

int readLevel_seq(int n, char level[ALT_MAX][LARG], int alt_fase[MAX_FASES]);
// Le todos os arquivos chamados Fase(n).txt e coloca o conteudo deles em uma matriz, separados por uma "ponte"

/*
HIGHSCORES
*/

void newHighscore(int score);
// Inicia a funcao que verifica se o usuario fez um novo recorde, e caso sim, ja grava no arquivo

void showHighscores();
// Mostra os highscores existentes na tela

void highscoreName(HIGHSCORE *user);
// Pergunta o nome do usuarioe coloca na posicao ".name" da estrutura dada

/*
GAME
*/

char keyPressed(COORDENADAS shots[MAX_SHOTS], int *ship_x, char screen[ALT][LARG]);
// Se uma tecla foi pressionada, ele detecta a acao correspondente e a inicia

int crashTest(char screen_y[LARG], int pos_x);
// Verifica se uma colisao aconteceu entre a nave e algum objeto, retorna 1 se sim, e 0 se nao

void moveShip(char direction, int *pos_x, char screen[ALT][LARG]);
// Move a nave, essa funcao eh chamada pela keyPressed(), quando a tecla correspondente eh encontrada

void printShip(int ship_x, char screen[ALT][LARG]);
// Coloca a nave na posicao inicial dela

void initializeScreen(char level[ALT_MAX][LARG], char screen[ALT][LARG], int *linhas);
// Inicia a primeira vez a tela do cenario, com inicio no level usado definido pela variavel altura

void nextLine(char level[ALT_MAX][LARG], int altura, char screen[ALT][LARG]);
// Coloca a proxima linha do cenario e deleta a ultima

void createShot(COORDENADAS shots[MAX_SHOTS], int pos_x, char screen[ALT][LARG]);
// Se dentro das condicoes necessarias (Nao excedeu o maximo de tiros) cria um tiro novo

void updateShots(COORDENADAS shots[MAX_SHOTS], char screen[ALT][LARG]);
// Desloca os tiros da nave para cima

void scoreAdd(char ch, int *score);
// Adiciona score dependendo do tipo de objeto destruido

void shotHit(COORDENADAS shots[MAX_SHOTS], char screen_level[ALT][LARG], char screen_ship[ALT][LARG], int *score, int *fase_atual);
// Verifica a colisao de um tiro com um objeto, no caso de haver colisao, o tiro e o objeto sao destruidos

void printColor(char ch, const int color, int bold);
// Printa o caractere dado com a cor dada, o ultimo argumento indica se deve ser negrito ou nao (1 ou 0, respectivamente)

void destroyObject(char ch, COORDENADAS shot, char screen_level[ALT][LARG]);
// Destroi o objeto presente na screen_level que esta em contato com as coordenadas fornecidas

void fuelRechargeTest(char screen_level[ALT][LARG], int ship_x, float *fuel);
// Verifica se ha contato com uma estacao de Combustivel, se sim, adiciona ao combustivel da nave

void moveEnemies(char screen_level[ALT][LARG], char enemy);
// Movimenta os inimigos de certo tipo na tela randomicamente

void refreshScreen(char screen_level[ALT][LARG], char screen_ship[ALT][LARG], int score, float fuel, int fase_atual, int lives, int lps, int n_fases);
// Printa o cenario, a nave e os tiros, alem do score e do combustivel para o usuario

void printScore(int score, float fuel, int lives, int lps, int fase_atual, int n_fases);
// Printa os escores

void startGame (char level[ALT_MAX][LARG], int *score, int linhas, int alt_fase[MAX_FASES], int n_fases);
// Comeca o jogo com base em um cenario dado a funcao, retorna o score realizado pelo usuario

int main()
{
    int score;
    int linhas;
    int n_fases;

    int alt_fases[MAX_FASES];

    int action;

    char level[ALT_MAX][LARG];

    initscr();

    noecho();
    nodelay(stdscr, TRUE); // Para o getch() nao esperar uma tecla, apenas indicar erro quando nada for pressionado
    curs_set(0); // Esconder o cursor
    //keypad(stdscr, TRUE); // Liga as teclas especiais

    srand (time(NULL));

    start_color(); // Iniciar a funcao de cores da ncurses
    //use_default_colors();

    init_pair(FLORESTA, COLOR_GREEN, COLOR_GREEN);
    init_pair(RIVER, COLOR_BLACK, COLOR_CYAN);
    init_pair(SHIP, COLOR_BLACK, COLOR_CYAN);
    init_pair(SHOT, COLOR_YELLOW, COLOR_CYAN);                        // Inicializacao dos pares de cores (fonte/fundo) que serao usados
    init_pair(COMBUSTIVEL, COLOR_RED, COLOR_WHITE);
    init_pair(ENEMY, COLOR_BLACK, COLOR_CYAN);
    init_pair(FLORESTA_ENFEITE, COLOR_BLACK, COLOR_GREEN);
    init_pair(BRIDGE, COLOR_RED, COLOR_RED);

    do
    {
        action = startMenu(&n_fases, &linhas, alt_fases, level); // Menu inicial

        switch (action)   // Switch da opcao selecionada no menu inicial
        {
        case 0:
            if (n_fases == 0) // Se foram encontradas fases
            {
                printWindow("Nao foram encontradas fases!", -1, -1);
            }
            else if (linhas != 0) // E nao ha erros na leitura das fases
            {
                score = 0;
                startGame(level, &score, linhas, alt_fases, n_fases);
                newHighscore(score); // Zera o score, inicia o jogo e verifica se foi feito um novo highscore
            }
            break;
        case 1:
            showHighscores(); // Mostra os highscores
            break;
        case 2:
            createLevel(n_fases+1); // Cria um level, se ja existir fase1.txt, ele cria fase2.txt e assim por diante
            break;
        case 3:
            showInstructions(); // Mostra as instrucoes
            break;
        case 4:
            if (confirmAction(EXIT_MSG) != 1) // Confirma se o usuario quer mesmo sair
                action = -1; // Caso a opcao for nao, retorna para o switch do menu inicial
            break;
        }

    }
    while (action != 4);

    // GAME OVER!
    clear();
    refresh();
    printWindow ("GAME OVER!", -1, -1); // -1, -1 Sera centrado na tela

    endwin();
    return 0;
}

/*
MENU FUNCTIONS
*/

int confirmAction(char *msg)
{
    WINDOW *confirm;

    int win_alt = 3; // altura da mensagem + 2
    int win_larg = strlen(msg) + 2; // largura da mensagem + 2

    int posy = (ALT - win_alt)/2; // [(altura da tela)-(altura da window)]/2 - centralizar
    int posx = (LARG - win_larg)/2; // [(largura da tela)-(largura da window)]/2

    char ch;

    confirm = createWindow(win_alt, win_larg, posy, posx); // Cria uma janela nas coordenadas especificadas e com o tamanho especificado

    wmove (confirm, 1, 1);
    wprintw (confirm, "%s", msg); // Imprime a mensagem na janela
    wrefresh(confirm);

    do
    {
        ch = getchar();
        ch = toupper(ch);
    }
    while (ch != 'S' && ch != 'N'); // Aguarda uma resposta do usuario

    destroyWindow(confirm); // Destroi a janela

    if (ch == 'S')
        return 1;
    return 0; // Retorna o valor correspondente a resposta
}

void showInstructions()
{
    char msg[LARG];

    clear();
    sprintf(msg, "INSTRUCOES");
    mvprintw(3, (LARG-5-strlen(msg))/2, "%s", msg);
    sprintf(msg, "->| Mova sua nave com as teclas A(esquerda) e D(direita).");
    mvprintw(10, (LARG-5-strlen(msg))/2, "%s", msg);
    sprintf(msg, "->| Atire com a tecla K.");                                                // Print das instrucoes
    mvprintw(12, (LARG-5-strlen(msg))/2, "%s", msg);
    sprintf(msg, "->| Recarregue seu combustivel com os Postos(G).");
    mvprintw(14, (LARG-5-strlen(msg))/2, "%s", msg);
    sprintf(msg, "->| Os inimigos sao Helicopteros(%c - %dPts) e Navios(%c - %dPts)", '%', HELI_SCORE, '#', NAVIO_SCORE);
    mvprintw(16, (LARG-5-strlen(msg))/2, "%s", msg);
    sprintf(msg, "->| Atirar nas Pontes(em vermelho) lhe da %d pontos e te faz avancar de Fase.", BRIDGE_SCORE);
    mvprintw(18, (LARG-5-strlen(msg))/2, "%s", msg);

    refresh();
    getchar();
}

void endGame(int score, int how_end)
{
    char msg[LARG];

    clear();
    sprintf(msg, "%s", "------------------- GAME OVER -------------------");
    mvprintw(1, (LARG-strlen(msg))/2, "%s", msg);
    if (how_end < 0)
        sprintf(msg, "%s", "[ Parabens! Voce terminou a ultima fase! ]"); // Caso voce tenha zerado o jogo..
    else
        sprintf(msg, "%s", "[ Nenhuma vida restante... ]"); //... ou morrido.
    mvprintw(((ALT-6)/2), (LARG-strlen(msg))/2, "%s", msg);
    sprintf(msg, "Score final: %d", score);
    mvprintw(((ALT-6)/2)+4, (LARG-strlen(msg))/2, "%s", msg);
    sprintf(msg, "%s", "- SPACE para continuar -");
    mvprintw(ALT-6, (LARG-strlen(msg))/2, "%s", msg);
    refresh();

    while (getchar() != ' ');  // Fica no game over até apertar o espaco...

    clear();
    refresh();
}

void Countdown(int counter, int level_larg)
{
    WINDOW *countdown;

    int win_alt = 3;
    int win_larg = 5;

    int posy = (ALT - win_alt)/2;
    int posx = (level_larg - win_larg)/2;

    clock_t sec_counter; // Variável do tipo clock_t

    counter++;

    countdown = createWindow(win_alt, win_larg, posy, posx);

    wmove (countdown, 1, 2);
    wprintw (countdown, "%d", counter-1);
    wrefresh(countdown);
    counter--;

    sec_counter = clock(); // clock() verifica os segundos

    do
    {
        if (clock() - sec_counter > (CLOCKS_PER_SEC)) // Testa se passou um segundo
        {
            wmove (countdown, 1, 2);
            wprintw (countdown, "%d", counter-1); // Imprime o proximo numero no countdown
            wrefresh(countdown);
            counter--;  // Decrementa o counter do countdown
            sec_counter = clock();
        }
    }
    while (counter > 0); // Imprime os valores de 3 a 1 como uma contagem regressiva, sendo a diminuicao do numero a cada segudo

    destroyWindow(countdown);
}

void printMenu(int cur_choice, char choices[10][60], int n_choices, int nmr, int linhas)
{
    clear();

    int a;
    char inst[80] = {"W e S para navegar - SPACE para selecionar"};

    a = 3;
    mvprintw(a, 10,   " ____  ____  _  _  ____  ____    ____    __    ____  ____  ");
    mvprintw(a+1, 10, "(  _ \\(_  _)( \\/ )( ___)(  _ \\  (  _ \\  /__\\  (_  _)(  _ \\ ");
    mvprintw(a+2, 10, " )   / _)(_  \\  /  )__)  )   /   )   / /(__)\\  _)(_  )(_) )");
    mvprintw(a+3, 10, "(_)\\_)(____)  \\/  (____)(_)\\_)  (_)\\_)(__)(__)(____)(____/ ");

    for (a=0; a<n_choices; a++) // Se for a escolha atual, imprime destacado, se nao, imprime normal
    {
        if (a == cur_choice)
        {
            attron(A_REVERSE); // Inverte a cor do fundo e da letra, para testacar a opcao
            mvprintw (10+2*a, ((LARG-strlen(choices[a]))/2), " %s ", choices[a]);
            attroff(A_REVERSE);
        }
        else
        {
            mvprintw (10+2*a, ((LARG-strlen(choices[a]))/2), " %s ", choices[a]); // As outras opcoes, nao destacadas
        }
    }

    // Printa as instrucoes
    move (LINES-3, (LARG-strlen(inst))/2);
    printw ("%s", inst);
    // Printa informacoes sobre as fases
    move (1, 2);
    printw("Fases encontradas: %d", nmr);
    move (1, 65);
    printw("Linhas: %d", linhas);

    refresh();
}

int startMenu(int *fases_encontradas, int *linhas, int alt_fases[MAX_FASES], char level[ALT_MAX][LARG])
{
    char choices[10][60] =
    {
        "Iniciar o Jogo",
        "Ver Highscores",
        "Criar Level",
        "Instrucoes",
        "Finalizar o Jogo"
    }; // Opcoes

    int n_choices = 5; // Numero de opcoes

    int cur_choice=0;
    char ch;

    printMenu(cur_choice, choices, n_choices, 0, 0);

    *fases_encontradas = testFiles();  // Usa a funcao testFiles para retornar o numero de fases disponiveis

    if (*fases_encontradas < 1)
    {
        createLevel(1);
        *fases_encontradas = 1;
    }

    *linhas = readLevel_seq(*fases_encontradas, level, alt_fases); // Numero de linhas

    do
    {
        printMenu(cur_choice, choices, n_choices, *fases_encontradas, *linhas);

        ch = toupper(getchar());

        switch (ch) // Navegacao no menu principal
        {
        case 'W':
            if (cur_choice >  0)
                cur_choice--;
            else
                cur_choice = n_choices-1;
            break;
        case 'S':
            if (cur_choice < n_choices-1)
                cur_choice++;
            else
                cur_choice = 0;
            break;
        }
    }
    while (ch != ' ');

    return cur_choice;
}

/*
FILES FUNCTIONS
*/
int testFiles()
{
    // highscore.bin
    // fase1.txt, fase2.txt, ...
    char fase[30];
    int fase_n=1;

    HIGHSCORE zerado;
    int a;

    FILE *fp;
    if (!(fp = fopen(HIGHSCORE_FILE, "rb"))) // Se nao existir um arquivo highscore.bin, cria outro zerado
    {
        fp = fopen(HIGHSCORE_FILE, "wb");
        if (!fp)
        {
            printWindow("Erro na criacao do arquivo \"highscore.bin\"!", 1, 1); // Mensagem de erro
            endwin();
            exit(1);
        }

        strcpy (zerado.name, " ");
        zerado.score = 0;

        for (a=0; a<10; a++)
        {
            fwrite (&zerado, sizeof(HIGHSCORE), 1, fp); // Zera todos os 10 espacos de highscore
        }

        printWindow("Arquivo \"highscore.bin\" criado!", 1, 1); // Mensagem de sucesso
    }
    fclose(fp);

    sprintf(fase, "fase%d.txt", fase_n);
    fp = fopen(fase, "r");
    while (fp) // Vai abrindo as fases ate nao achar mais nenhuma
    {
        fase_n++;
        fclose(fp);

        sprintf(fase, "fase%d.txt", fase_n);
        fp = fopen(fase, "r");
    }

    return (fase_n-1);
}

int readLevel_seq(int n, char level[ALT_MAX][LARG], int alt_fase[MAX_FASES]) // Le o nivel, armazena na matriz e conta as linhas no processo
{
    char fase[30];

    int linha=ALT;
    int a, b;

    FILE *fase_atual;

    for (a=n; a>0; a--)
    {
        sprintf (fase, "fase%d.txt", a);
        fase_atual = fopen(fase, "r");

        if (!fase_atual) // Erro no fopen
        {
            printWindow("Erro ao tentar carregar as fases!", -1, -1);
            linha = 0;
            return linha;
        }

        while (!feof(fase_atual) && linha<ALT_MAX) // Até chegar no final
        {
            fgets (level[linha], LARG-1, fase_atual);
            linha++; // Conta as linhas
        }
        linha--;

        if (linha >= ALT_MAX)
        {
            printWindow("Erro: Numero de linhas maximo atingido!", -1, -1);
            //printWindow("Favor retirar uma ou mais fases...", -1, -1);
            linha = 0;
            return linha;
        }

        fclose(fase_atual);

        alt_fase[a] = linha-2;
    }

    for (a=0; a<ALT; a++)
    {
        for (b=0; b<(strlen(level[ALT])-1); b++)
        {
            level[a][b] = 'T'; // Preenche com os T`s...
        }
        level[a][b] = '\n'; //... e termina a linha
    }

    if (linha != 0 && linha < ALT)
    {
        printWindow("Erro: Numero de linhas insuficiente!", -1 , -1);
        linha = 0;
    }

    return linha;
}

/*
HIGHSCORE
*/
void newHighscore(int score)
{
    HIGHSCORE all_high[10];
    HIGHSCORE new_high;
    FILE *fp;

    int a, b;

    fp = fopen(HIGHSCORE_FILE, "rb");
    for (a=0; a<10; a++)
    {
        fread(&all_high[a], sizeof(HIGHSCORE), 1, fp); // Le os higscores
    }
    fclose(fp);

    new_high.score = score;

    a=0;
    while (all_high[a].score >= new_high.score && a < 10) // Fica até o score obtido ser maior que algum highscore armazenado...
    {
        a++;
    }

    if (a < 10) //... se achou, e está entre os 10 melhores
    {
        highscoreName(&new_high);

        for (b=9; b>a; b--)
        {
            all_high[b] = all_high[b-1]; // Move os highscores menores que o obtido uma posicao para baixo
        }
        strcpy(all_high[a].name, new_high.name); // Armazena o nome do jogador do highscore obtido...
        all_high[a].score = new_high.score;      // ... e sua pontuacao.
    }

    fp = fopen(HIGHSCORE_FILE, "wb");
    for (a=0; a<10; a++)
    {
        fwrite (&all_high[a], sizeof(HIGHSCORE), 1, fp);
    }
    fclose(fp);
}

void showHighscores() // Funcao que mostra a tela de highscores ao selecionarmos essa opcao no menu
{
    clear();
    HIGHSCORE cur_high;
    FILE *fp;
    int a, b;

    fp = fopen(HIGHSCORE_FILE, "rb");

    mvprintw(1, (LARG-strlen("HIGHSCORES"))/2, "HIGHSCORES");

    move (((LINES-20)/2), 1);
    for (b=0; b<78; b++)
    {
        printw("-"); // Printa as linhas entre cada pontuacao
    }
    for (a=0; a<10; a++)
    {
        fread(&cur_high, sizeof(HIGHSCORE), 1, fp);
        move (((LINES-20)/2)+(2*a)+1, 2);
        printw("%2.d. %s", a+1, cur_high.name);
        if (cur_high.score > 0)
        {
            move (((LINES-20)/2)+(2*a)+1, 60);
            printw("Score: %d", cur_high.score);
        }
        move (((LINES-20)/2)+(2*a)+2, 1);
        for (b=0; b<78; b++)
        {
            printw("-"); // Mais linhas
        }
    }

    refresh();
    getchar();
}

void highscoreName(HIGHSCORE *user) // Funcao que recebe, do jogador, seu nome ao atingir um highscore
{
    WINDOW *win;

    int a;

    int win_alt = 5; // altura da mensagem + 2
    int win_larg = MAX_NAME_CHAR + 2;

    int posy = (ALT - win_alt)/2;
    int posx = (LARG - win_larg)/2;

    win = createWindow(win_alt, win_larg, posy, posx);

    wmove (win, 1, (win_larg-strlen("Parabens, voce conseguiu um novo recorde!"))/2);
    wprintw (win, "Parabens, voce conseguiu um novo recorde!");
    wmove (win, 2, (win_larg-strlen("Digite seu nome:"))/2);
    wprintw (win, "Digite seu nome:");
    wrefresh(win);

    echo();
    wmove (win, 3, 1);
    for (a=0; a<MAX_NAME_CHAR; a++)
        wprintw(win, " ");
    wrefresh(win);
    wmove (win, 3, 1);
    wgetnstr(win, (*user).name, MAX_NAME_CHAR);
    noecho();

    wrefresh(win);

    destroyWindow(win); // Funcao que fecha a janela da mensagem
}

/*
GAME FUNCTIONS
*/
char keyPressed(COORDENADAS shots[MAX_SHOTS], int *ship_x, char screen[ALT][LARG])
{
    int ch = getch();
    if (ch != ERR)
    {
        ch = toupper(ch);
        switch (ch) // Tecla presionada durante o jogo
        {
        case 'K': //K, de atirar
            createShot(shots, *ship_x, screen);
            break;
        case 'A': //A, de andar para a esquerda
        case 'D': //D, de andar para a direita
            moveShip (ch, ship_x, screen);
            break;
        case 27: // ESC, de sair
            if (confirmAction(EXIT_MSG) == 1)
                return 'Q';
            break;
        case 10: // ENTER, de pausar
            printWindow(PAUSE_MSG, -1, -1);
            break;
        case 'R': // R, de reiniciar
            if (confirmAction(RESET_MSG) == 1)
                return 'R';
            break;
        }
        ch = getch();
    }
    return 0;
}

int crashTest(char screen_y[LARG], int pos_x) // Funcao que testa colisao
{
    if ((screen_y[pos_x] != ' ') && (screen_y[pos_x] != 'G') && (screen_y[pos_x] != '!')) // Se bater em alguma coisa...
    {
        return 1;
    }
    return 0;
}

void moveShip(char direction, int *pos_x, char screen[ALT][LARG]) // Move a nave
{
    if (direction == 'A' || direction == 'a') // Esquerda
    {
        screen[NAVE_Y][*pos_x] = ' ';
        *pos_x = *pos_x - 1;
        screen[NAVE_Y][*pos_x] = 'A';
    }

    if (direction == 'D' || direction == 'd') // Direita
    {
        screen[NAVE_Y][*pos_x] = ' ';
        *pos_x = *pos_x + 1;
        screen[NAVE_Y][*pos_x] = 'A';
    }
}

void printShip(int ship_x, char screen[ALT][LARG]) // Printa a nave na tela
{
    screen[NAVE_Y][ship_x] = 'A';
}

void initializeScreen(char level[ALT_MAX][LARG], char screen[ALT][LARG], int *linhas)
{
    int a;

    for (a=ALT-1; a >= 0; a--, (*linhas)--)
    {
        strcpy (screen[a], level[*linhas]);
    }
    (*linhas)++;
}

void nextLine(char level[ALT_MAX][LARG], int altura, char screen[ALT][LARG])
{
    int b;

    for (b=ALT-1; b>0; b--)
    {
        strcpy (screen[b], screen[b-1]);
    }
    strcpy (screen[0], level[altura]);
}

void createShot(COORDENADAS shots[MAX_SHOTS], int pos_x, char screen[ALT][LARG])
{
    int err=0; // Procura por espaco livre para colocar o tiro novo
    int a = -1; // índice
    int shot;
    do
    {
        a++;
        shot = shots[a].y;
        if (shots[a].y == NAVE_Y-1) // para evitar que alguem segure o botao de disparo e solte 2 tiros sobrepostos
            err = 1;
        if (a == MAX_SHOTS-1 && shot > 5)
            err = 1; // Erro
    }
    while (shot > 5 && a<MAX_SHOTS);

    if (err == 0) // Se nao deu erro (tem espaco livre)
    {
        shots[a].x = pos_x; //POS_X
        shots[a].y = NAVE_Y-1; //POS_Y

        screen[shots[a].y][shots[a].x] = '!'; // Coloca o tiro
    }
}

void updateShots(COORDENADAS shots[MAX_SHOTS], char screen[ALT][LARG]) // Desloca os tiros para cima
{
    int a;

    for (a=0; a<MAX_SHOTS; a++)
    {
        if (shots[a].y > 5)
        {
            screen[shots[a].y][shots[a].x] = ' ';
            if (shots[a].y > 6)
            {
                shots[a].y--;
                screen[shots[a].y][shots[a].x] = '!';
            }
            else
            {
                shots[a].y = 1;
            }
        }
    }
}

void scoreAdd(char ch, int *score) // Incrementa a pontuacao ao destruir um inimigo
{
    switch (ch)
    {
    case 'N':
        *score += NAVIO_SCORE; // Ganha os pontos de um navio...
        break;
    case 'X':
        *score += HELI_SCORE; //... de um helicoptero...
        break;
    case 'P':
        *score += BRIDGE_SCORE; //... e de uma ponte.
        break;
    }
}

void shotHit(COORDENADAS shots[MAX_SHOTS], char screen_level[ALT][LARG], char screen_ship[ALT][LARG], int *score, int *fase_atual) // Colisao entre tiro e um objeto
{
    int a;
    char ch;

    for (a=0; a<MAX_SHOTS; a++)
    {
        if (shots[a].y > 2)
        {
            ch = screen_level[shots[a].y][shots[a].x];
            if (ch != ' ')
            {
                destroyObject (ch, shots[a], screen_level);
                scoreAdd(ch, &(*score)); // Aumenta a pontuacao
                screen_ship[shots[a].y][shots[a].x] = ' ';
                shots[a].y = 1;
                if (ch == 'P')
                    *fase_atual += 1; // Destruiu a ponte, vai para a próxima fase
            }
        }
    }
}

void printColor(char ch, const int color, int bold) // Cores
{
    if (bold == 1) // Negrito
    {
        attron (COLOR_PAIR(color) | A_BOLD);
        printw ("%c", ch);
        attroff (COLOR_PAIR(color) | A_BOLD);
    }
    else
    {
        attron (COLOR_PAIR(color));
        printw ("%c", ch);
        attroff (COLOR_PAIR(color));
    }
}

void destroyObject(char ch, COORDENADAS shot,  char screen_level[ALT][LARG]) // Destroi os objetos atingidor por um tiro
{
    int a;
    int dir[2];

    int start_hor;
    int start_ver;

    switch (ch)
    {
    case 'N':
        // Checagem da direcao do navio NNNN
        a=shot.x;
        while (screen_level[shot.y][a] == 'N')
            a--;
        dir[0] = -(a - shot.x);
        start_hor = (a - shot.x)+1; // posicao inicial do navio (eixo x)
        a=shot.x+1;
        while (screen_level[shot.y][a] == 'N')
            a++;
        dir[0] += (a - shot.x);
        a=shot.y;
        while (screen_level[a][shot.x] == 'N')
            a--;
        dir[1] = -(a - shot.y);
        start_ver = (a - shot.y)+1; // posicao inicial do navio (eixo y)
        a=shot.y+1;
        while (screen_level[a][shot.x] == 'N')
            a++;
        dir[1] += (a - shot.y);
        if (dir[0] == 5) // se for na direcao x, deleta este
        {
            a = shot.x + start_hor;
            while (screen_level[shot.y][a] == 'N')
            {
                screen_level[shot.y][a] = ' ';
                a++;
            }
        }
        else if (dir[1] == 5) // se for na direcao y, delete o mesmo
        {
            a = shot.y + start_ver;
            while (screen_level[a][shot.x] == 'N')
            {
                screen_level[a][shot.x] = ' ';
                a++;
            }
        }
        break;
    case 'P': // Ponte
        a=shot.x;
        while (screen_level[shot.y][a] == 'P')
        {
            while (screen_level[shot.y][a] == 'P') // deleta a parte esquerda da ponte
            {
                screen_level[shot.y][a] = ' ';
                a--;
            }
            a=shot.x+1;
            while (screen_level[shot.y][a] == 'P') // deleta a parte direita da ponte
            {
                screen_level[shot.y][a] = ' ';
                a++;
            }

            shot.y--; // avanca uma linha para checagem se a ponte tem mais linhas a serem destruidas
            a=shot.x;
        }
        break;
    case 'X': // Helicoptero ou...
    case 'G': // ... gasolina.
        screen_level[shot.y][shot.x] = ' '; // simplismente destroi o objeto
        break;
    }
}

void fuelRechargeTest(char screen_level[ALT][LARG], int ship_x, float *fuel)
{
    if (screen_level[NAVE_Y][ship_x] == 'G') // Se está encima de um posto de gasolina
    {
        *fuel += 30; // Incrementa fuel
    }
    if (*fuel > FUEL_MAX)
    {
        *fuel = FUEL_MAX; // Se passou do máximo, diminui para o máximo
    }
}

void moveEnemies(char screen_level[ALT][LARG], char enemy)
{
    int dir;
    int hv;
    int a, b, c;

    for (a=5; a<(ALT-5); a++) // comeca a procurar inimigos da posicao inicial do cenario aparente (cenario q esta na tela) e para antes de o inimigo comecar a sair da tela (para evitar q o programa teste valores de fora da matriz)
    {
        for (b=0; b<(strlen(screen_level[a])); b++)
        {
            dir = rand()%2; // Direcao aleatória
            if (dir == 0)
                dir--;

            if (screen_level[a][b] == enemy) // Se tem um inimigo nessa posicao
            {
                switch (screen_level[a][b])
                {
                case 'X': // Se é um helicóptero
                    hv = rand()%2;

                    if (hv == 0) // Movimento vertical
                    {
                        if (screen_level[a][b+dir] == ' ' && screen_level[a][b+(2*dir)] == ' ') // Testa se há espaco
                        {
                            screen_level[a][b] = ' '; // Espaco em branco onde ele estava
                            screen_level[a][b+dir] = 'X'; // Nova posicao dele
                            b++;
                        }
                    }
                    if (hv == 1) // Movimento horizontal
                    {
                        if (screen_level[a+dir][b] == ' ' && screen_level[a+(2*dir)][b] == ' ')
                        {
                            if (screen_level[a+dir][b+1] == ' ' && screen_level[a+dir][b-1] == ' ')
                            {
                                screen_level[a][b] = ' ';
                                screen_level[a+dir][b] = 'X';
                                a++;
                            }
                        }
                    }
                    break;
                case 'N':
                    /* NAVIO HORIZONTAL */
                    if (screen_level[a][b] == 'N' && screen_level[a][b+1] == 'N' && screen_level[a][b+2] == 'N' && screen_level[a][b+3] == 'N')
                    {
                        if (dir < 0 && screen_level[a][b-1] == ' ' && screen_level[a][b-2] == ' ')
                        {
                            screen_level[a][b-1] = 'N'; // apenas modifica 1 caractere, os outros 3 se manterao
                            screen_level[a][b+3] = ' ';
                        }
                        if (dir > 0 && screen_level[a][b+4] == ' ' && screen_level[a][b+5] == ' ')
                        {
                            screen_level[a][b+4] = 'N';
                            screen_level[a][b] = ' ';
                            b++;
                        }
                    }
                    /* NAVIO VERTICAL */
                    /* Mov. Horizontal */
                    if (MOV_NAVIO == 1)
                    {
                        // faz a checagem para possibilidade de movimento
                        if (screen_level[a][b] == 'N' && screen_level[a+1][b] == 'N' && screen_level[a+2][b] == 'N' && screen_level[a+3][b] == 'N') // navio existe
                        {
                            if (screen_level[a][b+dir] == ' ' && screen_level[a+1][b+dir] == ' ' && screen_level[a+2][b+dir] == ' ' && screen_level[a+3][b+dir] == ' ') // n tem nada na posicao que o navio ira ser colocado
                            {
                                if ((screen_level[a][b+(2*dir)] == ' ' && screen_level[a+1][b+(2*dir)] == ' ' && screen_level[a+2][b+(2*dir)] == ' ' && screen_level[a+3][b+(2*dir)] == ' ')) // nao ha nada 1 posicao a frente de onde o navio sera colocado (evita superposicao)
                                {
                                    for (c=0; c<4; c++)
                                    {
                                        screen_level[a+c][b] = ' ';
                                        screen_level[a+c][b+dir] = 'N';
                                    }
                                    b++;
                                }
                            }
                        }
                    }
                    /* Mov. Vertical */
                    if (MOV_NAVIO == 2)
                    {
                        if (screen_level[a][b] == 'N' && screen_level[a+1][b] == 'N' && screen_level[a+2][b] == 'N' && screen_level[a+3][b] == 'N') // navio existe
                        {
                            if (dir < 0 && screen_level[a-1][b] == ' ' && screen_level[a-2][b] == ' ') // checagem de espaco livre na posicao a frente e a frente+1
                            {
                                if (screen_level[a-1][b-1] == ' ' && screen_level[a-1][b+1] == ' ') // Evita superposicao
                                {
                                    screen_level[a-1][b] = 'N';
                                    screen_level[a+3][b] = ' ';
                                }
                            }
                            if (dir > 0 && screen_level[a+4][b] == ' ' && screen_level[a+5][b] == ' ')
                            {
                                if (screen_level[a+4][b-1] == ' ' && screen_level[a+4][b+1] == ' ') // Evita superposicao
                                {
                                    screen_level[a+4][b] = 'N';
                                    screen_level[a][b] = ' ';
                                    a++;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

void refreshScreen(char screen_level[ALT][LARG], char screen_ship[ALT][LARG], int score, float fuel, int fase_atual, int lives, int lps, int n_fases)
{
    int a, b;
    int alt;

    //clear();

    // print screen_level
    for (a=5; a<ALT; a++) // printa apenas a partir da posicao 5 da matriz (antes disso ela eh usada para destruicao de [navios na vertical/pontes/...] antes mesmo deles chegarem ao mapa
    {
        alt = a-5;
        b=0;
        while (screen_level[a][b] != '\0' && b < LARG)
        {
            move (alt-1, b);
            switch (screen_level[a][b]) // Vai printando no mapa
            {
            case ' ':
                printColor(screen_level[a][b], RIVER, 0);
                break;
            case 'G':
                printColor(screen_level[a][b], COMBUSTIVEL, 1);
                break;
            case 'X':
                printColor('%', ENEMY, 0);
                break;
            case 'N':
                printColor('#', ENEMY, 0);
                break;
            case 'T':
                printColor(screen_level[a][b], FLORESTA, 0);
                break;
            case 'P':
                printColor('P', BRIDGE, 0);
                break;
            default:
                printColor(screen_level[a][b], FLORESTA_ENFEITE, 1);
                break;
            }
            b++;
        }
    }

    // print screen_ship
    for (a=5; a<ALT; a++)
    {
        alt = a-5;
        b=0;
        while (screen_level[a][b] != '\0' && b < LARG)
        {
            if (screen_ship[a][b] != ' ')
            {
                move (alt-1, b);
                switch (screen_ship[a][b])
                {
                case '!':
                    printColor('!', SHOT, 1);
                    //printColor(screen_ship[alt][b], SHOT, 1);
                    break;
                case 'A':
                    printColor(screen_ship[a][b], SHIP, 0);
                    break;
                }
            }
            b++;
        }
    }

    printScore(score, fuel, lives, lps, fase_atual, n_fases); // printa a telinha de scores abaixo do cenario

    refresh();
}

void printScore(int score, float fuel, int lives, int lps, int fase_atual, int n_fases)
{
    int a;

    for (a=0; a<LARG-5; a++)
        mvprintw (SCORE_Y, a, " ");

    mvprintw (SCORE_Y, 2, "Score: %d", score);

    mvprintw (SCORE_Y, 26, "Fuel: ");
    printw("[");
    for (a=0; a<FUEL_MAX; a += FUEL_MAX/10)
    {
        if (fuel > a) // Representacao da gasolina
            printw(">");
        else
            printw(" ");
    }
    printw("]");

    mvprintw (SCORE_Y, 60, "Lives: ");
    if (lives < 4) // Até 3 vidas, printa A's
        for (a=0; a<lives; a++)
            printw ("%c ", 'A');
    else // Depois, printa o numero de vidas
        printw("%d", lives);

    mvprintw (0, 71, "LPS:%d", lps); // Loops por segundo do jogo
    mvprintw (1, 71, "Fase:%d/%d", fase_atual, n_fases); // [(fase_ataual)/(numero_de_fases)]
}

void startGame (char level[ALT_MAX][LARG], int *score, int linhas, int *alt_fase, int n_fases) // Principal funcao do jogo, é ela que comeca ele baseado nos mapas
{
    clear();

    int altura = linhas-ALT;
    char screen_level[ALT][LARG];
    char screen_ship[ALT][LARG];
    int ship_x;
    int crash;
    float fuel;
    int lives = LIVES_START;
    int life_plus_score = SCORE_PLUS_LIFE; // Quantos pontos precisa para ganhar a proxima vida
    int user_status = 0;
    int fase_atual = 1;

    int lps_count;
    int lps = 0;

    COORDENADAS shots[MAX_SHOTS];

    int a, b;

    clock_t screen_step;
    clock_t shot_step;
    clock_t navio_step;
    clock_t heli_step;
    clock_t second;

    while (lives > 0 && user_status != 'Q')
    {
        //altura = linhas-1; //alt_fase[fase_atual];
        altura = alt_fase[((fase_atual-1)%n_fases)+1];

        crash = 0;
        ship_x = strlen(level[1])/2;
        fuel = FUEL_MAX;

        // Zerar matriz da nave
        for (a=0; a<ALT; a++)
        {
            for (b=0; b<LARG; b++)
            {
                screen_ship[a][b] = ' ';

            }
        }
        // Zerar coordenadas dos tiros
        for (a=0; a<MAX_SHOTS; a++)
        {
            shots[a].x = 1;
            shots[a].y = 1;
        }

        initializeScreen(level, screen_level, &altura);
        printShip(ship_x, screen_ship);
        refreshScreen (screen_level, screen_ship, *score, fuel, fase_atual, lives, lps, n_fases);

        Countdown(COUNTDOWN_GAME_START, strlen(screen_level[0]));
        clear();
        refreshScreen (screen_level, screen_ship, *score, fuel, fase_atual, lives, lps, n_fases); // Retirar vestigios do CountDown

        screen_step = clock();
        shot_step = clock();
        heli_step = clock();
        navio_step = clock();
        second = clock();

        while ((getch()) != ERR); // Limpar a entrada

        lps_count = 0;
        do
        {
            if (*score > life_plus_score) // Se passou de 10.000
            {
                life_plus_score += SCORE_PLUS_LIFE;
                lives++; // +1 vida, ja que obteu 10.000 pontos
            }

            if (clock() - screen_step > (CLOCKS_PER_SEC)/(SCREEN_SPEED+(0.5*fase_atual))) // etapas de movimento da tela
            {
                altura--;
                if (altura < 0)
                {
                    altura = alt_fase[1];
                }
                nextLine (level, altura, screen_level);  // proxima linha

                shotHit(shots, screen_level, screen_ship, score, &fase_atual); // testa colisao com tiros

                fuel = fuel - (FUEL_PER_25_LINES/25.0); // diminui o combustivel

                fuelRechargeTest(screen_level, ship_x, &fuel); // verifica condicao de recarga do combustivel

                screen_step = clock();
            }

            if (clock() - shot_step > (CLOCKS_PER_SEC)/(SHOT_SPEED)) // Se é momento de mover os tiros..
            {
                updateShots(shots, screen_ship);

                shotHit(shots, screen_level, screen_ship, score, &fase_atual);

                shot_step = clock();
            }

            if (MOV_ENEMIES != 0)
            {
                if (MOV_HELI != 0)
                {
                    if (clock() - heli_step > (CLOCKS_PER_SEC)/(HELI_SPEED+(0.2*fase_atual))) // Se é momento de mover o helicóptero...
                    {
                        moveEnemies(screen_level, 'X');

                        heli_step = clock();
                    }
                }

                if (MOV_NAVIO != 0)
                {
                    if (clock() - navio_step > (CLOCKS_PER_SEC)/(NAVIO_SPEED+(0.2*fase_atual))) // ... ou o navio.
                    {
                        moveEnemies(screen_level, 'N');

                        navio_step = clock();
                    }
                }

                shotHit(shots, screen_level, screen_ship, score, &fase_atual);
            }

            if (clock() - second > CLOCKS_PER_SEC)
            {
                lps = lps_count;
                lps_count = 0;
                second = clock();
            }

            user_status = keyPressed(shots, &ship_x, screen_ship);

            refreshScreen (screen_level, screen_ship, *score, fuel, fase_atual, lives, lps, n_fases);

            crash = crashTest(screen_level[NAVE_Y], ship_x);

            lps_count++; // Ocorreu um loop do jogo
        }
        while (crash == 0 && fuel > 0 && user_status != 'Q' && user_status != 'R' && fase_atual <= n_fases); // Q de quit (apertou ESQ e confirmou), e R de Reiniciar (Apertou R e conformiu)

        if (fase_atual > n_fases) // Se voce passou todas as fases, o jogo te dá 0 vidas para acabar
            lives = 0;
        else if (user_status != 'Q' && user_status != 'R')
            lives--;
        else if (user_status == 'R')
        {
            lives = LIVES_START; // Reinicia a vida..
            fase_atual = 1;      // E os níveis.
        }
    }

    endGame(*score, (n_fases-fase_atual));
}
