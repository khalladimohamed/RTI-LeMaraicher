#ifndef OVESPSERVEUR_H
#define OVESPSERVEUR_H
#define NB_MAX_CLIENTS 100


typedef struct
{
    int idArticle;
    char intitule[20];
    int quantite;
    float prix;
} CADDIE;


bool  OVESP(char* requete, char* reponse, int socket, MYSQL* mysql_conn, CADDIE* caddie, int* nombreArticlesCaddie, float* montantTotalCaddie, int* idClientCourrant);
bool  OVESP_Login(const char* user,const char* password, const int nvClient, MYSQL* mysql_conn, int* idClientCourrant);
int   OVESP_Consult(int idArticle, char* reponse, MYSQL* mysql_conn);
int   OVESP_Achat(int idAricle, int quantite, char* reponse, MYSQL* mysql_conn, CADDIE* caddie, int* nombreArticlesCaddie, float* montantTotalCaddie);
bool  OVESP_Caddie(char* reponse, CADDIE* caddie, int* nombreArticlesCaddie);
bool  OVESP_Cancel(int idArticle, MYSQL* mysql_conn, CADDIE* caddie, int* nombreArticlesCaddie, float* montantTotalCaddie);
bool  OVESP_Cancel_All(MYSQL* mysql_conn, CADDIE* caddie, int* nombreArticlesCaddie, float* montantTotalCaddie);
int   OVESP_Confirmer(int idClient, char* reponse, MYSQL* mysql_conn, CADDIE* caddie, int* nombreArticlesCaddie, float* montantTotalCaddie);
void  OVESP_Close();

#endif