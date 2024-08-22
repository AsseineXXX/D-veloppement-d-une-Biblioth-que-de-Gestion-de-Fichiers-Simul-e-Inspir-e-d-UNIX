#include "BIBLIO_PROJET_OS.h"
#include <string.h>
int fd;
/*************************************HELPERS********************************/


/**
 * @file BIBLIO_PROJET_OS.c
 * @brief Ce fichier contient des fonctions de gestion de fichiers simulant celle d’UNIX.
 * Les fonctions suivantes permettent d'effectuer les opérations : formattage d'une partition, la création, la lecture / écriture, la recherche d’informations dans un fichier ainsi que sa destruction.. ainsi que d'autres fonctions secondaires telles que : allocation de blocs, recherche de l'offset d'un bloc de données dans la partition, effectuer une recherche dichotomique dans le tableau d'index, insertion triée ...etc
 */

/**
 * @brief Alloue un bloc de données.
 *
 * Cette fonction initialise un buffer de données et le retourne en sortie.
 * Le champ suiv est initialisé à -1 pour indiquer qu'il n'y a pas de bloc suivant.
 * Le champ nbChars est initialisé à 0.
 * Le tableau de données est initialisé avec des caractères nuls.
 *
 * @return Le bloc de données initialisé.
 */
blocData alloc_bloc() {
    struct blocData buf;
    buf.suiv = -1;
    buf.nbChars = 0;

    for (int i = 0; i < max_chars_par_bloc; i++) {
        buf.donnee[i] = '\0';
    }

    return buf;
}



/**
 * @brief Initialise une structure blocIndex.
 * @return La structure blocIndex initialisée.
 */
blocIndex init_blocIndex() {
    blocIndex bi;

    bi.nbFichiers=0;
}


 /**
 * @brief Vérifie si une structure blocData est pleine.
 *
 * Cette fonction vérifie si tous les éléments de la structure blocData sont remplis.
 *
 * @param blocData La structure blocData à vérifier.
 * @return 1 si la structure est pleine, 0 sinon.
 */
int estPleinBlocData(blocData* bloc){
    return bloc->nbChars == max_chars_par_bloc;
}


/**
 * @brief Trouve le bloc de données à partir d'une position dans le fichier.
 *
 * Cette fonction calcule le bloc de données (utile pour écrire/lire) en utilisant la position actuelle et le nombre maximum de caractères par bloc.
 *
 * @param positionActuelle La position actuelle dans le fichier.
 * @param nbCharMaxParBloc Le nombre maximum de caractères par bloc.
 * @return Le bloc de données concerné par la position "positionActuelle".
 */
int trouverBlocData(int positionActuelle, int nbCharMaxParBloc){
    return (positionActuelle / nbCharMaxParBloc) + 1;
}


/**
 * @brief Trouve la position relative dans le tableau de données à partir de la position actuelle dans le fichier.
 *
 * Cette fonction calcule la position relative dans le tableau de données en utilisant
 * la position actuelle et le nombre maximum de caractères par bloc.
 *
 * @param positionActuelle La position actuelle dans le bloc de caractères.
 * @param nbCharMaxParBloc Le nombre maximum de caractères par bloc.
 * @return La position d'insertion dans le bloc de caractères.
 */
int trouverPosition(int positionActuelle, int nbCharMaxParBloc){
    return positionActuelle % nbCharMaxParBloc;  
}

/**
 * @brief Trouve l'offset dans la partition du bloc numero blocNumber d'un fichier f.
 *
 * Cette fonction renvoie l'offset dans la partition du bloc
 * numero blocNumber d'un fichier f donné.
 *
 * @param f Pointeur vers le fichier.
 * @param blocNumber numéro du bloc du fichier.
 * @return L'offset du bloc dans la partition.
 */
