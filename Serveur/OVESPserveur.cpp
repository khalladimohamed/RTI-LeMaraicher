#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mysql.h>
#include <time.h>

#include "OVESPserveur.h"
#include "../lib/LibSockets.h"



//***** BD **********************************************************
pthread_mutex_t mysqlMutex = PTHREAD_MUTEX_INITIALIZER;


//***** Etat du protocole : liste des clients loggés ****************
int clients[NB_MAX_CLIENTS];
int nbClients = 0;
int idClientCourrant = -1;
int estPresent(int socket);
void ajoute(int socket);
void retire(int socket);

pthread_mutex_t mutexClients = PTHREAD_MUTEX_INITIALIZER;


//***** CADDIE ******************************************************
typedef struct
{
    int idArticle;
    char intitule[20];
    int quantite;
    float prix;
} CADDIE;

CADDIE caddie[MAX_TAILLE_CADDIE]; 
int nombreArticlesCaddie = 0;    
float montantTotalCaddie = 0.0;


//***** Parsing de la requete et creation de la reponse *************
bool OVESP(char* requete, char* reponse, int socket)
{

	// ***** Récupération nom de la requete *****************
	char *ptr = strtok(requete,"#");

	// ***** LOGIN ******************************************
	if (strcmp(ptr,"LOGIN") == 0)
	{
		char user[50], password[50];
        int newUser;
		strcpy(user,strtok(NULL,"#"));
		strcpy(password,strtok(NULL,"#"));
        newUser = atoi(strtok(NULL,"#"));
		
        printf("\t[THREAD %p] LOGIN de %s\n",pthread_self(),user);
		
        if (estPresent(socket) >= 0) // client déjà loggé
		{
			sprintf(reponse,"LOGIN#ko#Client déjà loggé !");
			return false;
		}
		else
		{
			if (OVESP_Login(user,password, newUser))
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
	}

	// ***** LOGOUT *****************************************
	if (strcmp(ptr,"LOGOUT") == 0)
	{
		printf("\t[THREAD %p] LOGOUT\n",pthread_self()); 
        if(nombreArticlesCaddie != 0)
			OVESP_Cancel_All();
        retire(socket);
		sprintf(reponse,"LOGOUT#ok");
		return false;
	}

	// ***** CONSULT ****************************************
	if (strcmp(ptr,"CONSULT") == 0)
	{
		int idArticle;
        idArticle = atoi(strtok(NULL,"#"));
        
        printf("\t[THREAD %p] CONSULT %d\n",pthread_self(), idArticle);
        
        if (estPresent(socket) == -1)
        {
            sprintf(reponse,"CONSULT#ko#Client non loggé !");
            return false;
        }    
        else
        {
            
            int resultat = OVESP_Consult(idArticle, reponse);

            if(resultat == -1) 
            { 
                sprintf(reponse,"CONSULT#ko#Article non trouve !");
                return false; 
            }
        }
	}

	// ***** ACHAT ******************************************
	if (strcmp(ptr,"ACHAT") == 0)
	{
		int idArticle, quantite;
        idArticle = atoi(strtok(NULL,"#"));
        quantite = atoi(strtok(NULL,"#"));

        printf("\t[THREAD %p] ACHAT %d %d\n",pthread_self(), idArticle, quantite);

		if (estPresent(socket) == -1)
        {
            sprintf(reponse,"ACHAT#ko#Client non loggé !");
            return false;
        }    
        else
        {
            
            int resultat = OVESP_Achat(idArticle, quantite, reponse);

            if(resultat == -1) 
            { 
                sprintf(reponse,"ACHAT#ko#Article non trouve !");
                return false; 
            }
            else
            {
                if(resultat == 0)
                {
                    sprintf(reponse,"ACHAT#ko#Stock unsufisant !");
                    return false;
                }
                
            }
        }
	}

	// ***** CADDIE *****************************************
	if (strcmp(ptr,"CADDIE") == 0)
	{
		if (estPresent(socket) == -1)
        {
            sprintf(reponse,"CADDIE#ko#Client non loggé !");
            return false;
        }    
        else
        {
            
            bool resultat = OVESP_Caddie(reponse);

            if(!resultat) 
                return false; 
        }
		
	}

	// ***** CANCEL *****************************************
	if (strcmp(ptr,"CANCEL") == 0)
	{
		int idArticle;
        idArticle = atoi(strtok(NULL,"#"));

        printf("\t[THREAD %p] CANCEL %d\n",pthread_self(), idArticle);

        if (estPresent(socket) == -1)
        {
            sprintf(reponse,"CANCEL#ko#Client non loggé !");
            return false;
        }    
        else
        {
            
            bool resultat = OVESP_Cancel(idArticle);

            if(resultat)
            {
                sprintf(reponse,"CANCEL#ok");
            }
            else
            {
                sprintf(reponse,"CANCEL#ko#L'article n'a pas été trouvé dans le caddie");
                return false; 
            }
        }
	}

	// ***** CANCEL_ALL *************************************
	if (strcmp(ptr,"CANCEL_ALL") == 0)
	{
		printf("\t[THREAD %p] CANCEL_ALL\n",pthread_self());

        if (estPresent(socket) == -1)
        {
            sprintf(reponse,"CANCEL_ALL#ko#Client non loggé !");
            return false;
        }    
        else
        {
            
            bool resultat = OVESP_Cancel_All();

            if(resultat)
            {
                sprintf(reponse,"CANCEL_ALL#ok");
            }
            else
            {
                sprintf(reponse,"CANCEL_ALL#ko#Le caddie est vide !");
                return false; 
            }
        }

	}

	// ***** CONFIRMER **************************************
	if (strcmp(ptr,"CONFIRMER") == 0)
	{
		printf("\t[THREAD %p] CONFIRMER\n",pthread_self());

        if (estPresent(socket) == -1)
        {
            sprintf(reponse,"CONFIRMER#ko#Client non loggé !");
            return false;
        }    
        else
        {
            
            int resultat = OVESP_Confirmer(idClientCourrant, reponse);

            if(resultat == -1)
            {
                sprintf(reponse,"CONFIRMER#ko#Problème avec la facture !");
            }
        }
	}

	return true;
}


bool OVESP_Login(const char* user, const char* password, const int nvClient)
{
    // Verrouiller le mutex pour protéger l'accès à la base de données MySQL
    pthread_mutex_lock(&mysqlMutex);

    MYSQL* mysql_conn; // Connexion MySQL
	MYSQL_RES* result; // Résultat de la requête
	MYSQL_ROW row;     // Ligne de résultat

    mysql_conn = mysql_init(NULL);
    
    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost","Student","PassStudent1_","PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        // Déverrouiller le mutex après avoir terminé les opérations sur la base de données
        pthread_mutex_unlock(&mysqlMutex);
        return false;
    }

    // Construisez et exécutez la requête SQL pour vérifier l'existence du client.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM clients WHERE login='%s' AND password='%s'", user, password);
    
    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return false;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return false;
    }

    if (mysql_num_rows(result) > 0)
    {
        // Le client existe et le mot de passe est correct.
        row = mysql_fetch_row(result);
        idClientCourrant = atoi(row[0]);
        mysql_free_result(result);
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
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
            pthread_mutex_unlock(&mysqlMutex);
            return false;
        }

        //pour recuperer la derniere valeur inserer avec AUTO INCREMENT 
        idClientCourrant = (int)mysql_insert_id(mysql_conn);

        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return true;
    }

    mysql_close(mysql_conn);
    pthread_mutex_unlock(&mysqlMutex);
    return false;
}


