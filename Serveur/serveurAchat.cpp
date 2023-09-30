#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "OVESPserveur.h"
#include "../lib/LibSockets.h"


void HandlerSIGINT(int s);
void TraitementConnexion(int sService);
void* FctThreadClient(void* p);
int sEcoute;


// Gestion du pool de threads
#define NB_THREADS_POOL 2
#define TAILLE_FILE_ATTENTE 20
int socketsAcceptees[TAILLE_FILE_ATTENTE];
int indiceEcriture = 0, indiceLecture = 0;
pthread_mutex_t mutexSocketsAcceptees;
pthread_cond_t condSocketsAcceptees;


int main(int argc,char* argv[])
{
    if (argc != 2)
    {
        printf("Erreur...\n");
        printf("USAGE : Serveur portServeur\n");
        exit(1);
    }
    // Initialisation socketsAcceptees
    pthread_mutex_init(&mutexSocketsAcceptees, NULL);
    pthread_cond_init(&condSocketsAcceptees, NULL);
    for (int i=0 ; i<TAILLE_FILE_ATTENTE ; i++)
        socketsAcceptees[i] = -1;

    // Armement des signaux
    struct sigaction A;
    A.sa_flags = 0;
    sigemptyset(&A.sa_mask);
    A.sa_handler = HandlerSIGINT;
    if (sigaction(SIGINT, &A, NULL) == -1)
    {
        perror("Erreur de sigaction");
        exit(1);
    }
    // Creation de la socket d'écoute
    if ((sEcoute = ServerSocket(atoi(argv[1]))) == -1)
    {
        perror("Erreur de ServeurSocket");
        exit(1);
    }
    // Creation du pool de threads
    printf("Création du pool de threads.\n");
    pthread_t th;
    for (int i=0 ; i<NB_THREADS_POOL ; i++)
        pthread_create(&th, NULL, FctThreadClient, NULL);

    // Mise en boucle du serveur
    int sService;
    char ipClient[50];
    printf("Demarrage du serveur.\n");
    

    while(1)
    {
        printf("Attente d'une connexion...\n");
        if ((sService = Accept(sEcoute, ipClient)) == -1)
        {
            perror("Erreur de Accept");
            close(sEcoute);
            SMOP_Close();
            exit(1);
        }

        // Vérifier si la file d'attente est pleine
        pthread_mutex_lock(&mutexSocketsAcceptees);
        if (indiceEcriture - indiceLecture >= TAILLE_FILE_ATTENTE)
        {
            // File d'attente pleine, refuser la connexion
            pthread_mutex_unlock(&mutexSocketsAcceptees);
            printf("File d'attente pleine, connexion refusée.\n");
            close(sService);
        }
        else
        {
            // Ajouter la connexion à la file d'attente
            socketsAcceptees[indiceEcriture] = sService;
            indiceEcriture++;
            if (indiceEcriture == TAILLE_FILE_ATTENTE) indiceEcriture = 0;
            pthread_mutex_unlock(&mutexSocketsAcceptees);
            pthread_cond_signal(&condSocketsAcceptees);
            printf("Connexion acceptée : IP=%s socket=%d\n", ipClient, sService);
        }
    }

}


void* FctThreadClient(void* p)
{
    int sService;
    
    while(1)
    {
        printf("\t[THREAD %p] Attente socket...\n",pthread_self());
        
        // Attente d'une tâche
        pthread_mutex_lock(&mutexSocketsAcceptees);
        while (indiceEcriture == indiceLecture)
            pthread_cond_wait(&condSocketsAcceptees, &mutexSocketsAcceptees);

        sService = socketsAcceptees[indiceLecture];
        socketsAcceptees[indiceLecture] = -1;
        indiceLecture++;
        if (indiceLecture == TAILLE_FILE_ATTENTE) indiceLecture = 0;
        pthread_mutex_unlock(&mutexSocketsAcceptees);
        
        // Traitement de la connexion (consommation de la tâche)
        printf("\t[THREAD %p] Je m'occupe de la socket %d\n", pthread_self(),sService);
        
        TraitementConnexion(sService);
    }

}


void HandlerSIGINT(int s)
{
    printf("\nArret du serveur.\n");
    close(sEcoute);
    pthread_mutex_lock(&mutexSocketsAcceptees);
    for (int i=0 ; i<TAILLE_FILE_ATTENTE ; i++)
        if (socketsAcceptees[i] != -1) close(socketsAcceptees[i]);
    
    pthread_mutex_unlock(&mutexSocketsAcceptees);
    OVESP_Close();
    exit(0);
}


void TraitementConnexion(int sService)
{
    char requete[200], reponse[200];
    int nbLus, nbEcrits;
    bool onContinue = true;
    
    while (onContinue)
    {
        printf("\t[THREAD %p] Attente requete...\n",pthread_self());
        

        // ***** Reception Requete ******************
        if ((nbLus = Receive(sService,requete)) < 0)
        {
            perror("Erreur de Receive");
            close(sService);
            HandlerSIGINT(0);
        }
        

        // ***** Fin de connexion ? *****************
        if (nbLus == 0)
        {
            printf("\t[THREAD %p] Fin de connexion du client.\n",pthread_self());
            close(sService);
            return;
        }
        requete[nbLus] = 0;
        printf("\t[THREAD %p] Requete recue = %s\n",pthread_self(),requete);
        

        // ***** Traitement de la requete ***********
        onContinue = OVESP(requete,reponse,sService);
        

        // ***** Envoi de la reponse ****************
        if ((nbEcrits = Send(sService,reponse,strlen(reponse))) < 0)
        {
            perror("Erreur de Send");
            close(sService);
            HandlerSIGINT(0);
        }
        printf("\t[THREAD %p] Reponse envoyee = %s\n",pthread_self(),reponse);
        if (!onContinue)
            printf("\t[THREAD %p] Fin de connexion de la socket %d\n",pthread_self(),sService);
    }
}