off_t trouveOffsetBlocFile(file* f, int blocNumber){
    off_t offsetBloc = f->numEntete;
    blocEntete be;
    blocData bd;


    //lecture de l'entete
    if (lseek(f->fd, offsetBloc, SEEK_SET)==-1) return ERROR_LSEEK;
    if (read(f->fd, &be, sizeof(struct blocEntete))==-1) return ERROR_READ;
    //lecture du nombre de blocs du fichier "f"
    int nbBlocs = be.nbBlocs;


    //cas ou le numero du bloc depasse le nombre de blocs actuel du fichier
    if (nbBlocs<blocNumber) {
        //calculer le nombre de blocs supplementaires
        blocNumber=blocNumber-nbBlocs;
        //se deplacer de blocNumber blocDATAs depuis la fin de la partition
        offsetBloc=lseek(f->fd, (blocNumber-1)*sizeof(struct blocData), SEEK_END);
        if (offsetBloc==-1) return ERROR_LSEEK;
        //renvoyer l'offset
        return offsetBloc;
    }

    //SINON il existe un bloc tq numBloc = blocNumber => parcours sequentiel jusqu'à l'arrivée au bloc
    //1- lecture de la tete de laliste (premier bloc data)
    offsetBloc=be.numTete;
    if (lseek(f->fd, offsetBloc, SEEK_SET)==-1) return ERROR_LSEEK;
    if (read(f->fd, &bd, sizeof(struct blocData))==-1) return ERROR_READ;
    //on a lu 1 bloc (le 1er) => decrementer le nombre de bocs restants à parcourrir:
    blocNumber--;
    int i=1;
    //parcours sequentiel de la liste chainée
    while (blocNumber>0) {
    i++;
        blocNumber--;
        //lire le bloc suivant
        offsetBloc=lseek(f->fd, bd.suiv, SEEK_SET);
        if (offsetBloc==-1) return ERROR_LSEEK;
        read(f->fd, &bd, sizeof(struct blocData));
    }

    //renvoyer l'offset
    return offsetBloc;

}


/**
 * @brief Renvoie la position du dernier caractère dans le fichier.
 *
 * Cette fonction parcourt le fichier à partir de l'entête jusqu'au dernier bloc de données.
 * Elle renvoie la position du dernier caractère non vide (différent de '\0') dans le fichier.
 *
 * @param f Le pointeur vers la structure de fichier.
 * @return La position du dernier caractère dans le fichier.
 *         Si une erreur se produit lors de la lecture ou du déplacement dans le fichier, la fonction renvoie une valeur d'erreur.
 */
int getPosLastCharFile(file* f){
    off_t offsetBloc = f->numEntete;
    blocEntete be;
    blocData bd;


    //lecture de l'entete
    lseek(f->fd, offsetBloc, SEEK_SET);
    if (read(f->fd, &be, sizeof(struct blocEntete))==-1) return ERROR_READ;
    //lecture du nombre de blocs du fichier "f"
    int nbBlocs = be.nbBlocs;
    //cas fichier vide : seek_end = debut fichier
    if (nbBlocs==0) return 0;
    // sinon :

    //parcours jusqu'au dernier bloc :
    ///// pour les blocs intermediaires : cumuler max_char_par_bloc avec "pos" precedant
    ///// pour le dernier bloc : parcourir (avec var i) jusqu'à avoir trouver bd.nbChar caractères differents du '\0' ---- renvoyer pos+i+1

    //aller vers premier bloc
    //1- lecture de la tete de laliste (premier bloc data)
    offsetBloc=be.numTete;
    if (lseek(f->fd, offsetBloc, SEEK_SET)==-1) return ERROR_LSEEK;
    if (read(f->fd, &bd, sizeof(struct blocData))==-1) return ERROR_READ;
    int pos=0; //variable contenant le cumul
    //parcours
    while (bd.suiv!=-1) {
        pos = pos + max_chars_par_bloc;
        //lire bloc suivant
        lseek(f->fd,bd.suiv,SEEK_SET);
        if (read(f->fd,&bd,sizeof(blocData))==-1) return ERROR_READ;
    }
    //en sortie on est sur le dernier bloc === parcourrir
    int count = 0;
    int i=0; //pour parcourir le tableau "donnee"
    while (count<bd.nbChars){
        if (bd.donnee[i]!='\0') count++;
        i++; //aller vers caractère suivant (case suivante de donnee)
    }
    pos = pos + i;
    return pos;
}
/******************blocIndex helpers*****************/

/**
 * @brief Effectue une recherche dichotomique dans un tableau trié d'éléments de type elemTabIndex.
 *
 * @param tableau Le tableau dans lequel effectuer la recherche.
 * @param taille La taille du tableau.
 * @param nomFichier Le nom du fichier à rechercher.
 * @return L'indice de l'élément correspondant au fichier recherché s'il est trouvé, -1 sinon.
 */
int rechercheDichotomique(elemTabIndex tableau[NB_FILES_MAX], int taille, char* nomFichier) {
    int debut = 0;
    int fin = taille - 1;

    while (debut <= fin) {
        int milieu = (debut + fin) / 2;
        int cmp = strcmp(tableau[milieu].nomFichier, nomFichier);

        if (cmp < 0)
            debut = milieu + 1;
        else if (cmp > 0)
            fin = milieu - 1;
        else
            return milieu; // Le fichier a été trouvé
    }

    return -1; // Le fichier n'a pas été trouvé
}