int OVESP_Consult(int idArticle, char* reponse)
{
    pthread_mutex_lock(&mysqlMutex);

    MYSQL* mysql_conn; 
    MYSQL_RES* result; 
    MYSQL_ROW row;    

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost","Student","PassStudent1_","PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    // Construisez et exécutez la requête SQL pour consulter l'article.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM articles WHERE id=%d", idArticle);

    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    if (mysql_num_rows(result) > 0)
    {
        // L'article a été trouvé.
        row = mysql_fetch_row(result);
        int articleID = atoi(row[0]);
        const char* intitule = row[1];
        float prix = atof(row[2]);
        int stock = atoi(row[3]);
        const char* image = row[4];

        mysql_free_result(result);
        mysql_close(mysql_conn);

        
        sprintf(reponse, "CONSULT#ok#%s#%.2f#%d#%s", intitule, prix, stock, image);

        pthread_mutex_unlock(&mysqlMutex);
        return articleID;
    }

    // L'article n'a pas été trouvé.
    mysql_free_result(result);
    mysql_close(mysql_conn);
    pthread_mutex_unlock(&mysqlMutex);
    return -1;
}


int OVESP_Achat(int idArticle, int quantite, char* reponse)
{
    pthread_mutex_lock(&mysqlMutex);

    MYSQL* mysql_conn; 
    MYSQL_RES* result; 
    MYSQL_ROW row;

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost","Student","PassStudent1_","PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    // Construisez et exécutez la requête SQL pour récupérer l'article.
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM articles WHERE id=%d", idArticle);

    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'exécution de la requête : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
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
            int nouveauStock = stock - quantite;
            snprintf(query, sizeof(query), "UPDATE articles SET stock=%d WHERE id=%d", nouveauStock, articleID);
            if (mysql_query(mysql_conn, query))
            {
                fprintf(stderr, "Erreur lors de la mise à jour du stock : %s\n", mysql_error(mysql_conn));
                mysql_free_result(result);
                mysql_close(mysql_conn);
                pthread_mutex_unlock(&mysqlMutex);
                return -1;
            }

            mysql_free_result(result);
            mysql_close(mysql_conn);

            // Ajoutez l'article au caddie
        	if (nombreArticlesCaddie < MAX_TAILLE_CADDIE)
        	{
            	caddie[nombreArticlesCaddie].idArticle = articleID;
            	strncpy(caddie[nombreArticlesCaddie].intitule, intitule, sizeof(caddie[nombreArticlesCaddie].intitule));
            	caddie[nombreArticlesCaddie].quantite = quantite;
            	caddie[nombreArticlesCaddie].prix = quantite * prix;
            	nombreArticlesCaddie++;
        	}

        	// Mise à jour du montant total du caddie
        	montantTotalCaddie += quantite * prix;
            printf("ACHAT#ok#%d#%s#%d#%.2f", articleID, intitule, quantite, prix);
            sprintf(reponse, "ACHAT#ok#%d#%s#%d#%.2f", articleID, intitule, quantite, prix);

            pthread_mutex_unlock(&mysqlMutex);
            return articleID;
        }
        else
        {
            pthread_mutex_unlock(&mysqlMutex);
            return 0;
        }
    }

    // L'article n'a pas été trouvé ou la quantité n'est pas suffisante.
    mysql_free_result(result);
    mysql_close(mysql_conn);
    pthread_mutex_unlock(&mysqlMutex);
    return -1;
}


