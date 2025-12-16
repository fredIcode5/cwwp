#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * NOUVELLE STRUCTURE AVL
 * ============================================================ */

typedef struct avl {
    char identifiant[128];   /* ID complet : ex "Facility complex #RH400057F" */
    long vol_max;           /* Volume maximal ou capté (selon l'option) */
    long vol_capte;         /* Volume capté (utilisé pour l'option 'real') */
    float perte;            /* Pourcentage de perte (utilisé pour l'option 'real') */
    struct avl *fg;
    struct avl *fd;
    int eq;
} avl;

/* ============================================================
 * FONCTIONS UTILES (inchangées)
 * ============================================================ */

int max2(int a, int b) { return a > b ? a : b; }
int min2(int a, int b) { return a < b ? a : b; }

int max3(int a, int b, int c) { return max2(a, max2(b, c)); }
int min3(int a, int b, int c) { return min2(a, min2(b, c)); }

/* ============================================================
 * CREATION D'UN NOEUD (Adaptée à la nouvelle structure et l'option)
 * ============================================================ */

// Note : Pour simplifier, nous utilisons vol1 et vol2 pour les deux colonnes de volume/perte
avl* createAVL(const char* id, long vol1, long vol2, float perte, const char* option) {
    avl* p = malloc(sizeof(avl));
    if (!p) exit(1);

    strcpy(p->identifiant, id);
    p->fg = NULL;
    p->fd = NULL;
    p->eq = 0;

    // Initialisation des champs basés sur l'option
    if (strcmp(option, "max") == 0) {
        p->vol_max = vol1; // vol1 est le volume max
        p->vol_capte = 0;
        p->perte = 0.0;
    } else if (strcmp(option, "src") == 0) {
        p->vol_max = 0;
        p->vol_capte = vol1; // vol1 est le volume capté
        p->perte = 0.0;
    } else if (strcmp(option, "real") == 0) {
        p->vol_max = vol1;     // vol1 est le volume max
        p->vol_capte = vol2;     // vol2 est le volume capté
        p->perte = perte; // perte est le pourcentage de perte
    } else {
        // Cas par défaut ou erreur (gestion basique)
        p->vol_max = 0;
        p->vol_capte = 0;
        p->perte = 0.0;
    }

    return p;
}

/* ============================================================
 * ROTATIONS (Inchangées)
 * ============================================================ */

avl* rotateLeft(avl* pRoot) {
    avl* pPivot = pRoot->fd;
    pRoot->fd = pPivot->fg;
    pPivot->fg = pRoot;

    int eqa = pRoot->eq;
    int eqp = pPivot->eq;

    pRoot->eq = eqa - max2(eqp, 0) - 1;
    pPivot->eq = min3(eqa - 2, eqa + eqp - 2, eqp - 1);

    return pPivot;
}

avl* rotateRight(avl* pRoot) {
    avl* pPivot = pRoot->fg;
    pRoot->fg = pPivot->fd;
    pPivot->fd = pRoot;

    int eqa = pRoot->eq;
    int eqp = pPivot->eq;

    pRoot->eq = eqa - min2(eqp, 0) + 1;
    pPivot->eq = max3(eqa + 2, eqa + eqp + 2, eqp + 1);

    return pPivot;
}

avl* doubleRotateLeft(avl* pRoot) {
    pRoot->fd = rotateRight(pRoot->fd);
    return rotateLeft(pRoot);
}

avl* doubleRotateRight(avl* pRoot) {
    pRoot->fg = rotateLeft(pRoot->fg);
    return rotateRight(pRoot);
}

/* ============================================================
 * EQUILIBRAGE (Inchangé)
 * ============================================================ */

avl* balanceAVL(avl* pRoot) {
    if (pRoot->eq >= 2) {
        if (pRoot->fd->eq >= 0)
            pRoot = rotateLeft(pRoot);
        else
            pRoot = doubleRotateLeft(pRoot);
    }
    else if (pRoot->eq <= -2) {
        if (pRoot->fg->eq <= 0)
            pRoot = rotateRight(pRoot);
        else
            pRoot = doubleRotateRight(pRoot);
    }
    return pRoot;
}

