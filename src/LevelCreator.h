#include <stdio.h>
#include <time.h>

#define LEVEL_ALT 400
#define LEVEL_LARG 80 // > (3*LARG_FLORESTA)
#define LARG_FLORESTA 20
#define ENEMIES_FREQ 1000 // Quanto maior mais raro (minimo = 7)
#define MAX_DIST_GAS 70 // Valor real = 150% disso
#define ISLAND_MAX_LARG 16 // Precisa ser menor que LARG_FLORESTA
#define ISLAND_MAX_ALT 20
#define ISLAND_MAX_DIST 50 // Distancia maxima entre 2 ilhas
#define ISLAND_MIN_DIST 10 // Distancia minima entre 2 ilhas

#define ALT_PONTE 2

void createLevel(int n)
{
    char level_name[50];
    char level_t[LEVEL_ALT][LEVEL_LARG+1];
    FILE *level;

    int a, b, c;
    int r, i_larg, i_alt;
    int free_space;
    int g_dist = -(rand()%((LEVEL_LARG-LARG_FLORESTA)-(2*LARG_FLORESTA)) + LARG_FLORESTA);

    srand(time(NULL));

    for (a=0; a<LEVEL_ALT; a++) // cria a ponte e a floresta
    {
        if (a < ALT_PONTE)
        {
            for (b=0; b<LEVEL_LARG; b++)
            {
                if (b < (LEVEL_LARG/4))
                    level_t[a][b] = 'T';
                else if (b > (LEVEL_LARG-LEVEL_LARG/4))
                    level_t[a][b] = 'T';
                else
                    level_t[a][b] = 'P';
            }
        }
        else
        {
            for (b=0; b<LEVEL_LARG; b++)
            {
                if (b < LARG_FLORESTA)
                    level_t[a][b] = 'T';
                else if (b > (LEVEL_LARG-LARG_FLORESTA))
                    level_t[a][b] = 'T';
                else
                    level_t[a][b] = ' ';
            }
        }
        level_t[a][b] = '\0';
    }

    a=ALT_PONTE+10;
    while (a<(LEVEL_ALT-ISLAND_MAX_ALT-10)) // cria as ilhas
    {
        r = rand()%(LEVEL_LARG-(2*LARG_FLORESTA));
        r += LARG_FLORESTA;

        i_alt = (rand()%ISLAND_MAX_ALT)+1;

        for (c=a; c<(a+i_alt); c++)
        {
            i_larg = (rand()%ISLAND_MAX_LARG)+2;

            for (b=(r-(i_larg/2)); b<(r+(i_larg/2)); b++)
                level_t[c][b] = 'T';
        }

        a += i_alt;
        a += rand()%(ISLAND_MAX_DIST-ISLAND_MIN_DIST);
        a += ISLAND_MIN_DIST;
    }

    for (a=ALT_PONTE+5; a<LEVEL_ALT-15; a++) // coloca os objetos no mapa randomicamente
    {
        for (b=LARG_FLORESTA; b<(LEVEL_LARG-LARG_FLORESTA); b++)
        {
            r = rand()%ENEMIES_FREQ + 1;

            g_dist++;
            if (g_dist > MAX_DIST_GAS * (LEVEL_LARG-LARG_FLORESTA))
                r = 10;

            switch (r)
            {
            case 1:
            case 2:
                if (level_t[a][b] == ' ' && level_t[a+1][b] == ' ' && level_t[a-1][b] == ' ' && level_t[a][b+1] == ' ' && level_t[a][b-1] == ' ')
                    level_t[a][b] = 'X';
                break;
            case 4:
            case 5:
                free_space = 0;
                for (c=b; c<(b+4); c++)
                    if (level_t[a][c] == ' ' && level_t[a+1][c] == ' ' && level_t[a-1][c] == ' ' && level_t[a][c+1] == ' ' && level_t[a][c-1] == ' ')
                        free_space++;
                if (free_space == 4)
                    for (c=0; c<4; c++)
                        level_t[a][b+c] = 'N';
                break;
            case 7:
            case 8:
                free_space = 0;
                for (c=a; c<(a+4); c++)
                    if (level_t[c][b] == ' ' && level_t[c+1][b] == ' ' && level_t[c-1][b] == ' ' && level_t[c][b+1] == ' ' && level_t[c][b-1] == ' ')
                        free_space++;
                if (free_space == 4)
                    for (c=0; c<4; c++)
                        level_t[a+c][b] = 'N';
                break;
            case 10:
                free_space = 0;
                for (c=a; c<(a+4); c++)
                    if (level_t[c][b] == ' ' && level_t[c+1][b] == ' ' && level_t[c-1][b] == ' ' && level_t[c][b+1] == ' ' && level_t[c][b-1] == ' ')
                        free_space++;
                if (free_space == 4)
                {
                    for (c=0; c<4; c++)
                        level_t[a+c][b] = 'G';
                    g_dist = -(rand()%((LEVEL_LARG-LARG_FLORESTA)-(2*LARG_FLORESTA)) + LARG_FLORESTA);
                }
                break;
            }
        }
    }

    sprintf(level_name, "fase%d.txt", n);
    level = fopen(level_name, "w");
    for (a=0; a<LEVEL_ALT; a++)
        fprintf(level, "%s\n", level_t[a]); // printa no arquivo
    fclose(level);
}
