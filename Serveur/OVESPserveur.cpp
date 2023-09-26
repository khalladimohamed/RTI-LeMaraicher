#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "OVESPserveur.h"


//***** BD **********************************************************
MYSQL* mysql_conn; // Connexion MySQL
MYSQL_RES* result; // Résultat de la requête
MYSQL_ROW row;     // Ligne de résultat

pthread_mutex_t mysqlMutex = PTHREAD_MUTEX_INITIALIZER;

// Verrouillez le mutex pour protéger l'accès à la base de données MySQL
//pthread_mutex_lock(&mysqlMutex);
// Déverrouillez le mutex après avoir terminé les opérations sur la base de données
//pthread_mutex_unlock(&mysqlMutex);


//***** Etat du protocole : liste des clients loggés ****************
int clients[NB_MAX_CLIENTS];
int nbClients = 0;
int estPresent(int socket);
void ajoute(int socket);
void retire(int socket);
pthread_mutex_t mutexClients = PTHREAD_MUTEX_INITIALIZER;



//***** Parsing de la requete et creation de la reponse *************
bool OVESP(char* requete, char* reponse,int socket)
{

	// ***** Récupération nom de la requete *****************
	char *ptr = strtok(requete,"#");

	// ***** LOGIN ******************************************
	if (strcmp(ptr,"LOGIN") == 0)
	{
		char user[50], password[50];
		strcpy(user,strtok(NULL,"#"));
		strcpy(password,strtok(NULL,"#"));
		printf("\t[THREAD %p] LOGIN de %s\n",pthread_self(),user);
		if (estPresent(socket) >= 0) // client déjà loggé
		{
			sprintf(reponse,"LOGIN#ko#Client déjà loggé !");
			return false;
		}
		else
		{
			if (OVESP_Login(user,password))
			{
				sprintf(reponse,"LOGIN#ok");
				ajoute(socket);
			}
			else
			{
				sprintf(reponse,"LOGIN#ko#Mauvais identifiants !");
				return false;
			}
		}

		//A completer / Modifier 
		//Commande:
		//« Login » 

		//Requête:
		//Login,
		//password,
		//nouveau
		//client ou pas

		//Réponse:
		//Oui ou non,
		//message (+
		//idClient) ou raison
	}

	// ***** LOGOUT *****************************************
	if (strcmp(ptr,"LOGOUT") == 0)
	{
		printf("\t[THREAD %p] LOGOUT\n",pthread_self());
		retire(socket);
		sprintf(reponse,"LOGOUT#ok");
		return false;
	}

	// ***** CONSULT ****************************************
	if (strcmp(ptr,"CONSULT") == 0)
	{
		//A completer

		//Commande:
		//« Consult » 

		//Requête:
		//idArticle 

		//Réponse:
		//idArticle ou -1,
		//intitule, stock,
		//prix, image


	}

	// ***** ACHAT ******************************************
	if (strcmp(ptr,"ACHAT") == 0)
	{
		//A completer
		//Commande:
		//« Achat » 

		//Requête :
		//idArticle,
		//quantité

		//Réponse:
		//idArticle ou -1,
		//quantité ou 0, prix

		
	}

	// ***** CADDIE *****************************************
	if (strcmp(ptr,"CADDIE") == 0)
	{
		//A completer
		//Commande:
        //« Caddie » 

		//Requête :
		//null

		//Réponse:
		//Contenu du
		//panier : (idArticle,
		//intitulé, quantité,
		//prix) × nombre
		//d’articles du panier



		
	}

	// ***** CANCEL *****************************************
	if (strcmp(ptr,"CANCEL") == 0)
	{
		//A completer
		//Commande:
		//« Cancel » 

		//Requête :
		//idArticle 

		//Réponse:
		//Oui ou non 


		
	}

	// ***** CANCEL_ALL *************************************
	if (strcmp(ptr,"CANCEL_ALL") == 0)
	{
		//A completer
		//Commande:
        //« Cancel All » 

		//Requête :
		//null

		//Réponse:
		//null
	
	}

	// ***** CONFIRMER **************************************
	if (strcmp(ptr,"CONFIRMER") == 0)
	{
		//A completer
		//Commande:
		//« Confirmer » 

		//Requête :
		//null 

		//Réponse:
		//Numéro de facture générée

	}


	
	return true;
}



