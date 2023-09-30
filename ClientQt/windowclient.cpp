#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include "../lib/LibSockets.h"
using namespace std;

extern WindowClient *w;

#define REPERTOIRE_IMAGES "images/"

int sClient;

void HandlerSIGINT(int s);

bool clientLogged = false;

int indiceArticle = 1;

bool   OVESP_Login(const char* user,const char* password, const int nvClient);
void   OVESP_Logout();
void   OVESP_Consult(int idArticle);
void   OVESP_Achat(int idArticle, int quantite);
void   OVESP_Cancel(int idArticle);
void   OVESP_Cancel_All();
void   OVESP_Confirmer();

void   Echange(char* requete, char* reponse);

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    ui->pushButtonPayer->setText("Confirmer achat");
    setPublicite("!!! Bienvenue sur le Maraicher en ligne !!!");

    // Exemples à supprimer
    setArticle("pommes",5.53,18,"pommes.jpg");
    ajouteArticleTablePanier("cerises",8.96,2);

    // Armement des signaux
    struct sigaction A;
    A.sa_flags = 0;
    sigemptyset(&A.sa_mask);
    A.sa_handler = HandlerSIGINT;
    if (sigaction(SIGINT,&A,NULL) == -1)
    {
      perror("Erreur de sigaction");
      exit(1);
    }

    // Connexion sur le serveur
    char serverIP[] = "192.168.109.130"; //Adresse IP du serveur / broadcast 192.168.109.255
    int serverPort = 50000; //Numéro de port du serveur

    if ((sClient = ClientSocket(serverIP, serverPort)) == -1)
    {
      perror("Erreur de ClientSocket");
      exit(1);
    }
    printf("Connecte sur le serveur.\n");
}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
  //int idClient = OVESP_Login(getNom(), getMotDePasse(), isNouveauClientChecked());

  if (OVESP_Login(getNom(), getMotDePasse(), isNouveauClientChecked()))
  {
    clientLogged = true;
    loginOK();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
  if(clientLogged)
  {
    OVESP_Logout();
    clientLogged = false;
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
  if(clientLogged)
  {
    if(indiceArticle > 0 && indiceArticle < 22)
    {
      indiceArticle++;
      OVESP_Consult(indiceArticle);
    }
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
  if(clientLogged)
  {
    if(indiceArticle > 0 && indiceArticle < 22)
    {
      indiceArticle--;
      OVESP_Consult(indiceArticle);
    }
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
  if(clientLogged)
  {
    OVESP_Achat(indiceArticle, getQuantite());
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
  if(clientLogged)
  {
    OVESP_Cancel(getIndiceArticleSelectionne());
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
  if(clientLogged)
  {
    OVESP_Cancel_All();
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
  if(clientLogged)
  {
    OVESP_Confirmer();
  }
  else
  {
    dialogueErreur("Erreur connection", "Le client est non connecter !");
  }
}

//***** Fin de connexion ********************************************
void HandlerSIGINT(int s)
{
  printf("\nArret du client.\n");
  OVESP_Logout();
  close(sClient);
  exit(0);
}

//***** Gestion du protocole OVESP ***********************************
bool OVESP_Login(const char* user, const char* password, const int newUser)
{
  char requete[200],reponse[200];
  bool onContinue = false;

  // ***** Construction de la requete *********************
  sprintf(requete,"LOGIN#%s#%s#%d",user, password, newUser);

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);

  // ***** Parsing de la réponse **************************
  char *ptr = strtok(reponse,"#"); // entête = LOGIN (normalement...)
  ptr = strtok(NULL,"#"); // statut = ok ou ko
  if (strcmp(ptr,"ok") == 0)
  {
    w->dialogueMessage("Login reussi", "Le client est connecté");
    onContinue = true; 
  }
  else
  {
    ptr = strtok(NULL,"#"); // raison du ko
    w->dialogueErreur("Erreur de login", ptr);
  }
  
  return onContinue;
}


//*******************************************************************
void OVESP_Logout()
{
  char requete[200],reponse[200];
  int nbEcrits, nbLus;

  // ***** Construction de la requete *********************
  sprintf(requete,"LOGOUT");

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);
}


//*******************************************************************
void OVESP_Consult(int idArticle)
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  sprintf(requete,"CONSULT#%d", idArticle);

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);

  // ***** Parsing de la réponse **************************
  char *ptr = strtok(reponse,"#"); // entête = CONSULT
  ptr = strtok(NULL,"#"); // statut = ok ou ko
  if (strcmp(ptr,"ok") == 0)
  {
     char *intitule = strtok(NULL,"#");
     float prix = atof(strtok(NULL,"#"));
     int stock = atoi(strtok(NULL,"#"));
     char *image = strtok(NULL,"#");

     w->setArticle(intitule, prix, stock, image);
  }
  else
  {
    ptr = strtok(NULL,"#"); // raison du ko
    w->dialogueErreur("Erreur de consult", ptr);
  }
}

//*******************************************************************
void OVESP_Achat(int idArticle, int quantite)
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  sprintf(requete,"ACHAT#%d#%d", idArticle, quantite);

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);

  // ***** Parsing de la réponse **************************
  char *ptr = strtok(reponse,"#"); // entête = ACHAT
  ptr = strtok(NULL,"#"); // statut = ok ou ko
  if (strcmp(ptr,"ok") == 0)
  {
     int idArticle = atoi(strtok(NULL,"#"));
     int quantite = atoi(strtok(NULL,"#"));
     float prix = atof(strtok(NULL,"#"));

     //dialogueMessage("Achat reussi idArticle : %d / quantite : %d / prix : %f", idArticle, quantite, prix);
     w->dialogueMessage("Achat", "Achat reussi");
  }
  else
  {
    ptr = strtok(NULL,"#"); // raison du ko
    w->dialogueErreur("Erreur d'achat", ptr);
  }
}

