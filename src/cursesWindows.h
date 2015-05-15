#include <curses.h>

/*
WINDOW *createWindow(int alt, int larg, int posy, int posx);
// Menu Handling: criar uma janela

void destroyWindow(WINDOW *win);
// Menu Handling: destruir a janela

void printWindow(char *msg, int posy, int posx);
// Printa a mensagem em uma janela na tela, e espera o usuario digitar alguma tecla
*/

WINDOW *createWindow(int alt, int larg, int posy, int posx) // Cria uma janela nova
{
    WINDOW *win;

    win = newwin(alt, larg, posy, posx);
    box(win, 0, 0); // Contorno Default

    wrefresh(win);

    return win;
}

void destroyWindow(WINDOW *win) // Deleta uma janela
{
    wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' '); // Apagar a borda
    wrefresh(win);

    delwin(win);
}

void printWindow(char *msg, int posy, int posx) // Printa uma janela pre pronta com uma mensagem, sai dela quando o usuario apertar uma tecla, como se fosse um balao de aviso
{
    WINDOW *win;

    int win_alt = 3; // altura da mensagem + 2
    int win_larg = strlen(msg) + 2;

    int a;

    if (posx == -1 && posy == -1)
    {
        posy = (LINES - win_alt)/2;
        posx = (COLS - win_larg)/2;
    }

    win = createWindow(win_alt, win_larg, posy, posx);

    wmove (win, 1, 1);
    wprintw (win, "%s", msg);
    wrefresh(win);
    getchar();

    wmove (win, 1, 1);
    for (a=1; a<win_larg-1; a++)
        wprintw(win, " ");

    destroyWindow(win);
    //wrefresh(win);
    refresh();
}
