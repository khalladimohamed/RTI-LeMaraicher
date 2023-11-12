package Controleur;


import ObjetsVESPAP.*;
import Vue.MainWindow;
import com.formdev.flatlaf.themes.FlatMacLightLaf;
import Modele.Facture;
import Vue.PayementWindow;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;



public class MainWindowController implements ActionListener{

    private MainWindow mainWindow;
    private Socket socket;
    private ObjectOutputStream oos;
    private ObjectInputStream ois;

    private int CurrentIdArticle = 0;

    public MainWindowController(MainWindow mainWindow)
    {
        this.mainWindow = mainWindow;
        oos = null;
        ois = null;
    }



    @Override
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() instanceof JButton)
        {
            JButton source = (JButton) e.getSource();

            if(source.getText().equals("Login"))
            {
                Login();
            }
            else if(source.getText().equals("Logout"))
            {
                Logout();
            }
            else if(source.getText().equals("Payer"))
            {
                Payer();
            }
            else if (source.getText().equals("Afficher les factures"))
            {
                GetFactures();
            }
            else if (source.getText().equals("Recuperer le detail de la facture"))
            {
                GetArticles();
            }
        }

    }



    private void Login()
    {
        String login = mainWindow.getNom();
        String password = mainWindow.getMotDePasse();
        boolean isNew = mainWindow.isNouveauEmployeChecked();

        try
        {
            socket = new Socket("localhost",6666);
            RequeteLogin requete = new RequeteLogin(login,password,isNew);
            oos = new ObjectOutputStream(socket.getOutputStream());
            ois = new ObjectInputStream(socket.getInputStream());
            oos.writeObject(requete);
            ReponseLogin reponse = (ReponseLogin) ois.readObject();
            if (reponse.isValide())
            {
                mainWindow.LoginOK();
                mainWindow.dialogueMessage("LOGIN", "L'employe est connecté");
            }
            else
            {
                mainWindow.dialogueErreur("LOGIN", reponse.getMessage());
                socket.close();
            }
        }
        catch (IOException | ClassNotFoundException ex)
        {
            mainWindow.dialogueErreur("LOGIN", "Problème de connexion" + ex.getMessage());
        }

    }



    private void Logout()
    {
        try
        {
            RequeteLogout requete = new RequeteLogout(mainWindow.getNom());
            oos.writeObject(requete);
            oos.close();
            ois.close();
            socket.close();

            mainWindow.LogoutOK();
        }
        catch (IOException ex)
        {
            mainWindow.dialogueErreur("LOGOUT", "Problème de connexion");
        }

    }



    private void Payer()
    {
        FlatMacLightLaf.setup();

        // Affichage de la fenêtre
        try {
            UIManager.setLookAndFeel(new FlatMacLightLaf());
        } catch (Exception e) {
            e.printStackTrace();
        }

        PayementWindow payementWindow = new PayementWindow(mainWindow, "Payement");
        PayementWindowController controller = new PayementWindowController(payementWindow, this.socket, this.oos, this.ois, mainWindow.getIndiceFactureSelectionne());
        payementWindow.setController(controller);
        payementWindow.setVisible(true);

        GetFactures();
        mainWindow.videTableArticle();
    }



    private void GetFactures()
    {
        try
        {
            RequeteGetFactures requete = new RequeteGetFactures(mainWindow.getNumClient());
            oos.writeObject(requete);
            ReponseGetFactures reponse = (ReponseGetFactures) ois.readObject();

            mainWindow.videTableFacture();
            for (Facture fact : reponse.getTableauFactures()) {
                if(!fact.getPaye())
                    mainWindow.ajouteFactureTable(fact.getIdFacture(), fact.getDate(), fact.getMontant(), fact.getPaye());
            }

        }
        catch (IOException | ClassNotFoundException ex)
        {
            mainWindow.dialogueErreur("Erreur", "Problème de connexion" + ex.getMessage());
        }
    }


    private void GetArticles()
    {
        int idFacture = mainWindow.getIdFacture();

        RequeteGetArticles requete = new RequeteGetArticles(idFacture);

        try {
            oos.writeObject(requete);
            ReponseGetArticles reponse = (ReponseGetArticles) ois.readObject();

            mainWindow.videTableArticle();

            while(reponse.getArticles().size() > 0)
            {
                mainWindow.ajouteArticleTable(reponse.getArticles().get(0).getIntitule(), reponse.getArticles().get(0).getPrixUnitaire(), reponse.getArticles().get(0).getQuantite());
                reponse.getArticles().remove(0);
            }

        }
        catch (IOException | ClassNotFoundException ex)
        {
            mainWindow.dialogueErreur("Erreur", "Erreur IO object requete Article" + ex.getMessage());
        }
    }

}