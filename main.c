#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCS 1000
#define BLOC_SIZE 512

typedef struct {
    int vide;
    int blocSuiv; // Utilise pour l'organisation chainee
} Bloc;
typedef struct {
    int id;      // Identifiant de l'enregistrement
    char data[100];     // Donnees de l'enregistrement
} Enrg;


typedef struct {
    char nom[50];
    int num_blocs;
    int num_Enrg;
    int premierBloc;
    int OrgGlobale; // 0 pour contigue 1 pour chaine
    int OrgInternal; // 0 pour non triee 1 pour triee
} Metadata;

Bloc MS[MAX_BLOCS];
Metadata metadata[MAX_BLOCS];
int bloc_count;
int bloc_factor;
void initialiserMS(int num_blocs, int bloc_size) {
    bloc_count = num_blocs;
    bloc_factor = bloc_size;

    for (int i = 0; i < bloc_count; i++) {
        MS[i].vide = 1;
        MS[i].blocSuiv = -1;
    }
    printf("La ms est initialisée par %d bloc de taille %d\n", bloc_count, bloc_factor);
}

void afficherCle(char *file_name) {
    // Recherche du fichier dans les meta
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Fichier %s non trouve.\n", file_name);
        return;
    }

    // Recuperer les information du fichier
    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;

    printf("cles des enregistrements dans le fichier %s:\n", file_name);

    // Parcourir les blocs et afficher les IDs d'enregistrements
    while (current_bloc != -1) {
        printf("Bloc %d:\n", current_bloc);
        for (int i = 0; i < BLOC_SIZE / sizeof(int); i++) {
            int *record = (int *)(&MS[current_bloc] + i * sizeof(int));
            if (*record != -1) { // Afficher uniquement les enregistrements non supprimés
                printf("ID d'enregistrement: %d\n", *record);
            }
        }
        current_bloc = MS[current_bloc].blocSuiv; // Passer au bloc suivant
    }
}


void insererEnrg(char *file_name, int Enrg_id, char *data) {
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Le fichier %s", file_name,"non trouvee");
        return;
    }

    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;

    while (current_bloc != -1) {
        for (int i = 0; i < BLOC_SIZE / sizeof(int); i++) {
            int *Enrg = (int *)(&MS[current_bloc] + i * sizeof(int));
            if (*Enrg == -1) {
                *Enrg = Enrg_id;
                strcpy((char *)(Enrg + 1), data);
                printf("L'enregistrement ID %d est inseree dans le fichier  %s.\n", Enrg_id, file_name);
                return;
            }
        }
        current_bloc = MS[current_bloc].blocSuiv;
    }

    printf("Espace isuffisant pour inserer l'enregistrement ID %d in file %s.\n", Enrg_id, file_name);
}
//modifier un enregistrement
void MAJEnrg(char *file_name, int Enrg_id, char *nouveau_data) {
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Fichier non trouvee");
        return;
    }

    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;
    int Enrg_trouve = 0;

    while (current_bloc != -1) {
        for (int i = 0; i < BLOC_SIZE / sizeof(int); i++) {
            int *Enrg = (int *)(&MS[current_bloc] + i * sizeof(int));
            if (*Enrg == Enrg_id) {
                strcpy((char *)(Enrg + 1), nouveau_data);
                printf("L'enregisterement ID %d mise a jour dans les fichier %s.\n", Enrg_id, file_name);
                Enrg_trouve = 1;
                break;
            }
        }


        current_bloc = MS[current_bloc].blocSuiv;
    }

    if (!Enrg_trouve) {
        printf("L'enregistrement non trouvee.\n");
    }
}
//fonction pour supprimer un enregistrement logiquement
void SuppressionLgiqueEnrg(char *file_name, int Enrg_id) {
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Fichier non trouvee\n");
        return;
    }

    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;
    int Enrg_trouve = 0;

    while (current_bloc != -1) {
        for (int i = 0; i < BLOC_SIZE / sizeof(int); i++) {
                        int *Enrg = (int *)(&MS[current_bloc] + i * sizeof(int));
            if (*Enrg == Enrg_id) {
                *Enrg = -1;
                printf("L'enregistrement  ID %d est logiquement supprimer de fichier %s.\n", Enrg_id, file_name);
                Enrg_trouve = 1;
                break;
            }
        }


        current_bloc = MS[current_bloc].blocSuiv;
    }

    if (!Enrg_trouve) {
        printf("Enregistrement non trouve dans le fichier\n");
    }
}