/**
 * @brief Insère un élément dans un tableau trié en fonction du nom de fichier.
 *
 * @param tableau Le tableau trié.
 * @param taille La taille actuelle du tableau.
 * @param element L'élément à insérer.
 * @return La position d'insertion de l'élément dans le tableau.
 */
int insertionTableauTrie(elemTabIndex* tableau, int taille, elemTabIndex element) {
    int i = taille - 1;

    while (i >= 0 && strcmp(tableau[i].nomFichier, element.nomFichier) > 0) {
        tableau[i + 1] = tableau[i];
        i--;
    }

    tableau[i + 1] = element;

    return i+1; //pos insertion
}

/*********************************************************************
 |       		FONCTIONS DE MANIPULATION D'ENTETE		|
 ********************************************************************/

 /****************************PRECONDITION: Partition formatée (appel effectué à myFormat)*****************************************/
 /*****************************************GETTERS********************************************/

/**
 * \brief Récupère le nombre de blocs d'un fichier.
 *
 * Cette fonction retourne le nombre de blocs d'un fichier spécifié par le pointeur \p f.
 *
 * \param f Le pointeur vers la structure de fichier.
 * \return Le nombre de blocs du fichier.
 *         En cas d'erreur, la fonction retourne ERROR_OTHER si \p f est NULL,
 *         ERROR_LSEEK si une erreur s'est produite lors du déplacement vers le début du fichier,
 *         ou ERROR_READ si une erreur s'est produite lors de la lecture du bloc d'entête.
 */
int getNbBlocsFile(file* f) { //fd: descripteur de fichier

    if (f == NULL)
        return ERROR_OTHER;

    blocEntete buff;

    off_t numEntete = f->numEntete; //recuperer l'offset vers l'entete du fichier depuis le debut de la partition


    //se deplacer vers le debut du fichier
    if (lseek(f->fd, numEntete, SEEK_SET) == -1)
        return ERROR_LSEEK;

    // Lire le bloc d'entete du fichier
    if (read(f->fd, &buff, sizeof(struct blocEntete)) == -1)
        return ERROR_READ;

	//à la sortie, f pointe vers le bloc suivant
    return buff.nbBlocs; //nombre de blocs du fichier
}


/**
 * \brief Récupère l'offset vers le premier bloc de données du fichier.
 *
 * Cette fonction retourne le nombre de blocs d'un fichier spécifié par le pointeur \p f.
 *
 * \param f Le pointeur vers la structure de fichier.
 * \return L'offset vers le premier bloc de données.
 *         En cas d'erreur, la fonction retourne ERROR_OTHER si \p f est NULL,
 *         ERROR_LSEEK si une erreur s'est produite lors du déplacement evrs bloc d'entete,
 *         ou ERROR_READ si une erreur s'est produite lors de la lecture du bloc d'entête.
 */
off_t getNumTeteFile(file* f) { //fd descripteur de fichier


    if (f == NULL)
        return ERROR_OTHER;


    blocEntete buff;


    off_t numEntete = f->numEntete; //recuperer l'offset vers l'entete du fichier depuis le debut de la partition


    //se deplacer vers le debut du fichier
    if (lseek(f->fd, numEntete, SEEK_SET) == -1)
        return ERROR_LSEEK;

    // Lire le bloc d'entete du fichier
    if (read(f->fd, &buff, sizeof(struct blocEntete)) == -1)
        return ERROR_READ;

        //à la sortie, l'offset du fichier est sur le 1er bloc data
    return buff.numTete; //nombre de blocs du fichier
}



/****************************PRECONDITION: Partition formatée (appel effectué à myFormat)*****************************************/
/*******************************************SETTERS*************************************************/

/**
 * @brief Modifie le nombre de blocs d'un fichier.
 *
 * Cette fonction permet de modifier le nombre de blocs d'un fichier spécifié.
 *
 * @param f Le pointeur vers la structure de fichier.
 * @param val La nouvelle valeur du nombre de blocs.
 * @return 0 en cas de succès, une valeur d'erreur sinon.
 */
int setNbBlocsFile(file* f, int val) {

    if (f == NULL)
        return ERROR_OTHER;

    blocEntete buff;

    //se deplacer vers le debut du fichier
    lseek(f->fd, f->numEntete, SEEK_SET);

    // Lire le bloc d'entete du fichier
    if (read(f->fd, &buff, sizeof(struct blocEntete)) == -1)
        return ERROR_READ;

    //modifier le nombre de blocs du fichier
    buff.nbBlocs = val;
    //Aller au bloc d'entete pour l'actualiser
    lseek(f->fd, f->numEntete, SEEK_SET);
    //actualiser le bloc d'entete
    if (write(f->fd, &buff, sizeof(struct blocEntete)) == -1)
        return ERROR_WRITE;

    return 0; //renvoyer 0 si succés
}