/* ============================================================
 * INSERTION (Adaptée à la nouvelle structure et l'option)
 * ============================================================ */

avl* insertAVL(avl* pRoot, const char* id, long vol1, long vol2, float perte, int* h, const char* option) {
    // Note : vol2 et perte ne sont utilisés que si option est 'real'
    if (pRoot == NULL) {
        *h = 1;
        return createAVL(id, vol1, vol2, perte, option);
    }

    int cmp = strcmp(id, pRoot->identifiant);

    if (cmp < 0) {
        pRoot->fg = insertAVL(pRoot->fg, id, vol1, vol2, perte, h, option);
        *h = -*h;
    }
    else if (cmp > 0) {
        pRoot->fd = insertAVL(pRoot->fd, id, vol1, vol2, perte, h, option);
    }
    else {
        // IDENTIFIANT EXISTANT : Mise à jour des données selon l'option
        if (strcmp(option, "max") == 0) {
            // vol1 est le nouveau vol_max pour cet ID
            pRoot->vol_max = vol1;
        } else if (strcmp(option, "src") == 0) {
            // vol1 est un nouveau vol_capte à cumuler
            pRoot->vol_capte += vol1;
        } else if (strcmp(option, "real") == 0) {
            // vol1 est le nouveau vol_max
            pRoot->vol_max = vol1;
            // vol2 est un nouveau vol_capte à cumuler
            pRoot->vol_capte += vol2 * (1 - pRoot->perte/100) ;
            // perte est la nouvelle perte à affecter
            pRoot->perte = perte;
        }

        *h = 0;
        return pRoot;
    }

    if (*h != 0) {
        pRoot->eq += *h;
        pRoot = balanceAVL(pRoot);
        if (pRoot->eq == 0) *h = 0;
        else *h = 1;
    }

    return pRoot;
}

/* ============================================================
 * PARCOURS INFIXE (pour debug - Adapté à la nouvelle structure)
 * ============================================================ */

void infix(avl* p, const char* option) {
    if (p != NULL) {
        infix(p->fg, option);
        if (strcmp(option, "real") == 0) {
             printf("%s;%ld;%ld;%.2f%%\n", p->identifiant, p->vol_max, p->vol_capte, p->perte);
        } else {
             // Pour 'max' ou 'src', on affiche l'ID et un seul volume (le principal)
             long principal_vol = (strcmp(option, "max") == 0) ? p->vol_max : p->vol_capte;
             printf("%s;%ld\n", p->identifiant, principal_vol);
        }
        infix(p->fd, option);
    }
}

/* ============================================================
 * LIBERATION MEMOIRE (Inchangée)
 * ============================================================ */

void libererEspace(avl* p) {
    if (p != NULL) {
        libererEspace(p->fg);
        libererEspace(p->fd);
        free(p);
    }
}

/* ============================================================
 * TRANSFORMATION AVL → TABLEAU POUR TRI (Adaptée)
 * ============================================================ */

// Structure de sortie (adaptée pour l'affichage final)
typedef struct {
    char identifiant[128];
    long vol_max;
    long vol_capte;
    float perte;
} Node;

void avlToArray(avl* p, Node* arr, int* index) {
    if (p != NULL) {
        avlToArray(p->fg, arr, index);
        strcpy(arr[*index].identifiant, p->identifiant);
        arr[*index].vol_max = p->vol_max;
        arr[*index].vol_capte = p->vol_capte;
        arr[*index].perte = p->perte;
        (*index)++;
        avlToArray(p->fd, arr, index);
    }
}

int countIdentifiers(avl* p) {
    if (p == NULL) return 0;
    return 1 + countIdentifiers(p->fg) + countIdentifiers(p->fd);
}