bool OVESP_Caddie(char* reponse)
{
    // Construire la réponse en fonction des articles dans le caddie
    char contenuDuCaddie[200]; 
    if (nombreArticlesCaddie > 0)
    {
        snprintf(contenuDuCaddie, sizeof(contenuDuCaddie), "CADDIE#ok");
        for (int i = 0; i < nombreArticlesCaddie; i++)
        {
            char articleInfo[256];
            snprintf(articleInfo, sizeof(articleInfo), "%d,%s,%d,%.2f;", caddie[i].idArticle, caddie[i].intitule, caddie[i].quantite, caddie[i].prix);
            strncat(contenuDuCaddie, articleInfo, sizeof(contenuDuCaddie));
        } 
    }
    else
    {
        snprintf(contenuDuCaddie, sizeof(contenuDuCaddie), "CADDIE#ko#Aucun article dans le caddie;");
        return false;
    }

    // Copier le contenu du caddie dans la réponse
    snprintf(reponse, sizeof(reponse), "%s", contenuDuCaddie);

    return true;
}


bool OVESP_Cancel(int idArticle)
{
    pthread_mutex_lock(&mysqlMutex);

    MYSQL* mysql_conn; 
    MYSQL_RES* result;
    MYSQL_ROW row;

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost","Student","PassStudent1_","PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return false;
    }

    
    // 1. Rechercher l'article dans le caddie en fonction de son ID.
    int indiceArticleDansCaddie = -1;
    for (int i = 0; i < nombreArticlesCaddie; i++)
    {
        if (caddie[i].idArticle == idArticle)
        {
            indiceArticleDansCaddie = i;
            break;
        }
    }

    char query[256];

    if (indiceArticleDansCaddie != -1)
    {
        // 2. Metter à jour la base de données avec la nouvelle quantité de l'article.
        int nouvelleQuantite = caddie[indiceArticleDansCaddie].quantite;
        snprintf(query, sizeof(query), "UPDATE articles SET stock = stock + %d WHERE id = %d", nouvelleQuantite, idArticle);

        if (mysql_query(mysql_conn, query))
        {
            fprintf(stderr, "Erreur lors de la mise à jour du stock dans la base de données : %s\n", mysql_error(mysql_conn));
            mysql_close(mysql_conn);
            pthread_mutex_unlock(&mysqlMutex);
            return false;
        }

        // 3. Supprimer l'article du caddie.
        for (int i = indiceArticleDansCaddie; i < nombreArticlesCaddie - 1; i++)
        {
            caddie[i] = caddie[i + 1];
        }
        nombreArticlesCaddie--;
    }
    else
    {
        pthread_mutex_unlock(&mysqlMutex);
        return false;
    }
    
    // Libérez la mémoire et fermez la connexion MySQL.
    mysql_close(mysql_conn);

    pthread_mutex_unlock(&mysqlMutex);
    return true;
}