/**
 * @brief Modifie l'offset vers la tête (premier bloc de données) du fichier.
 *
 * Cette fonction modifie l'offset vers la tête du fichier (premier bloc de données) spécifié par le pointeur `f`.
 * L'offset spécifié est donné par le paramètre `val`.
 *
 * @param f Le pointeur vers la structure de fichier.
 * @param val La nouvelle valeur de l'offset vers la tête du fichier.
 * @return 0 en cas de succès, une valeur d'erreur sinon.
 */
int setNumTeteFile(file* f, off_t val) {

    if (f == NULL)
        return ERROR_OTHER;


    blocEntete buff;


    off_t numEntete = f->numEntete; //recuperer l'offset vers l'entete du fichier depuis le debut de la partition


    //se deplacer vers le debut du fichier
    if (lseek(f->fd, numEntete, SEEK_SET) == -1)
        return ERROR_LSEEK;

    // Lire le bloc d'entete du fichier
    if (read(f->fd, &buff, sizeof(struct blocEntete)) == -1)
        return ERROR_READ;


    //modifier l'offset vers la tete
    buff.numTete = val;
    //faire un pas en arrière pour revenir au bloc d'entete
    if (lseek(f->fd, numEntete, SEEK_SET) == -1)
        return ERROR_LSEEK;
    //actualiser le bloc d'entete
    if (write(f->fd, &buff, sizeof(struct blocEntete)) == -1)
        return ERROR_WRITE;

    return 0; //renvoyer 0 si succés
}



/******************************************TAILLE FICHIER***************************************/

/**
 * @brief Calcule la taille du fichier.
 *
 * Cette fonction calcule la taille du fichier en fonction du nombre de blocs de données qu'il occupe.
 *
 * @param f Le pointeur vers la structure de fichier.
 * @return La taille du fichier en octets.
 *         En cas d'erreur, la fonction renvoie une valeur négative correspondant à un code d'erreur spécifique.
 */
int size(file* f){
	blocEntete buff;

	if (f== NULL)
		return ERROR_OTHER;

	off_t offset_entete = f->numEntete;
	int fd= f->fd; //descripteur de fichier de la partition

	//se deplacer vers le bloc d'entete du fichier
	if (lseek(fd, offset_entete, SEEK_SET) == -1)
        	return ERROR_LSEEK;

        //lire le bloc d'entete
        if (read(fd, &buff, sizeof(struct blocEntete)) == -1)
        	return ERROR_READ;

        //calculer la taille
        int sizeFile = buff.nbBlocs*sizeof(struct blocData);

        return sizeFile;
}


/**
 * \brief Calcule la taille réelle d'un fichier.
 *
 * Cette fonction calcule la taille réelle d'un fichier en parcourant les blocs de données
 * associés au fichier et en sommant le nombre de caractères de chaque bloc.
 *
 * \param f Un pointeur vers une structure de fichier contenant les informations nécessaires.
 * \return La taille réelle du fichier en nombre de caractères, ou 0 si le fichier est vide,
 *         ou une valeur d'erreur si une erreur s'est produite lors de la lecture des blocs.
 *         Les valeurs d'erreur possibles sont définies dans le fichier d'en-tête correspondant.
 */
int getSizeReelFile(file* f){

    off_t offsetBloc = f->numEntete;
    blocEntete be;

    //lire le bloc d'entete
    if (lseek(f->fd,offsetBloc,SEEK_SET)==-1) return ERROR_LSEEK;
    if (read(f->fd,&be,sizeof(blocEntete))==-1) return ERROR_READ;

    //obtenir le nombre de blocs
    int nbBlocs = be.nbBlocs;

    //---cas 1: fichier vide => size = 0
    if (nbBlocs==0) return 0;

    //---cas 2: le fichier contient des donnees
    ////parcours sequentiel et sommation de nbChars de chaque bloc jusqu'à arriver à suivant==-1'
    blocData bd;
    ////lire 1er bloc data "bd"
    if (lseek(f->fd,be.numTete,SEEK_SET)==-1) return ERROR_LSEEK;
    if (read(f->fd,&bd,sizeof(blocData))==-1) return ERROR_READ;
    ////initialiser la taille du fichier à taille du 1er bloc data
    int sizeReel = bd.nbChars;
    ////commencer le parcours
    while (bd.suiv!=-1){
        ////lire bloc prochain
        if (lseek(f->fd,bd.suiv,SEEK_SET)==-1) return ERROR_LSEEK;
        if (read(f->fd,&bd,sizeof(blocData))==-1) return ERROR_READ;
        sizeReel=sizeReel+bd.nbChars;
    }
    //renvoyer
    return sizeReel;
}

