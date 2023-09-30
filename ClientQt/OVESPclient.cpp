#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "OVESPclient.h"
#include "../lib/LibSockets.h"

//***** Gestion du protocole OVESP ***********************************
int SMOP_Login(const char* user, const char* password, const int newUser)
{
	char requete[200],reponse[200];
	int idClient = -1;

	// ***** Construction de la requete *********************
	sprintf(requete,"LOGIN#%s#%s#%d",user, password, newUser);

	// ***** Envoi requete + réception réponse **************
	Echange(requete,reponse);

	// ***** Parsing de la réponse **************************
	char *ptr = strtok(reponse,"#"); // entête = LOGIN (normalement...)
	ptr = strtok(NULL,"#"); // statut = ok ou ko
	if (strcmp(ptr,"ok") == 0)
	{
		idClient = 
	}
	else
	{
		ptr = strtok(NULL,"#"); // raison du ko
		printf("Erreur de login: %s\n",ptr);
	}
	return onContinue;
}


//*******************************************************************
void SMOP_Logout()
{
	char requete[200],reponse[200];
	int nbEcrits, nbLus;

	// ***** Construction de la requete *********************
	sprintf(requete,"LOGOUT");

	// ***** Envoi requete + réception réponse **************
	Echange(requete,reponse);

	// ***** Parsing de la réponse **************************
	// pas vraiment utile...
}


//*******************************************************************
void SMOP_Operation(char op,int a,int b)
{
	char requete[200],reponse[200];

	// ***** Construction de la requete *********************
	sprintf(requete,"OPER#%c#%d#%d",op,a,b);

	// ***** Envoi requete + réception réponse **************
	Echange(requete,reponse);

	// ***** Parsing de la réponse **************************
	char *ptr = strtok(reponse,"#"); // entête = OPER (normalement...)
	ptr = strtok(NULL,"#"); // statut = ok ou ko
	if (strcmp(ptr,"ok") == 0)
	{
		ptr = strtok(NULL,"#"); // résultat du calcul
		printf("Résultat = %s\n",ptr);
	}
	else
	{
		ptr = strtok(NULL,"#"); // raison du ko
		printf("Erreur: %s\n",ptr);
	}
}


//***** Echange de données entre client et serveur ******************
void Echange(char* requete, char* reponse)
{
	int nbEcrits, nbLus;

	// ***** Envoi de la requete ****************************
	if ((nbEcrits = Send(sClient,requete,strlen(requete))) == -1)
	{
		perror("Erreur de Send");
		close(sClient);
		exit(1);
	}

	// ***** Attente de la reponse **************************
	if ((nbLus = Receive(sClient,reponse)) < 0)
	{
		perror("Erreur de Receive");
		close(sClient);
		exit(1);
	}
	if (nbLus == 0)
	{
		printf("Serveur arrete, pas de reponse reçue...\n");
		close(sClient);
		exit(1);
	}
	reponse[nbLus] = 0;
}