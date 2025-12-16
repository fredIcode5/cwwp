#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===============================
   STRUCTURE AVL
   =============================== */
typedef struct arbre {
    char id[200];        // identifiant du noeud parent
    float fuite;         // somme des fuites
    int nb_enfants;      // nombre d'enfants
    int eq;
    struct arbre *fg;
    struct arbre *fd;
} Arbre;

/* ===============================
   OUTILS AVL
   =============================== */
int max(int a, int b) { return a > b ? a : b; }

/* ===============================
   ROTATIONS
   =============================== */
Arbre* rotG(Arbre* a) {
    Arbre* p = a->fd;
    a->fd = p->fg;
    p->fg = a;
    return p;
}

Arbre* rotD(Arbre* a) {
    Arbre* p = a->fg;
    a->fg = p->fd;
    p->fd = a;
    return p;
}

/* ===============================
   ÉQUILIBRAGE
   =============================== */
Arbre* equilibrer(Arbre* a) {
    if (a->eq >= 2) {
        if (a->fd->eq < 0)
            a->fd = rotD(a->fd);
        return rotG(a);
    }
    if (a->eq <= -2) {
        if (a->fg->eq > 0)
            a->fg = rotG(a->fg);
        return rotD(a);
    }
    return a;
}

/* ===============================
   CRÉATION NOEUD
   =============================== */
Arbre* creerNoeud(const char* id, float fuite) {
    Arbre* n = malloc(sizeof(Arbre));
    if (!n) exit(1);
    strcpy(n->id, id);
    n->fuite = fuite;
    n->nb_enfants = 1;
    n->eq = 0;
    n->fg = n->fd = NULL;
    return n;
}

/* ===============================
   INSERTION AVL
   =============================== */
Arbre* insererAVL(Arbre* a, const char* id, float fuite, int* h) {
    if (!a) {
        *h = 1;
        return creerNoeud(id, fuite);
    }

    int cmp = strcmp(id, a->id);

    if (cmp < 0) {
        a->fg = insererAVL(a->fg, id, fuite, h);
        *h = -*h;
    } else if (cmp > 0) {
        a->fd = insererAVL(a->fd, id, fuite, h);
    } else {
        // même parent → nouvel enfant
        a->fuite += fuite;
        a->nb_enfants++;
        *h = 0;
        return a;
    }

    if (*h) {
        a->eq += *h;
        a = equilibrer(a);
        *h = (a->eq == 0) ? 0 : 1;
    }

    return a;
}

/* ===============================
   PARCOURS AVL → SOMME ÉQUITABLE
   =============================== */
float sommeEquitable(Arbre* a) {
    if (!a) return 0.0f;

    float contribution = a->fuite / a->nb_enfants;

    return contribution
         + sommeEquitable(a->fg)
         + sommeEquitable(a->fd);
}

/* ===============================
   LIBÉRATION
   =============================== */
void libererAVL(Arbre* a) {
    if (!a) return;
    libererAVL(a->fg);
    libererAVL(a->fd);
    free(a);
}

/* ===============================
   MAIN
   =============================== */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"ID_USINE\"\n", argv[0]);
        return 1;
    }

    char* id_usine = argv[1];
    char line[512];
    Arbre* racine = NULL;
    int h;
    int trouve = 0;

    while (fgets(line, sizeof(line), stdin)) {
        char parent[200], b[100], c[100], d[10];
        float fuite;

        if (sscanf(line, "%199[^;];%99[^;];%99[^;];%9[^;];%f",
                   parent, b, c, d, &fuite) == 5) {

            racine = insererAVL(racine, parent, fuite, &h);
            trouve = 1;
        }
    }

    if (!trouve) {
        printf("-1\n");
        return 0;
    }

    float total_fuite = sommeEquitable(racine);
    float pertes_Mm3 = total_fuite / 1000000.0f;

    printf("Pertes totales pour %s : %.6f M.m3\n", id_usine, pertes_Mm3);

    FILE* f = fopen("pertes_usines.dat", "a");
    if (f) {
        fprintf(f, "%s;%.6f\n", id_usine, pertes_Mm3);
        fclose(f);
    }

    libererAVL(racine);
    return 0;
}
