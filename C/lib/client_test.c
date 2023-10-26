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
    

    char serverIP[] = "192.168.109.130"; //Adresse IP du serveur / broadcast 192.168.109.255
    int serverPort = 50000; //Numéro de port du serveur

    int clientSocket;
    char data[TAILLE_MAX_DATA];

    // Création de la socket client
    clientSocket = ClientSocket(serverIP, serverPort);

    if (clientSocket == -1) {
        perror("Erreur lors de la création du socket client");
        return 1;
    }

    // Envoyer des données au serveur
    char message[] = "Bonjour, serveur!";
    int sentBytes = Send(clientSocket, message, strlen(message));

    if (sentBytes == -1) {
        perror("Erreur lors de l'envoi des données");
        close(clientSocket);
        return 1;
    }

    printf("Données envoyées au serveur : %s\n", message);

    // Recevoir des données du serveur
    int receivedBytes = Receive(clientSocket, data);

    if (receivedBytes == -1) {
        perror("Erreur lors de la réception des données");
        close(clientSocket);
        return 1;
    }

    data[receivedBytes] = '\0';
    printf("Données reçues du serveur : %s\n", data);

    // Fermer la socket client
    close(clientSocket);

    return 0;
}