void creerFichier(char *name, int num_Enrg, int global_org, int internal_org) {
    int blocBesoin= (num_Enrg * sizeof(int) + BLOC_SIZE - 1) / BLOC_SIZE;

    int premierBloc = -1;
    int blocPrecedent = -1;

    for (int i = 0; i < bloc_count; i++) {
        if (MS[i].vide) {
            if (premierBloc  == -1) {
                premierBloc  = i;
            } else {
                MS[blocPrecedent].blocSuiv = i;
            }

            MS[i].vide = 0;
            blocPrecedent = i;
            blocBesoin=blocBesoin-1;

            if (blocBesoin == 0) break;
        }
    }

    if (blocBesoin > 0) {
        printf("Espace insuffisant pour creer ce fichier.\n");
        return;
    }

    Metadata meta;
    strcpy(meta.nom, name);
    meta.num_blocs = blocBesoin;
    meta.num_Enrg = num_Enrg;
    meta.premierBloc = premierBloc;
    meta.OrgGlobale = global_org;
    meta.OrgInternal = internal_org;

    // Enregistrer les meta
    metadata[premierBloc] = meta;
    printf("le fichier %s est cree avec %d d'enregistrements .\n", name, num_Enrg);
}
//cette fonction supprime un fichie deffinitivement de la ms
void SupprimerFichier(char *name) {
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Le fichier avec le nom: %s non trouvee!\n", name);
        return;
    }

    Metadata *file_meta = &metadata[file_index];
    int blocCourrant = file_meta->premierBloc;

    while (blocCourrant != -1) {
        int blocSuiv = MS[blocCourrant].blocSuiv;
        MS[blocCourrant].vide = 1;
        blocCourrant = blocSuiv;
    }

    memset(&metadata[file_index], 0, sizeof(Metadata));
    printf("Le fichier %s est suprime de la MS.\n", name);
}
//fonction pour renommer un fichier existant
void renommerFichier(char *ancien, char *nouveau) {
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, ancien) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("le fichier %s non trouvé .\n", ancien);
        return;
    }

    strcpy(metadata[file_index].nom, nouveau);
    printf("le nom de fichier %s est devenu %s.\n", ancien, nouveau);
}

//cette fonction affiche l'etat de la ms
void EtatMS() {
    printf("L'etat de la memoire secondaire :\n");
    for (int i = 0; i < bloc_count; i++) {
        if (MS[i].vide) {
            printf("Bloc %d: libre\n", i);
        } else {
            printf("Bloc %d: Occupee (File: %s)\n", i, metadata[i].nom);
        }
    }
}

void defragmentationFichier(char *name) {
    // Recherche du fichier dans les meta
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Fichier  %s non trouvé.\n", name);
        return;
    }

    // Recupérer les informations du fichier
    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;
    int prev_bloc = -1;
    int new_bloc_index = 0;

    while (current_bloc != -1) {
        if (new_bloc_index != current_bloc) {
            // Copier les donnees du bloc actuel au nouveau bloc
            memcpy(&MS[new_bloc_index], &MS[current_bloc], sizeof(Bloc));
            MS[current_bloc].vide = 1; // Marquer l'ancien bloc comme libre
            MS[new_bloc_index].vide = 0;

            // Mise a jour des pointeurs pour l'organisation chainee
            if (prev_bloc != -1) {
                MS[prev_bloc].blocSuiv = new_bloc_index;
            } else {
                file_meta->premierBloc = new_bloc_index;
            }
            prev_bloc = new_bloc_index;
        }

        current_bloc = MS[current_bloc].blocSuiv;
        new_bloc_index++;
    }

    // Marquer la fin de la liste
    if (prev_bloc != -1) {
        MS[prev_bloc].blocSuiv = -1;
    }

    printf("Fichier %s defragmenté.\n", name);
}
//la recherche dans un seul fichier
void chercherEnrg(char *file_name, int Enrg_id) {
    // Recherche du fichier dans les meta
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Filchier %s non trouvé.\n", file_name);
        return;
    }

    // Recuperer les informations du fichier
    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;
    int Enrg_trouve = 0;

    while (current_bloc != -1) {
        // Parcourir chaque enregistrement dans le bloc
        for (int i = 0; i < BLOC_SIZE / sizeof(int); i++) {
            int *Enrg = (int *)(&MS[current_bloc] + i * sizeof(int));
            if (*Enrg == Enrg_id) {
                printf("L'enregistrement ID %d est trouve dans le bloc %d à la position %d.\n", Enrg_id, current_bloc, i);
               Enrg_trouve = 1;
                break;
            }
        }
        if (Enrg_trouve) break;

        current_bloc = MS[current_bloc].blocSuiv; // Passer au bloc suivant si l'organisation chainee
    }

    if (!Enrg_trouve) {
        printf("enregistrement ID %d non trouve dans le fichier  %s.\n", Enrg_id, file_name);
    }
}