// Fonction de comparaison pour qsort (tri par volume_principal)
int compareVolume(const void* a, const void* b) {
    const Node* x = (const Node*)a;
    const Node* y = (const Node*)b;
    // On trie par vol_max, qui sert de référence principale dans ce contexte
    if (x->vol_max < y->vol_max) return -1;
    if (x->vol_max > y->vol_max) return 1;
    return 0;
}

/* ============================================================
 * MAIN — GESTION DES OPTIONS + LECTURE FICHIER + TRI
 * ============================================================ */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <option>\n", argv[0]);
        fprintf(stderr, "Options valides: max, src, real\n");
        return 1;
    }

    avl* racine = NULL;
    int h;
    char id[128];
    long vol1, vol2 = 0; // vol2 et perte sont à 0 par défaut
    float perte = 0.0;
    char line[512];
    const char* option = argv[1];

    if (strcmp(option, "max") != 0 && strcmp(option, "src") != 0 && strcmp(option, "real") != 0) {
        fprintf(stderr, "Erreur: Option invalide. Utilisez 'max', 'src', ou 'real'.\n");
        return 1;
    }

    printf("Lecture des données avec l'option '%s'...\n", option);

    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(option, "real") == 0) {
            // Format : IDENTIFIANT;VOLUME_MAX;VOLUME_CAPTE;POURCENTAGE_PERTE
            if (sscanf(line, "%127[^;];%ld;%ld;%f", id, &vol1, &vol2, &perte) == 4) {
                 racine = insertAVL(racine, id, vol1, 0, perte, &h, option);
            }
        } else {
            // Format : IDENTIFIANT;VOLUME (pour 'max' et 'src')
            if (sscanf(line, "%127[^;];%ld", id, &vol1) == 2) {
                // vol2 et perte restent à 0.0 par défaut pour 'max'/'src'
                racine = insertAVL(racine, id, vol1, 0, 0.0, &h, option);
            }
        }
    }

    int n = countIdentifiers(racine);
    if (n == 0) {
        printf("Aucun identifiant trouvé.\n");
        return 0;
    }
    
    Node* arr = malloc(n * sizeof(Node));
    int index = 0;

    avlToArray(racine, arr, &index);

    // Tri (ici, le tri est basé sur vol_max pour l'exemple)
    qsort(arr, n, sizeof(Node), compareVolume);

    printf("\n===== CONTENU TRIÉ PAR VOLUME MAX (%s) =====\n", option);
    for (int i = 0; i < n; i++) {
        if (strcmp(option, "real") == 0) {
             printf("%s;%ld;%ld;%.2f%%\n", arr[i].identifiant, arr[i].vol_max, arr[i].vol_capte, arr[i].perte);
        } else if (strcmp(option, "max") == 0) {
             printf("%s;%ld\n", arr[i].identifiant, arr[i].vol_max);
        } else { // src
             printf("%s;%ld\n", arr[i].identifiant, arr[i].vol_capte);
        }
    }

    printf("\nNombre d'identifiants différents : %d\n", n);

    // Écriture du fichier de sortie (Adapté à la nouvelle structure et l'option)
    FILE* fichier = fopen("lst_usine.txt", "w");
    if (!fichier) {
        perror("Erreur ouverture lst_usine.txt");
        free(arr);
        libererEspace(racine);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (strcmp(option, "real") == 0) {
             fprintf(fichier, "%s;%ld;%ld;%.2f%%\n", arr[i].identifiant, arr[i].vol_max, arr[i].vol_capte, arr[i].perte);
        } else if (strcmp(option, "max") == 0) {
             fprintf(fichier, "%s;%ld\n", arr[i].identifiant, arr[i].vol_max);
        } else { // src
             fprintf(fichier, "%s;%ld\n", arr[i].identifiant, arr[i].vol_capte);
        }
    }

    fclose(fichier);
    printf("\nFichier lst_usine.txt généré avec succès (trié par volume) !\n");

    free(arr);
    libererEspace(racine);
    return 0;
}