/***********************************************************************************************/
/*                          FONCTIONS PRINCIPALES                                              */
/***********************************************************************************************/
/************************MyFormat*******************************/

/**
 * \brief Fonction pour formater une partition et initialiser un bloc d'index vide.
 *
 * \param partitionName Le nom de la partition à formater.
 * \return 0 si le formattage est réussi, sinon un code d'erreur.
 *
 * Cette fonction tente de créer un fichier représentant la partition spécifiée. Si le fichier existe déjà,
 * il ouvre le fichier existant. Sinon, il crée un nouveau fichier et écrit un bloc d'index vide dedans.
 *
 * \note Cette fonction utilise les fonctions open, lseek et write pour manipuler les fichiers.
 *
 * \warning Assurez-vous d'avoir les permissions nécessaires pour créer et écrire dans le fichier spécifié.
 *
 * \see blocIndex
 */
int myFormat(char* partitionName){
    //initialiser un bloc d'index vide
    blocIndex bi = init_blocIndex();


    //essayer de creer le fichier representant la partition
    fd = open(partitionName, O_CREAT | O_EXCL | O_RDWR, 0777);

    if (fd == -1) {
        //en cas d'erreur car fichier existe deja
        if (errno == EEXIST) {
            //fichier de la partition existe deja
            printf("Formatage d'une partition qui existe deja...\n");
            //ouverture du fichier representant la partition
            fd = open(partitionName, O_RDWR);
            printf("formattage réussi.\n");
            return 0;
        } else {
            //erreur
            perror("Erreur lors du formattage de la partition.\n");
            return ERROR_OPEN;
        }
    } else {
        //Partition créée

        //ecriture du bloc d'index
        int offInd=offInd=lseek(fd,0,SEEK_SET);
        if (offInd==-1) return ERROR_LSEEK;
        if (write(fd, &bi, sizeof(struct blocIndex)) == -1) return ERROR_WRITE;

        printf("Partition formattée et bloc d'index initialisé avec succés.\n");
        return 0;
    }
}

/*********************************MyOpen***********************************/
///////////////file * myOpen(char* fileName);///////////////////////////////////////////////


/**
 * @brief Ouvre un fichier et retourne une structure file contenant les informations nécessaires.
 *
 * @param fileName Le nom du fichier à ouvrir.
 * @return Un pointeur vers la structure file si le fichier est ouvert avec succès, NULL sinon.
 */
file * myOpen(char* fileName) {
        off_t offsetEntete;
    // Charger le bloc d'index
    blocIndex index=init_blocIndex();
    if (lseek(fd, 0, SEEK_SET)==-1) {
    	perror("Erreur de deplacement\n");
    	return NULL;
    	} //se deplacer vers debut partition
    if (read(fd, &index, sizeof(struct blocIndex)) == -1) { //lire le bloc d'index
        perror("Erreur de lecture du bloc d'index\n");
        return NULL;
    }

    // Rechercher le fichier dans le tableau d'index
    int pos = rechercheDichotomique(index.tabIndex, index.nbFichiers, fileName);

    // Si le fichier n'existe pas, le créer
    if (pos == -1) {
        // Initialiser le bloc d'entête
        blocEntete entete;
        strcpy(entete.nomFichier, fileName);
        entete.nbBlocs = 0;
        entete.numTete = -1;


    // Écrire le bloc d'entête à la fin de la partition
        offsetEntete=lseek(fd,0,SEEK_END); //Aller à la fin de la partition
        if (write(fd, &entete, sizeof(struct blocEntete)) == -1) { //Ecrire l'entete
            perror("Erreur d'écriture du bloc d'entête\n");
            return NULL;
        }

        // Ajouter le fichier au tableau d'index et incrémenter le nombre de fichiers
        elemTabIndex element;
        strcpy(element.nomFichier, fileName);
        element.numBlocEntete = offsetEntete;
        pos=insertionTableauTrie(index.tabIndex, index.nbFichiers, element);
        index.nbFichiers=index.nbFichiers+1;

        // Écrire le bloc d'index à l'emplacement 0 de la partition
        if (lseek(fd, 0, SEEK_SET)==-1) {
        	perror("Erreur de deplacement\n");
        	return NULL;
        }
        if (write(fd, &index, sizeof(blocIndex)) == -1) {
            perror("Erreur d'écriture du bloc d'index");
            return NULL;
        }
    }

    // Ouvrir le fichier (qu'il soit nouveau ou existant)
    file *f = malloc(sizeof(file));
    if (f == NULL) {
        perror("Erreur d'allocation de mémoire");
        return NULL;
    }
    f->fd = fd;
    f->numEntete = index.tabIndex[pos].numBlocEntete;
    //printf("num entete is : ")
    f->pos = 0;

    return f;
}


