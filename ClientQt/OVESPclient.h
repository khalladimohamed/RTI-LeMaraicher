#ifndef OVESPCLIENT_H
#define OVESPCLIENT_H


int   OVESP_Login(const char* user,const char* password, const int nvClient);
int   OVESP_Consult(int idArticle, char* reponse);
int   OVESP_Achat(int idAricle, int quantite, char* reponse);
bool  OVESP_Caddie(int idClient, char* reponse);
bool  OVESP_Cancel(int idArticle);
bool  OVESP_Cancel_All();
bool  OVESP_Confirmer();
void  OVESP_Close();

void Echange(char* requete, char* reponse);
void SMOP_Logout();
void SMOP_Operation(char op,int a,int b);

#endif