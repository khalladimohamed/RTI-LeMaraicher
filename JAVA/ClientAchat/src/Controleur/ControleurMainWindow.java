package Controleur;

import Vue.MainWindow;
import Controleur.Lib.LibSocket;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.Socket;
import java.util.Timer;
import java.util.TimerTask;

public class ControleurMainWindow implements ActionListener {
    private MainWindow mainWindow;
    private Socket sSocket;
    private int CurrentIdArticle = 0;

    public ControleurMainWindow(MainWindow mainWindow)
    {
        this.mainWindow = mainWindow;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        JButton source = (JButton) e.getSource();
        boolean boucle = true;

        if(source.getText().equals("Login"))
        {
            // Création de la socket et connexion sur le serveur
            try {
                sSocket = new Socket("192.168.109.130", 50000);
            } catch (IOException ex) {
                throw new RuntimeException(ex);
            }

            // Envoi de la requête
            LibSocket.send(sSocket, "LOGIN#" + mainWindow.getNom() + "#" + mainWindow.getMotDePasse() + "#" + mainWindow.isNouveauClientChecked());

            // Réponse du Serveur
            Timer timer = new Timer();
            timer.schedule(new TimerTask() {
                @Override
                public void run() {
                    // Code à exécuter après 2 secondes
                    String reponse = LibSocket.receive(sSocket);

                    String[] mots = reponse.split("#");

                    if (mots[0].equals("LOGIN")) {
                        if (mots[1].equals("ok")) {
                            Consult(CurrentIdArticle);
                            mainWindow.LoginOK();
                            mainWindow.dialogueMessage("LOGIN", "Le client est connecté");
                            Actualiser_Panier();
                        } else if (mots[1].equals("ko")) {
                            mainWindow.dialogueErreur("LOGIN", mots[2]);
                            return;
                        }
                    }
                }
            }, 2000);

        }

        else if(source.getText().equals("Logout"))
        {
            //Envoie de la requete
            LibSocket.send(sSocket,"LOGOUT");

            //Réponse
            String reponse = LibSocket.receive(sSocket);

            String[] mots = reponse.split("#");

            //Fermeture de la socket
            try {
                sSocket.close();
            } catch (IOException ex) {
                throw new RuntimeException(ex);
            }

            if(mots[1].equals("ok"))
            {
                mainWindow.LogoutOK();
            }
        }

        else if(source.getText().equals(">>>"))
        {
            CurrentIdArticle = (CurrentIdArticle+1)%21;
            Consult(CurrentIdArticle);
        }

        else if(source.getText().equals("<<<"))
        {
            CurrentIdArticle = (CurrentIdArticle+20)%21;
            Consult(CurrentIdArticle);
        }

        else if(source.getText().equals("Acheter"))
        {
            if(mainWindow.getQuantite() == 0)
                return;

            LibSocket.send(sSocket,"ACHAT#"+(CurrentIdArticle+1)+"#"+mainWindow.getQuantite());

            String reponse = LibSocket.receive(sSocket);

            String[] mots = reponse.split("#");

            if(mots[0].equals("ACHAT"))
            {

                if(mots[1].equals("ok"))
                {
                    mainWindow.ajouteArticleTablePanier(CurrentIdArticle+1, mots[3],Float.parseFloat(mots[5]),Integer.parseInt(mots[4]));
                    Consult(CurrentIdArticle);
                }
                else
                {
                    mainWindow.dialogueErreur(mots[0],mots[2]);
                }
            }

            Actualiser_Panier();
        }

        else if(source.getText().equals("Supprimer article"))
        {
            if(mainWindow.getIdArticleSelectionne() == -1)
                return;

            LibSocket.send(sSocket,"CANCEL#"+mainWindow.getIdArticleSelectionne());

            String reponse = LibSocket.receive(sSocket);

            String[] mots = reponse.split("#");

            if(mots[1].equals("ok"))
            {
                Actualiser_Panier();
                Consult(CurrentIdArticle);
            }
            else
                mainWindow.dialogueErreur(mots[0],"Error");
        }

        else if(source.getText().equals("Vider le panier"))
        {
            LibSocket.send(sSocket, "CANCEL_ALL");

            String reponse = LibSocket.receive(sSocket);

            String[] mots = reponse.split("#");

            if(!mots[1].equals("ok"))
            {
                mainWindow.dialogueErreur(mots[0],"Error");
                return;
            }

            Actualiser_Panier();
            Consult(CurrentIdArticle);
            mainWindow.setTotal(0);
        }

        else if(source.getText().equals("Confirmer achat"))
        {
            LibSocket.send(sSocket, "CONFIRMER");

            String reponse = LibSocket.receive(sSocket);

            String[] mots = reponse.split("#");

            if(mots[1].equals("ok"))
            {
                Actualiser_Panier();
                mainWindow.setTotal(0);
                mainWindow.dialogueMessage("SUCCESS", "Achat reussi !");
            }
        }

    }

    private void Consult(int id)
    {
        id++;
        LibSocket.send(sSocket,"CONSULT#"+id);

        String reponse = LibSocket.receive(sSocket);

        String[] mots = reponse.split("#");

        if (mots[0].equals("CONSULT"))
        {
            if (!mots[1].equals("ko"))
            {
                mainWindow.setArticle(mots[2], Float.parseFloat(mots[3]), Integer.parseInt(mots[4]), mots[5]);
            }
            else
                mainWindow.dialogueErreur(mots[0],mots[2]);
        }

    }

    private void Actualiser_Panier() {
        mainWindow.videTablePanier();

        LibSocket.send(sSocket, "CADDIE");

        String reponse = LibSocket.receive(sSocket);

        String[] mots = reponse.split("#");

        if (mots[0].equals("CADDIE") && mots[1].equals("ok")) {
            String contenu;
            if (mots.length >= 4) {
                contenu = mots[3];

                if (contenu.isEmpty()) {
                    System.out.println("Le caddie est vide.");
                } else {
                    String[] articles = contenu.split(";");

                    float total = 0;

                    for (String article : articles) {
                        String[] attributs = article.split(",");
                        if (attributs.length == 4) {
                            int idArticle = Integer.parseInt(attributs[0]);
                            String intitule = attributs[1];
                            int quantite = Integer.parseInt(attributs[2]);
                            float prix = Float.parseFloat(attributs[3]);

                            total += prix * quantite;
                            System.out.println("ID: " + idArticle + ", Intitule: " + intitule + ", Quantite: " + quantite + ", Prix: " + prix);

                            mainWindow.ajouteArticleTablePanier(idArticle, intitule, prix, quantite);
                        }
                    }

                    mainWindow.setTotal(total);
                }
            }
        } else {
            // Gérer le cas où la réponse du serveur est incorrecte.
            System.out.println("La réponse du serveur est incorrecte.");
        }
    }


}
