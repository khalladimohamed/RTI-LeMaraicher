




int main() {
    int serverSocket = ServerSocket(50000);

    // ... Votre code pour accepter les connexions

    close(serverSocket);

    return 0;
}


int main() {
    int serverSocket = ServerSocket(50000);

    while (1) {
        char ipClient[INET_ADDRSTRLEN];
        int clientSocket = Accept(serverSocket, ipClient);

        // Traitez la connexion client ici
        printf("Client connecté depuis l'adresse IP : %s\n", ipClient);

        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}
