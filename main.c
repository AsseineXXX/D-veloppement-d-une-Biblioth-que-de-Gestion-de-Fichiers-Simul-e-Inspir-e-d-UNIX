#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "BIBLIO_PROJET_OS.h"

/*******************************************************************/

int main()
{
    fd=-1;
    int action;
    file* f= malloc(sizeof(file)); //va contenir le fichier
    char fileName[MAX_LEN_NAME]; //va contenir le nom du fichier recemment ouvert
    int nbBytes; //stockes le nombre d'octets lu/ecrits
    char partitionName[MAX_LEN_NAME];
    printf("-----------------L'execution a commencé, Bienvenue dans notre programme !\n");
    while (true) {
       printf("Bonjour! Veuillez choisir l'action à effectuer :\n"
           "0-Formattage de la partition\n"
           "1-Ouverture d'un fichier\n"
           "2-Ecriture dans un fichier\n"
           "3-Lecture d'un fichier\n"
           "4-Deplacement dans un fichier\n"
           "5-Quitter le programme\n");

        scanf("%d", &action);

        switch (action) {
            case 0:
                printf("\033[2J\033[H");
                puts("\nBienvenue sur MyFormat !! Vous allez formater votre partition.\n");
                printf("Veuillez saisir le nom de la partition à formatter :\n");
                getchar(); //effacer le buffer de lecture
                fgets(partitionName, sizeof(partitionName), stdin);
                partitionName[strcspn(partitionName, "\n")] = '\0';
                printf("Nom de la partition : %s\n",partitionName);
                if (myFormat(partitionName)==-1) {
                    printf("\nErreur myFormat..");
                    exit(ERROR_OTHER);
                }
                 printf("FIN formattage\n*--------------------------******--------------------------------*\n");
                break;
            case 1:
                printf("\033[2J\033[H");
                if (fd==-1 || fd==-1) {
                    printf("! Veuillez formatter d'abord une partition ! \n");
                    break;
                }
                printf("\n Bienvenue dans MyOpen !! vous allez ouvrir un fichier contenu dans la partition formattée actuellement.\n");
                printf("Veuillez saisir le nom du fichier à ouvrir : ");
                getchar(); //effacer le buffer de lecture
                fgets(fileName,sizeof(fileName),stdin);
                fileName[strcspn(fileName, "\n")] = '\0';
                f=myOpen(fileName);
                if (f==NULL){
                    printf("\nErreur myOpen..");
                    exit(ERROR_OTHER);
                }
                printf("Fichier ouvert : \n");
                printf("* Nom du fichier : %s\n",fileName);
                printf("* Taille du fichier (sans comptabiliser les vides et les meta données) : %d\n",getSizeReelFile(f));
                printf("* Taille totale ocupée par les blocs de données du fichier : %d\n",size(f));
                printf("* Position actuelle dans le fichier : %d\n",f->pos);
                printf("* Deplacement vers le bloc d'entete : %ld\n",f->numEntete);
                printf("FIN ouverture\n*--------------------------******--------------------------------*\n");
                break;
            case 2:
                printf("\033[2J\033[H");
                if (f==NULL || fd==-1) {
                    printf("! Impossible d'écrire, veuillez ouvrir d'abord un fichier !\n");
                    break;
                }
                printf("\n Bienvenue dans MyWrite !! vous allez ecrire dans le fichier '%s' que vous avez recemment ouvert ! \n",fileName);
                printf("Veuillez saisir la chaine de caractère à écrire : ");
                void* donneeWrite = (void*)malloc(2500);
                getchar(); //effacer le buffer de lecture
                fgets((char*)donneeWrite,2500,stdin);
                ((char*)donneeWrite)[strcspn((char*)donneeWrite, "\n")] = '\0';
                printf("* Donnée saisie : %s\n", (char*)donneeWrite);
                printf("* Nombre d'octets à écrire : %ld\n",strlen((char*)donneeWrite));
                printf("Écriture en cours ...\n");
                nbBytes=myWrite(f,donneeWrite,strlen((char*)donneeWrite));
                printf("* Nombre d'octets ecrits : %d\n",nbBytes);
                printf("* Position actuelle dans le fichier : %d\n",f->pos);
                free(donneeWrite); //liberer espace memoire
                printf("FIN écriture\n*--------------------------******--------------------------------*\n");
                break;
            case 3:
                printf("\033[2J\033[H");
                if (f==NULL || fd==-1) {
                    printf("! Impossible de lire, veuillez ouvrir d'abord un fichier !\n");
                    break;
                }
                getchar(); //effacer le buffer de lecture
                printf("\nBienvenue dans MyRead !! vous allez lire des données du fichier '%s' que vous avez recemment ouvert ! \n",fileName);
                printf("Veuillez saisir la nombre d'octets à lire : ");
                int nbOcts;
                scanf("%d",&nbOcts);
                printf("* Nombre d'octets à lire : %d\n",nbOcts);
                printf("Lecture en cours ...\n");
                void* donneeRead=(void*)malloc(nbOcts);
                nbBytes=myRead(f,donneeRead,nbOcts);
                //affichage
                printf("-Donnée lue : \n");
                for (int i=0;i<nbOcts;i++){
                    printf("%c",((char*)donneeRead)[i]);
                }
                printf("\n\n* %d octets lus avec succés\n",nbOcts);
                printf("* Position actuelle dans le fichier : %d\n",f->pos);
                free(donneeRead); //liberer espace memoire
                printf("FIN lecture\n*--------------------------******--------------------------------*\n");
                break;
            case 4:
                printf("\033[2J\033[H");
                getchar(); //effacer le buffer de lecture
                if (f==NULL || fd==-1) {
                    printf("! Impossible de se deplacer, veuillez ouvrir d'abord un fichier !\n");
                    break;
                }
                int deplacement;
                int base;
                printf("\n Bienvenue dans MySeek !! vous allez vous deplacer à l'interieur du fichier '%s' que vous avez recemment ouvert ! \n",fileName);
                printf("- La position actuelle dans le fichier est : %d\n",f->pos);
                printf("\nVeuillez introduire les paramètres suivants : \n");
                printf("* Base (valeurs possibles: 0:SEEK_SET, 1:SEEK_CUR, 2:SEEK_END) : ");
                scanf("%d",&base);
                printf("\n* Déplacement : ");
                scanf("%d",&deplacement);
                printf("\n\nVous avez introduit les paramètres suivants :");
                printf("\nbase = %d | déplacement = %d \n",base,deplacement);
                printf("Deplacement en cours ...\n");
                mySeek(f,deplacement,base);
                printf("Deplacement effectué ! \n* Position courrante : %d\n\n",f->pos);
                printf("FIN MySeek\n*--------------------------******--------------------------------*\n");
                break;
            case 5:
                printf("\033[2J\033[H");
            //quitter le programme
                printf("\nVous allez quitter le programme.. À bientôt! ");
                //liberer espace
                myClose(f); //fermer le fichier
                closePartition(f->fd);
                //quitter
                exit(0);
            default:
                break;
        }
    }
    return 0;
}
