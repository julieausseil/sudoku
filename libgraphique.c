/*******************************************************************************
 * libraphique.c                                                               *
 * Quelques fonctions C basÃ©es sur la SDl pour rÃ©aliser des opÃ©rations         *
 * graphiques simples                                                          *
 *                                                                             *
 * Pour compiler en ligne de commande :                                        *
 * gcc ../lib/libgraphique.c prog.c -o resultat                                *
 *           `sdl-config --libs --cflags` -lm -lSDL                            *
 *                                                                             *
 * oÃ¹                                                                          *
 *             prog.c : votre code source                                      *
 *             resultat    : nom de l'exÃ©cutable                               *
 *******************************************************************************
*/

////////////////////////////////////////////////////////////////////////////////
// 0. directives prÃ©processeur

#include <stdlib.h>
#include <stdio.h>
#include "libgraphique.h"


////////////////////////////////////////////////////////////////////////////////
// 0. variables globales et macros

SDL_Surface *ecran = NULL;                 // totalitÃ© de l'Ã©cran
SDL_Event lastevent ;                      // utilisÃ© pour gestion Ã©vÃ©nements
int LARGEUR = -1 ;                         // largeur de l'Ã©cran en pixels
int HAUTEUR = -1 ;                         // hauteur de l'Ã©cran en pixels
#define octets_par_pixel ecran->format->BytesPerPixel
#define largeur_ecran (ecran->pitch / 4)


////////////////////////////////////////////////////////////////////////////////
// 1. Ouvrir et fermer une fenÃªtre

// ouvrir une fenÃªtre de taille largeur (x), hauteur (y)
void ouvrir_fenetre(int largeur, int hauteur){
    SDL_Init(SDL_INIT_VIDEO); 
    ecran = SDL_SetVideoMode(largeur, hauteur, 32, SDL_SWSURFACE ); 

    //initialisation des variables globales
    LARGEUR = largeur ;
    HAUTEUR = hauteur ;
    printf("LARGEUR %d HAUTEUR %d\n",LARGEUR,HAUTEUR);

    // pour permettre les rÃ©pÃ©titions de touche si elles restent enfoncÃ©es
    SDL_EnableKeyRepeat(100, 100);
}


// terminer le programme
void fermer_fenetre(){
    SDL_Quit();
    exit(0);
}

// teste si la fin du programme a Ã©tÃ© demandÃ©e et le termine si nÃ©cessaire
void _test_arret() {
    if ((lastevent.type == SDL_QUIT) || 
            ( (lastevent.type == SDL_KEYDOWN )
              && (lastevent.key.keysym.sym == SDLK_ESCAPE)) 
       )
        fermer_fenetre() ;
}


///////////////////////////////////////////////////////////////////////////////
// 2. Fonctions de dessin

// actualise l'affichage des modifications graphiques
// sans appel Ã  cet fonction les modifications sont non apparentes
void actualiser(){
    SDL_PollEvent(&lastevent) ;
    _test_arret();
    SDL_Flip(ecran) ;
}


// fonction de dessin principale
// changer la Couleur du Point pix
void changer_pixel(Point pix, COULEUR Couleur) {
    if ((0 <= pix.x) && (pix.x < LARGEUR) && (0 <= pix.y ) && (pix.y < HAUTEUR))
    {
        *( (Uint32*)ecran->pixels + pix.y * largeur_ecran + pix.x ) = Couleur ;
    }
}


// dessine un rectangle de Couleur de largeur et hauteur donnÃ©es
// coin est le coin haut, gauche
void dessiner_rectangle(Point coin, int largeur, int hauteur, COULEUR Couleur) {
    Point p ;
    int bord_droit = coin.x + largeur ;
    int bord_bas = coin.y + hauteur ;
    for (p.x = coin.x; p.x < bord_droit ; ++(p.x)) {
        for (p.y = coin.y ; p.y  < bord_bas ; ++(p.y) ) {
            changer_pixel(p, Couleur);
        }
    }
}