//***** Traitement des requetes *************************************
/*bool OVESP_Login(const char* user,const char* password)
{
	
	//A completer
	//Actions / Explications
	//Vérification de l’existence et du mot
	//de passe du client / Création d’un
	//nouveau client dans la table clients


}*/

bool OVESP_Login(const char* user, const char* password, const bool nvClient)
{
    // Ici, vous devez insérer le code pour vérifier l'existence du client
    // dans la table "clients" et valider le mot de passe.
    // Si le client existe et le mot de passe est correct, retournez true.
    // Si le client n'existe pas et nvClient est vrai, créez un nouveau client dans la table "clients" et retournez true.
    // Sinon, retournez false.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL
	MYSQL_RES* result; // Résultat de la requête
	MYSQL_ROW row;     // Ligne de résultat

    mysql_conn = mysql_init(NULL);
    
    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    // Construisez et exécutez la requête SQL pour vérifier l'existence du client.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM clients WHERE login='%s' AND password='%s'", user, password);
    
    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        return false;
    }

    if (mysql_num_rows(result) > 0)
    {
        // Le client existe et le mot de passe est correct.
        mysql_free_result(result);
        mysql_close(mysql_conn);
        return true;
    }

    // Le client n'existe pas ou le mot de passe est incorrect.
    mysql_free_result(result);

    if (nvClient)
    {
        // Créez un nouveau client dans la table "clients" si nvClient est vrai.
        snprintf(query, sizeof(query), "INSERT INTO clients (login, password) VALUES ('%s', '%s')", user, password);

        if (mysql_query(mysql_conn, query))
        {
            fprintf(stderr, "Erreur lors de la création du nouveau client : %s\n", mysql_error(mysql_conn));
            mysql_close(mysql_conn);
            return false;
        }

        mysql_close(mysql_conn);
        return true;
    }

    mysql_close(mysql_conn);
    return false;
}

/*int OVESP_Consult(int idArticle)
{

	//A completer
	//Actions / Explications
	//Consultation d’un article en BD → si
	//article non trouvé, retour -1 au client


}*/

int OVESP_Consult(int idArticle)
{
    // Ici, vous devez insérer le code pour consulter un article en BD
    // en fonction de son ID (idArticle) et retourner les informations
    // de l'article. Si l'article est trouvé, retournez l'ID de l'article,
    // le nom, le stock, le prix et le chemin de l'image. Si l'article n'est
    // pas trouvé, retournez -1.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL
    MYSQL_RES* result; // Résultat de la requête
    MYSQL_ROW row;     // Ligne de résultat

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return -1;
    }

    // Construisez et exécutez la requête SQL pour consulter l'article.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM articles WHERE id=%d", idArticle);

    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return -1;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        return -1;
    }

    if (mysql_num_rows(result) > 0)
    {
        // L'article a été trouvé.
        row = mysql_fetch_row(result);
        int articleID = atoi(row[0]);
        const char* intitule = row[1];
        int stock = atoi(row[3]);
        float prix = atof(row[2]);
        const char* image = row[4];

        mysql_free_result(result);
        mysql_close(mysql_conn);

        // Vous pouvez utiliser ces valeurs pour construire la réponse.
        // Par exemple, vous pouvez utiliser sprintf pour construire la réponse.
        // Assurez-vous de libérer la mémoire appropriée pour la réponse.
        // Exemple : sprintf(reponse, "CONSULT#%d#%s#%d#%.2f#%s", articleID, intitule, stock, prix, image);

        return articleID;
    }

    // L'article n'a pas été trouvé.
    mysql_free_result(result);
    mysql_close(mysql_conn);
    return -1;
}

/*int OVESP_Achat(int idAricle, int quantite)
{

	//A completer
	//Actions / Explications
	//Si article non trouvé, retour -1. Si
	//trouvé mais que stock insuffisant,
	//retour d’une quantité 0 → Si ok, le
	//stock est mis à jour en BD et le
	//contenu du caddie est mémorisé au
	//niveau du serveur → actuellement
	//aucune action sur tables factures et
	//ventes

}*/

