#ifndef OVESPSERVEUR_H
#define OVESPSERVEUR_H
#define NB_MAX_CLIENTS 100
#define MAX_TAILLE_CADDIE 50


bool  OVESP(char* requete, char* reponse, int socket);
bool  OVESP_Login(const char* user,const char* password, const bool nvClient);
int   OVESP_Consult(int idArticle, char* reponse);
int   OVESP_Achat(int idAricle, int quantite, char* reponse);
bool  OVESP_Caddie(char* reponse);
bool  OVESP_Cancel(int idArticle);
bool  OVESP_Cancel_All();
bool  OVESP_Confirmer();
void  OVESP_Close();

#endif