//la recherche dans tout les fichier
void RechercheGlobale(int Enrg_id) {
    int Enrg_trouve = 0;

    for (int i = 0; i < bloc_count; i++) {
        if (!MS[i].vide) {
            Metadata *file_meta = &metadata[i];
            int current_bloc = file_meta->premierBloc;

            while (current_bloc != -1) {
                // Parcourir chaque enregistrement dans le bloc
                for (int j = 0; j < BLOC_SIZE / sizeof(int); j++) {
                    int *Enrg = (int *)(&MS[current_bloc] + j * sizeof(int));
                    if (*Enrg == Enrg_id) {
                        printf("Enregistrement ID %d trouve dans le fichier %s, bloc %d, position %d.\n", Enrg_id, file_meta->nom, current_bloc, j);
                        Enrg_trouve = 1;
                        break;
                    }
                }
                if (Enrg_trouve) break;

                current_bloc = MS[current_bloc].blocSuiv; // Passer au bloc suivant pour l'organisation chainee
            }
        }

    }

    if (!Enrg_trouve) {
        printf("Enregistrement ID %d non trouve dans la memoire secondaire.\n", Enrg_id);
    }
}
//compacter la ms
void compact_memory() {
    int new_bloc_index = 0;

    for (int i = 0; i < bloc_count; i++) {
        if (!MS[i].vide) {
            Metadata *file_meta = &metadata[i];
            int current_bloc = file_meta->premierBloc;
            int prev_bloc = -1;

            while (current_bloc != -1) {
                if (new_bloc_index != current_bloc) {
                    // Copier les donnes du bloc actuel au nouveau bloc
                    memcpy(&MS[new_bloc_index], &MS[current_bloc], sizeof(Bloc));
                    MS[current_bloc].vide = 1;
                    MS[new_bloc_index].vide = 0;

                    // Mise a jour des pointeur pour l'organisation chaine
                    if (prev_bloc != -1) {
                        MS[prev_bloc].blocSuiv = new_bloc_index;
                    } else {
                        file_meta->premierBloc = new_bloc_index;
                    }
                    prev_bloc = new_bloc_index;
                }

                current_bloc = MS[current_bloc].blocSuiv;
                new_bloc_index++;
            }
        }
    }

    // Marquer les blocs restants comme libres
    for (int i = new_bloc_index; i < bloc_count; i++) {
        MS[i].vide = 1;
        MS[i].blocSuiv = -1;
    }

    printf("La memoire est compacté avec succés .\n");
}

//fonction pour vider la ms
void ViderMemoire() {
    for (int i = 0; i < bloc_count; i++) {
        MS[i].vide = 1;
        MS[i].blocSuiv = -1;
    }

    for (int i = 0; i < bloc_count; i++) {
        memset(&metadata[i], 0, sizeof(Metadata));
    }

    printf("la memoire est vide.\n");
}

//cette fonction iffiche les information des fichiers existant dans la ms
void AfficherFichiers() {
    printf("Liste des fichiers dans le système:\n");

    for (int i = 0; i < bloc_count; i++) {
        if (metadata[i].premierBloc != -1) {
               printf("Nom du fichier%s\n" ,metadata[i].nom);
                printf("Nombre de blocs%d\n",metadata[i].num_blocs);
                printf("Nombre d'enregistrements %d\n",metadata[i].num_Enrg);
               printf("Premier bloc%d\n", metadata[i].premierBloc);
        }
    }
}