//*******************************************************************
void   OVESP_Cancel(int idArticle)
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  sprintf(requete,"CANCEL#%d", idArticle);

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);

  // ***** Parsing de la réponse **************************
  char *ptr = strtok(reponse,"#"); // entête = CANCEL
  ptr = strtok(NULL,"#"); // statut = ok ou ko
  if (strcmp(ptr,"ok") == 0)
  {
     w->dialogueMessage("Cancel", "Supression reussi de l'article");
  }
  else
  {
    ptr = strtok(NULL,"#"); // raison du ko
    w->dialogueErreur("Erreur de cancel", ptr);
  }

}


//*******************************************************************
void   OVESP_Cancel_All()
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  sprintf(requete,"CANCEL_ALL#");

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);

  // ***** Parsing de la réponse **************************
  char *ptr = strtok(reponse,"#"); // entête = CANCEL
  ptr = strtok(NULL,"#"); // statut = ok ou ko
  if (strcmp(ptr,"ok") == 0)
  {
     w->dialogueMessage("Cancel_All", "Supression reussi du panier");
  }
  else
  {
    ptr = strtok(NULL,"#"); // raison du ko
    w->dialogueErreur("Erreur de cancel all", ptr);
  }
}

//*******************************************************************
void OVESP_Confirmer()
{
  char requete[200],reponse[200];

  // ***** Construction de la requete *********************
  sprintf(requete,"CONFIRMER#");

  // ***** Envoi requete + réception réponse **************
  Echange(requete,reponse);

  // ***** Parsing de la réponse **************************
  char *ptr = strtok(reponse,"#"); // entête = CONFIRMER
  ptr = strtok(NULL,"#"); // statut = ok ou ko
  if (strcmp(ptr,"ok") == 0)
  {
     int idFacture = atoi(strtok(NULL,"#"));
     w->dialogueMessage("Facture", "Creation de facture reussi");
  }
  else
  {
    ptr = strtok(NULL,"#"); // raison du ko
    w->dialogueErreur("Erreur de facturation", ptr);
  }
}


//***** Echange de données entre client et serveur ******************
void Echange(char* requete, char* reponse)
{
  int nbEcrits, nbLus;

  // ***** Envoi de la requete ****************************
  if ((nbEcrits = Send(sClient,requete,strlen(requete))) == -1)
  {
    perror("Erreur de Send");
    close(sClient);
    exit(1);
  }

  // ***** Attente de la reponse **************************
  if ((nbLus = Receive(sClient,reponse)) < 0)
  {
    perror("Erreur de Receive");
    close(sClient);
    exit(1);
  }
  if (nbLus == 0)
  {
    printf("Serveur arrete, pas de reponse reçue...\n");
    close(sClient);
    exit(1);
  }
  reponse[nbLus] = 0;
}