// trace une ligne du Point p1 au point p2 dela Couleur donnÃ©e
// utilise l'algorithme de Bresenham
void dessiner_ligne(Point p1, Point p2, COULEUR Couleur)
{
    // signes deplacement x et y
    int dx, dy ;

    printf("x %d y %d   ",p2.x,p2.y);
    if (p1.x < p2.x)
        dx = 1 ;
    else if (p1.x > p2.x)
        dx = -1 ;
    else
        dx = 0 ;

    if (p1.y < p2.y)
        dy = 1 ;
    else if (p1.y > p2.y)
        dy = -1 ;
    else
        dy = 0 ;
    printf("dx %d dy %d   ",dx,dy);

    // valeur absolue pente
    float pente_abs ; 

    float erreur = 0;
    Point p ; // point courant

    //lignes horizontales et certicales : plus rapide
    if (dy == 0) 
    {
        printf(" H\n");
        p.y = p1.y ;
        for(p.x = p1.x ; p.x != p2.x ; p.x += dx)
            changer_pixel(p,Couleur);
    }
    else if (dx == 0)
    {  
        printf(" V\n");
        p.x = p1.x ;
        for(p.y = p1.y ; p.y != p2.y ; p.y += dy)
            changer_pixel(p,Couleur);
    }
    else
    {

        pente_abs  = (p2.y - p1.y)/((float)(p2.x - p1.x)) ;
        if (pente_abs < 0)
            pente_abs = - pente_abs ;
        printf("%f ", pente_abs);

        if (pente_abs <=1 )
        {
            printf(" A\n");
            {
                p.y = p1.y ; 
                for(p.x = p1.x ; p.x != p2.x ; p.x += dx)
                {
                    changer_pixel(p, Couleur);
                    erreur += pente_abs ;
                    printf("erreur %f\n",erreur);
                    if (erreur > 0.5)
                    {
                        erreur -= 1 ;
                        p.y += dy ;
                    }
                }
            }
        }
        else 
        {
            pente_abs = 1/pente_abs ;
            printf(" B\n");
            p.x = p1.x ; 
            for(p.y = p1.y ; p.y != p2.y ; p.y += dy)
            {
                changer_pixel(p, Couleur);
                erreur += pente_abs ;
                if (erreur > 0.5)
                {
                    erreur -= 1 ;
                    p.x += dx ;
                }
            }
        }
    }
    changer_pixel(p2,Couleur);
}


// affiche l'image sous forme .bmp (bitmap), contenue dans le mÃªme dossier
// nom est une chaine de caracteres qui est le nom (complet) du fichier image
// coin est le coin haut, gauche voulu pour l'image Ã  afficher dans l'ecran
void afficher_image(char *nom, Point coin){
    SDL_Surface *img = SDL_LoadBMP(nom) ;
    SDL_Rect position_img ;
    position_img.x = coin.x;
    position_img.y = coin.y;
    SDL_BlitSurface(img,NULL,ecran,&position_img);
}


////////////////////////////////////////////////////////////////////////////////
// 3. Gestion des Ã©vÃ©nements

// renvoie le code SDLK de la prochaine touche pressÃ©e
// fonction bloquante
int attendre_touche(void){
    do {
        SDL_WaitEvent(&lastevent) ;
        _test_arret() ;
    }
    while (lastevent.type != SDL_KEYDOWN ) ;
    return lastevent.key.keysym.sym;
}

// renvoie les coordonnees du prochain clic (gauche ou droite) de souris
// fonction bloquante
Point attendre_clic() {
    do {
        SDL_WaitEvent(&lastevent) ;
        _test_arret();
    }
    while (lastevent.type != SDL_MOUSEBUTTONDOWN) ;
    Point p ;
    p.x = lastevent.button.x ;
    p.y = lastevent.button.y ;
    return p;
}


////////////////////////////////////////////////////////////////////////////////
// 4. Autres

// renvoie une Couleur (UInt32) RGB avec les valeurs entre 0 et 255 donnÃ©es
// en rouge r, vert g et bleu b
COULEUR fabrique_Couleur(int r, int g, int b)
{
    COULEUR C;
    return ((r%256)<<16) + ((g%256)<<8) + (b%256);
    return C;
}




