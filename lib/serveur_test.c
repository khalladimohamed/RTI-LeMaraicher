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

int main() {
    
    int serverPort = 50000; // Numéro de port du serveur

    int serverSocket, clientSocket;
    char clientIP[INET_ADDRSTRLEN];
    char data[TAILLE_MAX_DATA];

    // Création de la socket serveur
    serverSocket = ServerSocket(serverPort);

    if (serverSocket == -1) {
        perror("Erreur lors de la création du socket serveur");
        return 1;
    }

    printf("Serveur en écoute sur le port %d...\n", serverPort);

    // Accepter la connexion d'un client
    clientSocket = Accept(serverSocket, clientIP);

    if (clientSocket == -1) {
        perror("Erreur lors de l'acceptation de la connexion du client");
        close(serverSocket);
        return 1;
    }

    printf("Client connecté depuis l'adresse IP : %s\n", clientIP);

    // Recevoir des données du client
    int receivedBytes = Receive(clientSocket, data);

    if (receivedBytes == -1) {
        perror("Erreur lors de la réception des données du client");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    data[receivedBytes] = '\0';
    printf("Données reçues du client : %s\n", data);

    // Envoyer des données au client
    char message[] = "Bonjour, client!";
    int sentBytes = Send(clientSocket, message, strlen(message));

    if (sentBytes == -1) {
        perror("Erreur lors de l'envoi des données");
        close(clientSocket);
        return 1;
    }

    printf("Données envoyées au client : %s\n", message);

    // Fermer la socket du client et du serveur
    close(clientSocket);
    close(serverSocket);

    return 0;
}
