#ifndef BIBLIO_PROJET_OS_H_INCLUDED
#define BIBLIO_PROJET_OS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define max_chars_par_bloc 10
#define ERROR_OTHER -1
#define ERROR_OPEN -2
#define ERROR_READ -3
#define ERROR_WRITE -4
#define ERROR_LSEEK -5
#define NB_FILES_MAX 1500
#define MAX_LEN_NAME 255

extern int fd;
/*********************************************************************
 |       		Structures de données				|
 ********************************************************************/
/**
 * @struct blocEntete
 * @brief Structure représentant l'en-tête d'un bloc de données.
 * 
 * Cette structure contient les informations relatives à un bloc de données,
 * telles que le nom du fichier, l'offset vers le premier bloc de données du fichier
 * depuis le début de la partition, et le nombre total de blocs de données du fichier.
 */
typedef struct blocEntete{
    char nomFichier[MAX_LEN_NAME]; /**< Le nom du fichier */
    off_t numTete; /**< L'offset vers le premier bloc de données du fichier */
    int nbBlocs; /**< Le nombre total de blocs de données du fichier */
}blocEntete;


/**
 * @struct blocData
 * @brief Structure représentant un bloc de données.
 * 
 * Cette structure contient les informations relatives à un bloc de données.
 * Chaque bloc peut contenir un nombre maximum de "b" données (enregistrements).
 * 
 * @var blocData::nbChars
 * Nombre de caractères présents actuellement dans le bloc.
 * 
 * @var blocData::donnee
 * Tableau de caractères contenant les données du bloc.
 * 
 * @var blocData::suiv
 * Offset vers le bloc de données suivant depuis le début de la partition.
 */
typedef struct blocData{
    int nbChars; //nombre de chars presents actuellement dans le bloc
    char donnee[max_chars_par_bloc]; //un bloc contient au maximum "b" données (enregistrements)
    off_t suiv; //offset vers le bloc de donnees suivant depuis le debut de la partititon
} blocData;



/**
 * @struct file
 * @brief Structure représentant un fichier.
 * 
 * Cette structure contient les informations nécessaires pour gérer un fichier.
 */
typedef struct file{
    int fd; /**< Descripteur de fichier = descripteur de la partition */
    int pos; /**< Pointeur de lecture/écriture */
    off_t numEntete; /**< Offset vers le bloc d'entête du fichier depuis le début de la partition */
}file;


/**
 * @struct elemTabIndex
 * @brief Structure représentant un élément du tableau d'index.
 */
typedef struct elemTabIndex{
    char nomFichier[MAX_LEN_NAME]; /**< Le nom du fichier, le tableau sera ordonné selon ce champ */
    off_t numBlocEntete; /**< Offset vers le bloc d'entête du fichier depuis le début de la partition */
}elemTabIndex;


/**
 * @struct blocIndex
 * @brief Structure représentant un bloc d'index.
 */
typedef struct blocIndex{
    int nbFichiers; /**< Le nombre total de fichiers dans la partition, équivalent au nombre d'éléments présents dans le tableau d'index. */
    elemTabIndex tabIndex[NB_FILES_MAX]; /**< Le tableau d'index contenant les éléments de l'index. */
}blocIndex;


/*********************************************************************
 |       		Prototypes fonctions				|
 ********************************************************************/

/**************************************HELPERS****************************************/

blocData alloc_bloc(); //INITIALISER UN BLOC VIDE
blocIndex init_blocIndex();
int estPleinBlocData(blocData* bloc);
int trouverBlocData(int positionActuelle, int nbCharMaxParBloc);
int trouverPosition(int positionActuelle, int nbCharMaxParBloc);
off_t trouveOffsetBlocFile(file* f, int blocNumber);
int rechercheDichotomique(elemTabIndex* tableau, int taille, char* nomFichier);
int insertionTableauTrie(elemTabIndex* tableau, int taille, elemTabIndex element);
int getPosLastCharFile(file* f);

//MANIPULATION D'ENTETE
//////getters
int getNbBlocsFile(file* f);
off_t getNumTeteFile(file* f);
////// setters
int setNbBlocsFile(file* f, int val);
int setNumTeteFile(file* f, off_t val);

//TAILLE D'UN FICHIER
int size(file* f); //en termes de l'espace totale alloué par les blocs
int getSizeReelFile(file* f); //en terme de données (nombre de caracteres)





/***********************************************************************************************/
/*                          FONCTIONS PRINCIPALES                                              */
/***********************************************************************************************/
//myFormat
int myFormat(char* partitionName);

//myOpen
file* myOpen(char* fileName);

//myWrite
int myWrite(file* f, void* buffer, int size);

//MyRead
int myRead(file *f,void * buffer, int nBytes);

//MySeek
void mySeek(file* f, int offset, int base);

//MyClose
void myClose(file* f);

//closePartition
int closePartition(int fd);
#endif // BIBLIO_PROJET_OS_H_INCLUDED