int OVESP_Achat(int idArticle, int quantite)
{
    // Ici, vous devez insérer le code pour gérer l'achat d'un article.
    // Vous devez vérifier si l'article existe et si la quantité est disponible en stock.
    // Si l'article est trouvé et la quantité est suffisante, mettez à jour le stock,
    // et retournez l'ID de l'article, la quantité achetée et le prix total.
    // Si l'article n'est pas trouvé ou la quantité n'est pas suffisante, retournez -1.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL
    MYSQL_RES* result; // Résultat de la requête
    MYSQL_ROW row;     // Ligne de résultat

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return -1;
    }

    // Construisez et exécutez la requête SQL pour récupérer l'article.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM articles WHERE id=%d", idArticle);

    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return -1;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        return -1;
    }

    if (mysql_num_rows(result) > 0)
    {
        // L'article a été trouvé.
        row = mysql_fetch_row(result);
        int articleID = atoi(row[0]);
        const char* intitule = row[1];
        int stock = atoi(row[3]);
        float prix = atof(row[2]);

        if (stock >= quantite)
        {
            // Le stock est suffisant pour l'achat.
            // Mettez à jour le stock dans la base de données.
            int nouveauStock = stock - quantite;
            snprintf(query, sizeof(query), "UPDATE articles SET stock=%d WHERE id=%d", nouveauStock, articleID);
            if (mysql_query(mysql_conn, query))
            {
                fprintf(stderr, "Erreur lors de la mise à jour du stock : %s\n", mysql_error(mysql_conn));
                mysql_free_result(result);
                mysql_close(mysql_conn);
                return -1;
            }

            mysql_free_result(result);
            mysql_close(mysql_conn);

            // Retournez les informations de l'achat.
            // Vous pouvez utiliser ces valeurs pour construire la réponse.
            // Par exemple, vous pouvez utiliser sprintf pour construire la réponse.
            // Assurez-vous de libérer la mémoire appropriée pour la réponse.
            // Exemple : sprintf(reponse, "ACHAT#%d#%d#%.2f", articleID, quantite, quantite * prix);

            return articleID;
        }
    }

    // L'article n'a pas été trouvé ou la quantité n'est pas suffisante.
    mysql_free_result(result);
    mysql_close(mysql_conn);
    return -1;
}

/*bool OVESP_Caddie()
{

	//A completer
	//Actions / Explications
	//Retourne l’entièreté du contenu du
	//caddie au client

}*/

bool OVESP_Caddie()
{
    // Ici, vous devez insérer le code pour récupérer le contenu du caddie
    // du client actuellement connecté. Vous devez parcourir les articles
    // dans le caddie, récupérer les informations nécessaires depuis la base
    // de données et construire une réponse contenant ces informations.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL
    MYSQL_RES* result; // Résultat de la requête
    MYSQL_ROW row;     // Ligne de résultat

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    // Construisez et exécutez la requête SQL pour récupérer le contenu du caddie.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM caddie WHERE idClient=%d", idClient); // Vous devez remplacer idClient par l'ID du client actuellement connecté.

    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        return false;
    }

    // Vous devez parcourir les résultats et construire la réponse.
    // Par exemple, vous pouvez utiliser une boucle pour parcourir les lignes
    // de résultat et ajouter les informations de chaque article au format de la réponse.

    // Assurez-vous de libérer la mémoire appropriée pour la réponse.

    // Exemple de construction de la réponse :
    // char reponse[1024];
    // snprintf(reponse, sizeof(reponse), "CADDIE#%s", contenuDuCaddie);

    // Vous devez également gérer le cas où le caddie est vide.

    // Libérez la mémoire et fermez la connexion MySQL.
    mysql_free_result(result);
    mysql_close(mysql_conn);

    return true;
}

/*bool OVESP_Cancel(int idArticle)
{

	//A completer
	//Actions / Explications
	//Supprime un article du caddie et met à
	//jour à la BD

}*/

bool OVESP_Cancel(int idArticle)
{
    // Ici, vous devez insérer le code pour annuler un article du caddie
    // du client actuellement connecté. Vous devez supprimer l'article spécifié
    // du caddie et mettre à jour la base de données si nécessaire.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    // Vous devez construire la requête SQL pour supprimer l'article spécifié du caddie.
    // Assurez-vous également de mettre à jour la base de données pour refléter cette modification.

    // Par exemple :
    // char query[256];
    // snprintf(query, sizeof(query), "DELETE FROM caddie WHERE idClient=%d AND idArticle=%d", idClient, idArticle);
    // Si vous utilisez une transaction, assurez-vous de valider la transaction après cette opération.

    // Libérez la mémoire et fermez la connexion MySQL.
    mysql_close(mysql_conn);

    return true;
}


/*bool OVESP_Cancel_All()
{

	//A completer
	//Actions / Explications
	//Supprime tous les articles du caddie et
	//met à jour la BD

}*/

