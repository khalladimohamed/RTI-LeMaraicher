#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "LibSockets.h"


int ServerSocket(int port)
{
	int sServeur;
	char portChar[6];
	snprintf(portChar, sizeof(portChar), "%d", port);
	
	printf("port : %s\n", portChar);
	printf("pid = %d\n",getpid());
	
	// Creation de la socket
	if ((sServeur = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Erreur de socket()");
		exit(1);
	}
	printf("socket creee = %d\n",sServeur);
	
	// Construction de l'adresse
	struct addrinfo hints;
	struct addrinfo *results;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV; // pour une connexion passive
	
	//l'ip adresse doit etre null pour prendre l'adresse de la machine local
	if (getaddrinfo(NULL, portChar, &hints, &results) != 0) 
	{
		close(sServeur);
		exit(1);
	}
	
	// Liaison de la socket à l'adresse
	if (bind(sServeur,results->ai_addr,results->ai_addrlen) < 0)
	{
		perror("Erreur de bind()");
		exit(1);
	}

	return sServeur;
}


int Accept(int sEcoute, char* ipClient) 
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Écoute pour les connexions entrantes
    if (listen(sEcoute, SOMAXCONN) == -1) {
        perror("Erreur de listen()");
        exit(1);
    }

    // Accepte la connexion d'un client
    int clientSocket = accept(sEcoute, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        perror("Erreur de accept()");
        exit(1);
    }

    // Récupère l'adresse IP distante du client à l'aide de getpeername
    if (ipClient != NULL) {
        struct sockaddr_in peerAddr;
        socklen_t peerAddrLen = sizeof(peerAddr);
        if (getpeername(clientSocket, (struct sockaddr*)&peerAddr, &peerAddrLen) == -1) {
            perror("Erreur de getpeername()");
            exit(1);
        }
        strcpy(ipClient, inet_ntoa(peerAddr.sin_addr));
    }

    return clientSocket;
}


int ClientSocket(char* ipServeur, int portServeur) 
{
    int clientSocket;
    char portServeurChar[6];
	snprintf(portServeurChar, sizeof(portServeurChar), "%d", portServeur);
    
    // Création de la structure d'informations sur l'hôte
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Résolution de l'adresse IP du serveur
    struct addrinfo* results;
    if (getaddrinfo(ipServeur, portServeurChar, &hints, &results) != 0) {
        perror("Erreur de getaddrinfo()");
        exit(1);
    }

    // Création de la socket client
    if ((clientSocket = socket(results->ai_family, results->ai_socktype, 0)) == -1) {
        perror("Erreur de socket()");
        freeaddrinfo(results);
        exit(1);
    }

    // Connexion au serveur
    if (connect(clientSocket, results->ai_addr, results->ai_addrlen) == -1) {
        perror("Erreur de connect()");
        close(clientSocket);
        freeaddrinfo(results);
        exit(1);
    }

    freeaddrinfo(results);

    return clientSocket;
}


int Send(int sSocket,char* data,int taille)
{
	if (taille > TAILLE_MAX_DATA)
		return -1;
	// Preparation de la charge utile
	char trame[TAILLE_MAX_DATA+2];
	memcpy(trame,data,taille);
	trame[taille] = '#';
	trame[taille+1] = ')';
	// Ecriture sur la socket
	return write(sSocket,trame,taille+2)-2;
}


int Receive(int sSocket,char* data)
{
	bool fini = false;
	int nbLus, i = 0;
	char lu1,lu2;
	while(!fini)
	{
		if ((nbLus = read(sSocket,&lu1,1)) == -1)
			return -1;
		if (nbLus == 0) return i; // connexion fermee par client
		if (lu1 == '#')
		{
			if ((nbLus = read(sSocket,&lu2,1)) == -1)
				return -1;
			if (nbLus == 0) return i; // connexion fermee par client
			if (lu2 == ')')
			{
				fini = true;
			} 
			else
			{
				data[i] = lu1;
				data[i+1] = lu2;
				i += 2;
			}
		}
		else
		{
			data[i] = lu1;
			i++;
		}
	}
	return i;
}