/*********************************MyWrite**********************************/

/**
 * @brief Permet d'écrire des données dans un fichier.
 *
 * La fonction myWrite prend en paramètres un pointeur vers une structure de fichier (file* f), un pointeur vers un buffer de données (void* buffer),
 * et la taille des données à écrire (int size). Elle retourne le nombre de caractères écrits.
 *
 * La fonction myWrite effectue les opérations suivantes :
 * 1. Trouver le numéro du blocData dans lequel écrire.
 * 2. Obtenir le nombre de blocs du fichier.
 * 3. Gérer le cas où le numéro du bloc dépasse le nombre actuel de blocs dans le fichier.
 *    - Si le fichier est vide, créer un blocData et le chaîner avec le blocEntete.
 *    - Sinon, créer les blocs nécessaires et les chaîner avec le dernier bloc existant.
 * 4. Se déplacer vers l'emplacement où l'écriture va se passer.
 * 5. Trouver la position d'insertion dans le bloc.
 * 6. Écrire les caractères un par un.
 * 7. Gérer le cas où la fin du bloc est atteinte.
 *    - Si un bloc suivant existe, actualiser le bloc précédent et se déplacer vers le bloc suivant.
 *    - Sinon, allouer un nouveau bloc à la fin de la partition.
 * 8. Mettre à jour le nombre de caractères présents dans le bloc.
 * 9. Mettre à jour la position courante dans le fichier.
 * 10. Écrire le blocData final et mettre à jour le nombre de blocs.
 *
 * @param f Un pointeur vers une structure de fichier.
 * @param buffer Un pointeur vers un buffer de données.
 * @param size La taille des données à écrire.
 * @return Le nombre de caractères écrits.
 */
