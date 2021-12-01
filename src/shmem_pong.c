/*****************************************************
 * Copyright Grégory Mounié 2016                     *
 *           Frédéric Pétrot 2016                    *
 *                                                   *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <assert.h>

const int TAILLEX = 800;
const int TAILLEY = 600;

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_SDL_error(msg) \
  do { fprintf(stderr, "SDL :" msg ", ligne %d \n", __LINE__); exit(EXIT_FAILURE); } while (0)


void putpixel(SDL_Surface * surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *) p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *) p = pixel;
        break;
    }
}

/* parametre de l'affichage du carre */
int x;
int y;
int deltax;
int deltay;
int taille;
int couleur;

void draw_ball(SDL_Surface * canvas)
{
    /* fond noir sur l'ancienne position */
    int noir = 0;
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            putpixel(canvas, x + k, y + l, noir);


    /* modification de position */
    x += deltax;
    y += deltay;

    /* inversion des directions et ajustement */
    if (x + taille > TAILLEX) {
        x = TAILLEX - taille;
        deltax *= -1;
    }

    if (x < 0) {
        x = 0;
        deltax *= -1;
    }

    if (y + taille > TAILLEY) {
        y = TAILLEY - taille;
        deltay *= -1;
    }

    if (y < 0) {
        y = 0;
        deltay *= -1;
    }

    /* dessin de la balle */
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            putpixel(canvas, x + k, y + l, couleur);
}


int main(int argc, char **argv)
{
    SDL_Surface *canvas = NULL;
    SDL_Window *ecran = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        handle_SDL_error("Init");

    ecran = SDL_CreateWindow("Ensipong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, TAILLEX, TAILLEY, 0);
    if (ecran == NULL)
        handle_SDL_error("SetVideoMode");

    //   SDL_WM_SetCaption("Mini carrés", NULL);

    srandom(getpid());

    x = random() % TAILLEX;
    y = random() % TAILLEY;
    deltax = random() % 5 + 1;
    deltay = random() % 5 + 1;
    taille = random() % 20 + 10;
    couleur =
        (random() % 256 << 16) | (random() % 256 << 8) | (random() % 255);

  /********************
   * Le tampon a changer pour mettre en place le couplage mémoire
   * entre les différents processus.
   *********************/
    void *tampon = malloc(TAILLEX * TAILLEY * 4);
    if (tampon == NULL)
        handle_error("malloc");

    canvas =
        SDL_CreateRGBSurfaceFrom(tampon, TAILLEX, TAILLEY, 32, TAILLEX * 4,
                                 0x0, 0x0, 0x0, 0x0);
    if (canvas == NULL)
        handle_SDL_error();

    renderer = SDL_CreateRenderer(ecran, -1, SDL_RENDERER_SOFTWARE);

    assert(canvas->pixels == tampon);

    if (SDL_BlitSurface(canvas, NULL, SDL_GetWindowSurface(ecran), NULL) == -1)
        handle_SDL_error("BlitSurface");

    SDL_RenderPresent(renderer);

    while (1) {
        SDL_Delay(20);

        draw_ball(canvas);

        if (SDL_BlitSurface(canvas, NULL, SDL_GetWindowSurface(ecran), NULL) == -1)
            handle_SDL_error("BlitSurface");

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                goto fin;
        }
    }

  fin:
    SDL_FreeSurface(canvas);
    free(tampon);
    SDL_Quit();

    return 0;
}