//fonction pour afficher le contenu d'un fichier
void ContenuFichier(char *file_name) {
    // Recherche du fichier dans les meta
    int file_index = -1;
    for (int i = 0; i < bloc_count; i++) {
        if (strcmp(metadata[i].nom, file_name) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printf("Fichier %s non trouve.\n", file_name);
        return;
    }

    // Recuperer les informations du fichier
    Metadata *file_meta = &metadata[file_index];
    int current_bloc = file_meta->premierBloc;

    printf("Contenu du fichier %s:\n", file_name);

    while (current_bloc != -1) {
        printf("Bloc %d:\n", current_bloc);
        // Parcourir chaque enregistrement dans le bloc
        for (int i = 0; i < BLOC_SIZE / sizeof(int); i++) {
            int *Enrg = (int *)(&MS[current_bloc] + i * sizeof(int));
            if (*Enrg != -1) { // Afficher seulement les enregistrements existee;
                printf("Enregistrement %d: %d\n", i, *Enrg);
            }
        }
        current_bloc = MS[current_bloc].blocSuiv; // Passer au bloc suivant pour l'organisation chainee
    }
}


int main() {
     int choice;
    while (1) {
        printf("\nMenu:\n");
        printf("1. Initializser la mémoire secondaire\n");
        printf("2. Créer un fichier\n");
        printf("3. Afficher l'etat de la MS\n");
        printf("4. Afficher les fchier existant dans le systeme\n");
        printf("5. Afficher le contenu d'un fichier\n");
        printf("6. Inserer un enregistrement\n");
        printf("7. Modifier un enregistrement\n");
        printf("8. Supprimer un enregistrement logiquement\n");
        printf("9. Supprimer un fichier \n");
        printf("10. Renommer un fichier\n");
        printf("11. Defragmenter un fichier\n");
        printf("12. Compacter la MS\n");
        printf("13. Vider la MS\n");
        printf("14. Rechercher dans la MS\n");
        printf("15. Afficher les ID des enregistrements d'un fichier\n");
        printf("16. Quitter\n");
        printf("Entrez votre choix: ");
        scanf("%d", &choice);

        if (choice == 1) {
            int num_blocks, block_size;
            printf("Entrez le nombre de blocs: ");
            scanf("%d", &num_blocks);
            printf("Entrez la taille du bloc: ");
            scanf("%d", &block_size);
            initialiserMS(num_blocks, block_size);
        } else if (choice == 2) {
            char name[50];
            int num_records, global_org, internal_org;
            printf("Entrez le nom de fichier: ");
            scanf("%s", name);
            printf("Entrez le nombre d'enregistrement  : ");
            scanf("%d", &num_records);
            printf("Mode d'organisation globale (0  contigue, 1 chainée): ");
            scanf("%d", &global_org);
            printf("Mode d'organisation intern (0 non triee, 1 triee): ");
            scanf("%d", &internal_org);
            creerFichier(name, num_records, global_org, internal_org);
        } else if (choice == 3) {
            EtatMS();
        } else if (choice == 4) {
            AfficherFichiers();
        } else if (choice == 5) {
            char file_name[50];
            printf("Entrez le nom de fichier: ");
            scanf("%s", file_name);
            ContenuFichier(file_name);
        } else if (choice == 6) {
            char file_name[50], data[100];
            int record_id;
            printf("Entrez le nom de fichier: ");
            scanf("%s", file_name);
            printf("Entrez la clé  de l'enregistrement: ");
            scanf("%d", &record_id);
            printf("Entez le donnees: ");
            scanf("%s", data);
            insererEnrg(file_name, record_id, data);
        } else if (choice == 7) {
            char file_name[50], new_data[100];
            int record_id;
            printf("Entrez le nom de fichier: ");
            scanf("%s", file_name);
            printf("Entrez la clé de l'enregistrement a modifier: ");
            scanf("%d", &record_id);
            printf("Entrez les nouveaux details: ");
            scanf("%s", new_data);
            MAJEnrg(file_name, record_id, new_data);
        } else if (choice == 8) {
            char file_name[50];
            int record_id;
            printf("Entez le nom de fichier: ");
            scanf("%s", file_name);
            printf("Entez la clé de l'enregistrement: ");
            scanf("%d", &record_id);
            SuppressionLgiqueEnrg(file_name, record_id);
        } else if (choice == 9) {
            char name[50];
            printf("Entrez le nom de fichier: ");
            scanf("%s", name);
            SupprimerFichier(name);
        } else if (choice == 10) {
            char old_name[50], new_name[50];
            printf("Entrez le nom courrant de fichier: ");
            scanf("%s", old_name);
            printf("Entrez le nouveau nom : ");
            scanf("%s", new_name);
            renommerFichier(old_name, new_name);
        } else if (choice == 11) {
            char file_name[50];
            printf("Entrez le nom de fichier : ");
            scanf("%s", file_name);
            defragmentationFichier(file_name);
        } else if (choice == 12) {
            compact_memory();
        } else if (choice == 13) {
            ViderMemoire();
        } else if (choice == 14) {
            int record_id;
            printf("Entrez la cle de l'enregistrement pour le chercher: ");
            scanf("%d", &record_id);
            RechercheGlobale(record_id);
        }else if(choice==15){
              printf("Entrez le nom de fichier :");
              char nom[50];
              scanf("%s", nom);
              afficherCle(nom);

        } else if (choice == 16) {
            break;
        } else {
            printf("Choix invalide.\n");
        }
    }

    return 0;
}