int myWrite(file* f, void* buffer, int size) {
    blocData currentBloc;
    off_t currentBlocOffset;
    int positionInBloc;
    int i;
    char* buff = (char*)buffer;


    // Trouver le numero du blocData dans lequel écrire
    int blocNumber = trouverBlocData(f->pos, max_chars_par_bloc);

    //Obtenir le nombre de blocs du fichier
    int nbBlocs=getNbBlocsFile(f);

    // cas 1 : cas ou le numero du bloc depasse le nombre acuel de blocs dans le fichier
    //=> creer nbBlocs-blocNumber blocs et se deplacer vers le dernier bloc créé
    if (blocNumber>nbBlocs) {
        //cas particulier: cas ou le fichier est vide => creer blocData et le chainer avec blocEntete
        if (nbBlocs==0) {
            currentBloc=alloc_bloc();
            //l'ecrire à la fin de la partition
            currentBlocOffset=lseek(f->fd, 0, SEEK_END);
            if (currentBlocOffset==-1) return ERROR_LSEEK;
            if (write(f->fd, &currentBloc, sizeof(struct blocData))==-1) return ERROR_WRITE;
            //chainage avec entete
            setNumTeteFile(f,currentBlocOffset);
            //incrementer nbBlocs
            nbBlocs++;
            setNbBlocsFile(f,nbBlocs);
        }
        off_t offsetBlocPrec;
        blocData blocPrec;
        //iterations pour la creation des blocs
        while (blocNumber>nbBlocs) {
            // ----------------1) CREATION DU NOUVEAU BLOC
            //creer un nouveau bloc
            currentBloc=alloc_bloc();
            //l'ecrire à la fin de la partition
            currentBlocOffset=lseek(f->fd, 0, SEEK_END);
            if (currentBlocOffset==-1) return ERROR_LSEEK;
            if (write(f->fd, &currentBloc, sizeof(struct blocData))==-1) return ERROR_WRITE;
            // ----------------2) CHAINAGE
            //obtenir l'offset du dernier bloc du fichier dans la partition
            offsetBlocPrec=trouveOffsetBlocFile(f, nbBlocs);
            //le lire
            lseek(f->fd, offsetBlocPrec, SEEK_SET);
            if (read(f->fd, &blocPrec, sizeof(blocData))==-1) return ERROR_READ;
            //effectuer le chainage
            blocPrec.suiv=currentBlocOffset;
            //l'actualiser
            lseek(f->fd, offsetBlocPrec, SEEK_SET);
            if (write(f->fd, &blocPrec, sizeof(blocData))==-1) return ERROR_WRITE;
            //incrementer le nombre de blocs
            setNbBlocsFile(f,nbBlocs+1);
            nbBlocs++;
        }

    }
    // se deplacer vers ou va se passer l'ecriture
    else {
        currentBlocOffset= trouveOffsetBlocFile(f, blocNumber);
        lseek(f->fd, currentBlocOffset, SEEK_SET);
        read(f->fd, &currentBloc, sizeof(struct blocData));
    }

    // Trouver la position d'insertion dans le bloc
    positionInBloc = trouverPosition(f->pos, max_chars_par_bloc);
    // Écrire les caractères un par un
    for (i = 0; i < size; i++) {
        //Si on est arrivé à la fin du bloc
        if (positionInBloc==max_chars_par_bloc) {
            if (currentBloc.suiv!=-1){
            //--cas 1 : bloc suivant existe, on est pas encore arrivé à la fin du fichier
                //actualiser le bloc precedant
                lseek(f->fd, currentBlocOffset, SEEK_SET);
                 write(f->fd, &currentBloc, sizeof(blocData));
                //se deplacer vers le bloc suivant
                currentBlocOffset=lseek(f->fd, currentBloc.suiv, SEEK_SET);
                read(f->fd, &currentBloc, sizeof(blocData));

            } else {
            //--cas 2 : allouer un nouveau bloc
                //le nouveau bloc se trouve à la fin de la partition => actualiser le chainage
                currentBloc.suiv = lseek(f->fd, 0, SEEK_END);
                lseek(f->fd, currentBlocOffset, SEEK_SET);
                write(f->fd, &currentBloc, sizeof(struct blocData));
                //re-initialiser currentBloc pour l'utiliser comme etant le nouveau bloc
                currentBloc = alloc_bloc();
                currentBlocOffset = lseek(f->fd, 0, SEEK_END);
                write(f->fd,&currentBloc,sizeof(blocData));
                setNbBlocsFile(f, getNbBlocsFile(f) + 1); //actualiser le nombre de blocs
            }
            positionInBloc = 0;
            blocNumber++;

        }

        // Écrire le caractère et mettre à jour le nombre de caractères
        //si la case etait auparavant vide (pas d'ecraement) alors incrementer le nb de caracteres presents dans bloc
        if (currentBloc.donnee[positionInBloc]=='\0'){
            currentBloc.nbChars=currentBloc.nbChars+1;
        }
        currentBloc.donnee[positionInBloc] = buff[i];
        positionInBloc++;
        // Mettre à jour la position courante dans le fichier
        f->pos++;
    }

    // Écrire le blocData final et mettre à jour le nombre de blocs
    lseek(f->fd, currentBlocOffset, SEEK_SET);
    write(f->fd, &currentBloc, sizeof(struct blocData));

    return size;
}

/*********************************MyRead*************************************/
/**
 * @brief Permet de lire des données à partir d'un fichier.
 *
 * La fonction myRead lit les données à partir d'un fichier spécifié par le descripteur de fichier (file *f).
 * Les données lues sont stockées dans un tampon (void *buffer) d'une taille spécifiée (int nBytes).
 * La fonction retourne le nombre d'octets lus avec succès.
 *
 * @param f Le descripteur de fichier à partir duquel lire les données.
 * @param buffer Le tampon dans lequel stocker les données lues.
 * @param nBytes Le nombre d'octets à lire.
 * @return Le nombre d'octets lus avec succès, ou une valeur d'erreur en cas d'échec.
 */