bool OVESP_Cancel_All()
{
    // Ici, vous devez insérer le code pour annuler tous les articles du caddie
    // du client actuellement connecté. Vous devez supprimer tous les articles
    // du caddie et mettre à jour la base de données si nécessaire.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    // Vous devez construire la requête SQL pour supprimer tous les articles du caddie du client.
    // Assurez-vous également de mettre à jour la base de données pour refléter cette modification.

    // Par exemple :
    // char query[256];
    // snprintf(query, sizeof(query), "DELETE FROM caddie WHERE idClient=%d", idClient);
    // Si vous utilisez une transaction, assurez-vous de valider la transaction après cette opération.

    // Libérez la mémoire et fermez la connexion MySQL.
    mysql_close(mysql_conn);

    return true;
}

/*bool OVESP_Confirmer()
{

	//A completer
	//Actions / Explications
	//Création d’une facture et BD et ajout
	//des éléments du caddie dans la BD

}*/

bool OVESP_Confirmer()
{
    // Ici, vous devez insérer le code pour confirmer la commande et créer une facture.
    // Vous devez ajouter les éléments du caddie à la table des ventes, générer un numéro
    // de facture, calculer le montant total, créer la facture dans la base de données, etc.

    // Par exemple (à adapter à votre utilisation réelle de MySQL) :
    // Remplacez "mysql_query" par la méthode que vous utilisez pour exécuter des requêtes MySQL.

    MYSQL* mysql_conn; // Connexion MySQL

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        return false;
    }

    // Vous devez construire les requêtes SQL nécessaires pour créer la facture et les ventes associées.
    // Assurez-vous de gérer correctement les transactions et de mettre à jour la base de données.

    // Par exemple :
    // char query[256];
    // // Insérer les éléments du caddie dans la table des ventes (ventes)
    // snprintf(query, sizeof(query), "INSERT INTO ventes (idFacture, idArticle, quantite) SELECT F.idFacture, C.idArticle, C.quantite FROM factures F JOIN caddie C ON F.idClient = C.idClient WHERE F.idClient = %d", idClient);
    // if (mysql_query(mysql_conn, query))
    // {
    //     fprintf(stderr, "Erreur lors de l'exécution de la requête pour insérer les ventes : %s\n", mysql_error(mysql_conn));
    //     // En cas d'erreur, annulez la transaction si nécessaire.
    //     // ...
    //     mysql_close(mysql_conn);
    //     return false;
    // }

    // // Calculer le montant total
    // snprintf(query, sizeof(query), "SELECT SUM(A.prix * C.quantite) FROM articles A JOIN caddie C ON A.id = C.idArticle WHERE C.idClient = %d", idClient);
    // if (mysql_query(mysql_conn, query))
    // {
    //     fprintf(stderr, "Erreur lors du calcul du montant total : %s\n", mysql_error(mysql_conn));
    //     // En cas d'erreur, annulez la transaction si nécessaire.
    //     // ...
    //     mysql_close(mysql_conn);
    //     return false;
    // }
    // // Récupérer le montant total depuis le résultat de la requête
    // // et effectuer d'autres opérations de création de facture.

    // Libérez la mémoire et fermez la connexion MySQL.
    mysql_close(mysql_conn);

    return true;
}


//***** Gestion de l'état du protocole ******************************
int estPresent(int socket)
{
	int indice = -1;
	pthread_mutex_lock(&mutexClients);
	for(int i=0 ; i<nbClients ; i++)
		if (clients[i] == socket) { indice = i; break; }
	pthread_mutex_unlock(&mutexClients);
	return indice;
}


void ajoute(int socket)
{
	pthread_mutex_lock(&mutexClients);
	clients[nbClients] = socket;
	nbClients++;
	pthread_mutex_unlock(&mutexClients);
}


void retire(int socket)
{
	int pos = estPresent(socket);
	if (pos == -1) return;
	pthread_mutex_lock(&mutexClients);
	for (int i=pos ; i<=nbClients-2 ; i++)
		clients[i] = clients[i+1];
	nbClients--;
	pthread_mutex_unlock(&mutexClients);
}


//***** Fin prématurée **********************************************
void OVESP_Close()
{
	pthread_mutex_lock(&mutexClients);
	for (int i=0 ; i<nbClients ; i++)
	close(clients[i]);
	pthread_mutex_unlock(&mutexClients);
}