bool OVESP_Cancel_All()
{
    pthread_mutex_lock(&mysqlMutex);

    MYSQL* mysql_conn; 
    MYSQL_RES* result;
    MYSQL_ROW row;

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "Student", "PassStudent1_", "PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return false;
    }

    char query[256];

    // Parcourir le caddie et mettre à jour les stocks dans la base de données.
    for (int i = 0; i < nombreArticlesCaddie; i++)
    {
        int idArticle = caddie[i].idArticle;
        int quantite = caddie[i].quantite;
        printf("test cancel_all quantite : %d / idArticle : %d\n", quantite, idArticle);
        // Mettre à jour le stock dans la base de données.
        snprintf(query, sizeof(query), "UPDATE articles SET stock = stock + %d WHERE id = %d", quantite, idArticle);

        if (mysql_query(mysql_conn, query))
        {
            fprintf(stderr, "Erreur lors de la mise à jour du stock dans la base de données : %s\n", mysql_error(mysql_conn));
            mysql_close(mysql_conn);
            pthread_mutex_unlock(&mysqlMutex);
            return false;
        }
    }

    // Réinitialiser le caddie en le vidant.
    nombreArticlesCaddie = 0;

    // Libérez la mémoire et fermez la connexion MySQL.
    mysql_close(mysql_conn);

    pthread_mutex_unlock(&mysqlMutex);
    return true;
}

int OVESP_Confirmer(int idClient, char* reponse)
{
    pthread_mutex_lock(&mysqlMutex);

    MYSQL* mysql_conn; 
    MYSQL_RES* result; 
    MYSQL_ROW row;

    mysql_conn = mysql_init(NULL);

    // Établissez la connexion à la base de données MySQL.
    if (mysql_real_connect(mysql_conn, "localhost", "Student", "PassStudent1_", "PourStudent", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "Erreur de connexion MySQL : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    char query[256];

    // Vérifiez si la table facture est vide.
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM factures");
    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de la vérification de la table facture : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    result = mysql_store_result(mysql_conn);
    if (result == NULL)
    {
        fprintf(stderr, "Aucun résultat de la requête.\n");
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    row = mysql_fetch_row(result);

    // Insérez la facture dans la table facture.
    snprintf(query, sizeof(query), "INSERT INTO factures (idClient, date, montant, paye) VALUES (%d, DATE_FORMAT(NOW(), '%%Y-%%m-%%d'), %.2f, 0)", idClient, montantTotalCaddie);


    if (mysql_query(mysql_conn, query))
    {
        fprintf(stderr, "Erreur lors de l'insertion de la facture : %s\n", mysql_error(mysql_conn));
        mysql_close(mysql_conn);
        pthread_mutex_unlock(&mysqlMutex);
        return -1;
    }

    //pour recuperer la derniere valeur inserer avec AUTO INCREMENT 
    int numeroFacture = (int)mysql_insert_id(mysql_conn);

    // Parcourir le caddie et insérer chaque élément dans la table vente.
    for (int i = 0; i < nombreArticlesCaddie; i++)
    {
        int idArticle = caddie[i].idArticle;
        int quantite = caddie[i].quantite;
        float prixUnitaire = caddie[i].prix;

        // Insérer l'élément du caddie dans la table vente.
        snprintf(query, sizeof(query), "INSERT INTO ventes (idFacture, idArticle, quantite) VALUES (%d, %d, %d)", numeroFacture, idArticle, quantite);

        if (mysql_query(mysql_conn, query))
        {
            fprintf(stderr, "Erreur lors de l'insertion de l'élément du caddie dans la table vente : %s\n", mysql_error(mysql_conn));
            mysql_close(mysql_conn);
            pthread_mutex_unlock(&mysqlMutex);
            return -1;
        }
    }

    // Réinitialiser le caddie en le vidant.
    nombreArticlesCaddie = 0;
    // Réinitialiser le montant total.
    montantTotalCaddie = 0.0; 

    // Retourner le numéro de facture généré.
    sprintf(reponse, "CONFIRMER#ok#%d", numeroFacture);

    // Libérer la mémoire et fermez la connexion MySQL.
    mysql_close(mysql_conn);

    pthread_mutex_unlock(&mysqlMutex);
    return numeroFacture;
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