int myRead(file *f,void * buffer, int nBytes){
    //mySeek(f,-1,SEEK_END);
    blocData currentBloc;
    off_t currentBlocOffset;
    int positionInBloc;
    int i;

    if(f==NULL || buffer==NULL || nBytes <=0)
    {
        //Vérification des paramétres d'entrée
        return ERROR_OTHER;
    }
    //on récupére le descripteur du fichier de f
    int fd = f->fd;
    // Trouver le numero du blocData dans lequel écrire
    int blocNumber = trouverBlocData(f->pos, max_chars_par_bloc);
    // se deplacer vers ou va se passer la lecture
    currentBlocOffset= trouveOffsetBlocFile(f, blocNumber);
    lseek(f->fd, currentBlocOffset, SEEK_SET);
   // read(f->fd, &currentBloc, sizeof(blocData));

    // Trouver la position exacte (dans le bloc) à partir de laquelle on va effectuer la lecture
    positionInBloc = trouverPosition(f->pos, max_chars_par_bloc);

    if(positionInBloc == -1){
        printf("La position %d est au delà de 'espace du fichier ..",f->pos);
        return ERROR_LSEEK;
    }

    int nbyteslu = 0; // initialisation du nombre d'octets lus
    //lecture caractére par caractére

    //lecture du 1er bloc à lire
    // la lecture d'un bloc de données à partir de la position pos
    ssize_t bloc_read = read(fd, &currentBloc, sizeof(blocData));
    //mise à jour du nombre d'octets lus
    // nbyteslu = bloc_read;
    if (bloc_read == -1) {
        perror("Erreur lors de la lecture du fichier");
        return ERROR_READ;
    }
    for (i = 0; i < nBytes; i++) {

         // lire le caractére correspondant
         ((char*)buffer)[i] = currentBloc.donnee[positionInBloc];
         //Mettre à jour le nombre de caractéres lus
         nbyteslu++;
         //Avance le pointeur de lecture
         positionInBloc++;

        // Mettre à jour la position courante dans le fichier
        f->pos++;
        //Si on a parcours tout le bloc, passer au suivant
        if(positionInBloc == max_chars_par_bloc)
        {
            positionInBloc = 0;
           // Vérifier s'il reste toujours des octets à lire mais qu'on est arrivé à la fin du fichier
            if (nbyteslu < nBytes && currentBloc.suiv == -1) {
                     // Fin du fichier atteinte avant de finir la lecture des nBytes
                     break;
             }

            // Passer au bloc de données suivant pour continuer à lire
            off_t bloc_suivant = currentBloc.suiv;
            if (bloc_suivant == -1) {
               // Fin du fichier atteinte
                break;
            }

            // Se déplacer au prochain bloc de données
            int  prochain_bloc = lseek(fd, bloc_suivant , SEEK_SET);
            if (prochain_bloc == -1) {
                perror("Erreur lors du positionnement du pointeur de fichier");
                return ERROR_LSEEK;
            }
            // la lecture d'un bloc de données à partir de la position pos
            ssize_t bloc_read = read(fd, &currentBloc, sizeof(blocData));
            //mise à jour du nombre d'octets lus
            // nbyteslu = bloc_read;
            if (bloc_read == -1) {
                perror("Erreur lors de la lecture du fichier");
                return ERROR_READ;
            }
        }
    }

    return nbyteslu;
}

/*********************************MySeek***********************************/

/**
 * @brief Déplace la position courante dans un fichier.
 *
 * Cette fonction permet de déplacer la position courante dans un fichier en fonction de l'offset et de la base spécifiés.
 *
 * @param f Un pointeur vers la structure de fichier.
 * @param offset L'offset de déplacement.
 * @param base La base de déplacement (SEEK_SET, SEEK_CUR, SEEK_END).
 *
 * @note Si la nouvelle position est négative, une erreur sera affichée.
 */
void mySeek(file* f, int offset, int base){
    int pos=f->pos;
    switch (base) {
        case SEEK_SET:
            offset>=0?f->pos=offset:perror("impossible de se deplacer de l'offset fournit. Nouvelle position negative.");
            break;
        case SEEK_CUR:
            pos = pos+offset;
            pos>=0?f->pos=pos:perror("impossible de se deplacer de l'offset fournit. Nouvelle position negative.");
            break;
        case SEEK_END:
            pos=getPosLastCharFile(f)+offset;
            pos>=0?f->pos=pos:perror("impossible de se deplacer de l'offset fournit. Nouvelle position negative.");
            break;
        default: //
            perror("valeur erronée de 'base', valeurs possibles: SEEK_SET, SEEK_CUR, SEEK_END.");
    }
}
/*********************************MyClose***********************************/

/**
 * \brief Ferme un fichier.
 *
 * Cette fonction libère la mémoire allouée pour la structure de fichier spécifiée.
 *
 * \param f Le pointeur vers la structure de fichier à fermer.
 */
void myClose(file* f){
    free(f);
}
/*********************************closePartition****************************/

/**
 * @brief Ferme une partition.
 *
 * Cette fonction ferme le descripteur de fichier (partition) spécifié.
 *
 * @param fd Le descripteur de fichier à fermer.
 * @return 0 en cas de succès, ERROR_OTHER en cas d'erreur.
 */
int closePartition(int fd){
    if (close(fd)==-1) return ERROR_OTHER;
    return 0;
}
