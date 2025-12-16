#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * STRUCTURE AVL
 * ============================================================ */

typedef struct avl {
    char identifiant[128];
    long vol_max;           /* UTILISATION: Volume Brut du dernier enregistrement pour 'real' */
    long vol_capte;         /* UTILISATION: Volume Net cumulé pour 'real' */
    float perte;            /* UTILISATION: Pourcentage de perte du dernier enregistrement */
    struct avl *fg;
    struct avl *fd;
    int eq;
} avl;

/* ============================================================
 * PROTOTYPES (Pour éviter les avertissements et erreurs)
 * ============================================================ */
void freeAVL(avl* p);

/* ============================================================
 * FONCTIONS UTILES (inchangées)
 * ============================================================ */

int max2(int a, int b) { return a > b ? a : b; }
int min2(int a, int b) { return a < b ? a : b; }

int max3(int a, int b, int c) { return max2(a, max2(b, c)); }
int min3(int a, int b, int c) { return min2(a, min2(b, c)); }

/* ============================================================
 * CRÉATION D'UN NOEUD (Adaptée pour 'real' en 3 colonnes)
 * ============================================================ */
// vol1 = Vol Brut (col 2) | perte = Taux (col 3)
avl* createAVL(const char* id, long vol1, long vol2, float perte, const char* option) {
    avl* p = malloc(sizeof(avl));
    if (!p) exit(1);

    strcpy(p->identifiant, id);
    p->fg = NULL;
    p->fd = NULL;
    p->eq = 0;

    if (strcmp(option, "real") == 0) {
        // Calcul sécurisé du Volume Net
        double perte_decimale = (double)perte / 100.0;
        long vol_net_calcule = (long)((double)vol1 * (1.0 - perte_decimale)); 

        p->vol_max = vol1;         // Vol Brut (pour la sortie)
        p->vol_capte = vol_net_calcule; // Vol Net
        p->perte = perte;          // Taux de Perte
    } else if (strcmp(option, "max") == 0) {
        p->vol_max = vol1;
        p->vol_capte = 0;
        p->perte = 0.0;
    } else if (strcmp(option, "src") == 0) {
        p->vol_max = 0;
        p->vol_capte = vol1;
        p->perte = 0.0;
    } else {
        p->vol_max = 0;
        p->vol_capte = 0;
        p->perte = 0.0;
    }

    return p;
}

/* ============================================================
 * ROTATIONS ET EQUILIBRAGE (Inchangés)
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
 * INSERTION (Corrigée pour 'real' en 3 colonnes)
 * ============================================================ */
// vol1 = Vol Brut (col 2) | vol2 est 0 | perte = Taux (col 3)
avl* insertAVL(avl* pRoot, const char* id, long vol1, long vol2, float perte, int* h, const char* option) {
    if (pRoot == NULL) {
        *h = 1;
        // On passe vol2=0 pour 'real'
        return createAVL(id, vol1, 0, perte, option);
    }

    int cmp = strcmp(id, pRoot->identifiant);

    if (cmp < 0) {
        pRoot->fg = insertAVL(pRoot->fg, id, vol1, 0, perte, h, option);
        *h = -*h;
    }
    else if (cmp > 0) {
        pRoot->fd = insertAVL(pRoot->fd, id, vol1, 0, perte, h, option);
    }
    else {
        // IDENTIFIANT EXISTANT : Mise à jour des données
        if (strcmp(option, "max") == 0) {
            pRoot->vol_max = vol1;
        } else if (strcmp(option, "src") == 0) {
            pRoot->vol_capte += vol1;
        } else if (strcmp(option, "real") == 0) {
            // Mise à jour du Volume Brut (vol_max) avec la dernière valeur lue
            pRoot->vol_max = vol1;
            
            // Mise à jour de la Perte avec la dernière valeur lue
            pRoot->perte = perte;

            // Calcul et cumul du Volume Net
            double perte_decimale = (double)perte / 100.0;
            long vol_net_calcule = (long)((double)vol1 * (1.0 - perte_decimale)); 
            
            pRoot->vol_capte += vol_net_calcule; 
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
 * LIBÉRATION DE LA MÉMOIRE (freeAVL remplace libererEspace)
 * ============================================================ */

void freeAVL(avl* p) {
    if (p != NULL) {
        freeAVL(p->fg);
        freeAVL(p->fd);
        free(p);
    }
}

/* ============================================================
 * TRANSFORMATION AVL → TABLEAU POUR TRI
 * ============================================================ */

typedef struct {
    char identifiant[128];
    long vol_max;       // Vol Brut du dernier enregistrement
    long vol_capte;     // Vol Net cumulé
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

// Fonction de comparaison pour qsort
int compareVolume(const void* a, const void* b) {
    const Node* x = (const Node*)a;
    const Node* y = (const Node*)b;
    // Tri basé sur vol_max (Vol Brut)
    if (x->vol_max < y->vol_max) return -1;
    if (x->vol_max > y->vol_max) return 1;
    return 0;
}

/* ============================================================
 * MAIN
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
    long vol1; 
    float perte; 
    char line[512];
    const char* option = argv[1];

    if (strcmp(option, "max") != 0 && strcmp(option, "src") != 0 && strcmp(option, "real") != 0) {
        fprintf(stderr, "Erreur: Option invalide. Utilisez 'max', 'src', ou 'real'.\n");
        return 1;
    }

    // printf("Lecture des données avec l'option '%s'...\n", option);

    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(option, "real") == 0) {
            // Lecture de 3 champs : ID;VOL_BRUT;PERTE%
            if (sscanf(line, "%127[^;];%ld;%f", id, &vol1, &perte) == 3) {
                 racine = insertAVL(racine, id, vol1, 0, perte, &h, option);
            }
        } else {
            // Lecture de 2 champs : ID;VOLUME (pour 'max' et 'src')
            if (sscanf(line, "%127[^;];%ld", id, &vol1) == 2) {
                racine = insertAVL(racine, id, vol1, 0, 0.0, &h, option);
            }
        }
    }

    int n = countIdentifiers(racine);
    if (n == 0) {
        printf("Aucun identifiant trouvé.\n");
        freeAVL(racine);
        return 0;
    }
    
    Node* arr = malloc(n * sizeof(Node));
    if (!arr) {
        perror("Erreur d'allocation tableau");
        freeAVL(racine);
        return 1;
    }
    int index = 0;

    avlToArray(racine, arr, &index);
    qsort(arr, n, sizeof(Node), compareVolume);

    // Écriture du fichier de sortie
    FILE* fichier = fopen("lst_usine.txt", "w");
    if (!fichier) {
        perror("Erreur ouverture lst_usine.txt");
        free(arr);
        freeAVL(racine);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (strcmp(option, "real") == 0) {
             // Sortie demandée : ID; VOLUME_NET_CUMULÉ; VOLUME_BRUT
             fprintf(fichier, "%s;%ld;%ld\n", arr[i].identifiant, arr[i].vol_capte, arr[i].vol_max);
        } else if (strcmp(option, "max") == 0) {
             fprintf(fichier, "%s;%ld\n", arr[i].identifiant, arr[i].vol_max);
        } else { // src
             fprintf(fichier, "%s;%ld\n", arr[i].identifiant, arr[i].vol_capte);
        }
    }

    fclose(fichier);
    
    // Nettoyage final
    free(arr);
    freeAVL(racine);
    
    // Le script menu.sh contient déjà le message de succès

    return 0;
}