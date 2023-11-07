#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <time.h>
#include <string.h>

typedef struct
{
  int   id;
  char  intitule[20];
  float prix;
  int   stock;  
  char  image[20];
} ARTICLE;

typedef struct
{
  int id;
  char login[20];
  char password[20];
} CLIENT;

typedef struct
{
  int id;
  char login[20];
  char password[20];
} EMPLOYE;

typedef struct
{
  int id;
  int idClient;
  char date[20];
  float montant;
  int paye;
} FACTURE;

typedef struct
{
  int idFacture;
  int idArticle;
  int quantite;
} VENTE;


ARTICLE Elm[] = 
{
  {1,"carottes",2.16f,9,"carottes.jpg"},
  {2,"cerises",9.75f,8,"cerises.jpg"},
  {3,"artichaut",1.62f,15,"artichaut.jpg"},
  {4,"bananes",2.6f,8,"bananes.jpg"},
  {5,"champignons",10.25f,4,"champignons.jpg"},
  {6,"concombre",1.17f,5,"concombre.jpg"},
  {7,"courgette",1.17f,14,"courgette.jpg"},
  {8,"haricots",10.82f,7,"haricots.jpg"},
  {9,"laitue",1.62f,10,"laitue.jpg"},
  {10,"oranges",3.78f,23,"oranges.jpg"},
  {11,"oignons",2.12f,4,"oignons.jpg"},
  {12,"nectarines",10.38f,6,"nectarines.jpg"},
  {13,"peches",8.48f,11,"peches.jpg"},
  {14,"poivron",1.29f,13,"poivron.jpg"},
  {15,"pommes de terre",2.17f,25,"pommesDeTerre.jpg"},
  {16,"pommes",4.00f,26,"pommes.jpg"},
  {17,"citrons",4.44f,11,"citrons.jpg"},
  {18,"ail",1.08f,14,"ail.jpg"},
  {19,"aubergine",1.62f,17,"aubergine.jpg"},
  {20,"echalotes",6.48f,13,"echalotes.jpg"},
  {21,"tomates",5.49f,22,"tomates.jpg"}
};

CLIENT Clients[] = {
    {1, "client1", "password1"},
    {2, "client2", "password2"},
};

EMPLOYE Employes[] = {
    {1, "employe1", "password1"},
    {2, "employe2", "password2"},
};

FACTURE Factures[] = {
    {1, 1, "2023-09-26", 20.5, 1},
    {2, 2, "2023-09-27", 30.0, 0},
};

VENTE Ventes[] = {
    {1, 1, 3},
    {1, 2, 2},
    {2, 3, 5},
};

int main(int argc,char *argv[])
{
  // Connection a MySql
  printf("Connection a la BD...\n");
  MYSQL* connexion = mysql_init(NULL);
  mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

  // Creation de la table articles
  printf("Creation de la table articles...\n");
  mysql_query(connexion,"drop table articles;"); // au cas ou elle existerait deja
  mysql_query(connexion,"create table articles (id INT(4) auto_increment primary key, intitule varchar(20),prix FLOAT(4),stock INT(4),image varchar(20));");

  // Création de la table clients
  printf("Création de la table clients...\n");
  mysql_query(connexion, "drop table clients;"); // Au cas où elle existerait déjà
  mysql_query(connexion, "create table clients (id INT(4) auto_increment primary key, login varchar(20), password varchar(20));");

  // Création de la table employes
  printf("Création de la table employes...\n");
  mysql_query(connexion, "drop table employes;"); // Au cas où elle existerait déjà
  mysql_query(connexion, "create table employes (id INT(4) auto_increment primary key, login varchar(20), password varchar(20));");

  // Création de la table factures
  printf("Création de la table factures...\n");
  mysql_query(connexion, "drop table factures;"); // Au cas où elle existerait déjà
  mysql_query(connexion, "create table factures (id INT(4) auto_increment primary key, idClient INT(4), date varchar(20), montant FLOAT(4), paye INT(1));");

  // Création de la table ventes
  printf("Création de la table ventes...\n");
  mysql_query(connexion, "drop table ventes;"); // Au cas où elle existerait déjà
  mysql_query(connexion, "create table ventes (idFacture INT(4), idArticle INT(4), quantite INT(4));");

  // Ajout de tuples dans la table articles
  printf("Ajout de 21 articles la table articles...\n");
  char requete[256];
  for (int i=0 ; i<21 ; i++)
  {
	  sprintf(requete,"insert into articles values (NULL,'%s',%f,%d,'%s');",Elm[i].intitule,Elm[i].prix,Elm[i].stock,Elm[i].image);
	  mysql_query(connexion,requete);
  }

  // Ajout de tuples dans la table clients
  printf("Ajout de clients...\n");
  for (int i = 0; i < sizeof(Clients) / sizeof(CLIENT); i++)
  {
    sprintf(requete, "insert into clients values (%d, '%s', '%s');", Clients[i].id, Clients[i].login, Clients[i].password);
    mysql_query(connexion, requete);
  }

  // Ajout de tuples dans la table employes
  printf("Ajout de employes...\n");
  for (int i = 0; i < sizeof(Employes) / sizeof(EMPLOYE); i++)
  {
    sprintf(requete, "insert into employes values (%d, '%s', '%s');", Employes[i].id, Employes[i].login, Employes[i].password);
    mysql_query(connexion, requete);
  }

  // Ajout de tuples dans la table factures
  printf("Ajout de factures...\n");
  for (int i = 0; i < sizeof(Factures) / sizeof(FACTURE); i++)
  {
    sprintf(requete, "insert into factures values (%d, %d, '%s', %f, %d);", Factures[i].id, Factures[i].idClient, Factures[i].date, Factures[i].montant, Factures[i].paye);
    mysql_query(connexion, requete);
  }

  // Ajout de tuples dans la table ventes
  printf("Ajout de ventes...\n");
  for (int i = 0; i < sizeof(Ventes) / sizeof(VENTE); i++)
  {
    sprintf(requete, "insert into ventes values (%d, %d, %d);", Ventes[i].idFacture, Ventes[i].idArticle, Ventes[i].quantite);
    mysql_query(connexion, requete);
  }


  // Deconnection de la BD
  mysql_close(connexion);
  exit(0);
}
