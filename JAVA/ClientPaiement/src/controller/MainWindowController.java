package controller;


import VESPAP.*;
import Vue.MainWindow;
import model.Facture;
import view.dialog.CustomDialog;
import view.window.WindowClient;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Properties;


public class MainWindowController implements ActionListener, MouseListener {

    private WindowClient mainWindow;
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
        }
        else if (e.getSource() instanceof JTextField) {
            JTextField source = (JTextField) e.getSource();

            System.out.println("TextField");

            if(source.getToolTipText().equals("NumClient"))
            {
                GetFactures();
            }
        }

    }

    public void mouseClicked(MouseEvent e) {
        System.out.println("Mouse Clicked");
        JTable source = (JTable) e.getSource();
        if (source.getToolTipText().equals("Facture"))
        {
            int idFacture = (int) source.getValueAt(source.getSelectedRow(), 0);

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

    private void Login()
    {
        Properties prop = new Properties();

//        try (FileInputStream fis = new FileInputStream("properties.properties")) {
//            prop.load(fis);
//        } catch (IOException e) {
//            e.printStackTrace();
//        }

//        String ipServeur = prop.getProperty("Serveur");
//        int portServeur = Integer.parseInt(prop.getProperty("Port"));
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
            }
            else
            {
                mainWindow.dialogueErreur("LOGIN", reponse.getMessage());
                socket.close();
            }
        }
        catch (IOException | ClassNotFoundException ex)
        {
            mainWindow.dialogueErreur("LOGIN", "Problème de connexio" + ex.getMessage());
        }

    }

    private void Logout()
    {
        try
        {
            RequeteLogout requete = new RequeteLogout(mainWindow.getNom()); //Je pense que le getnom sert à rien
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

        CustomDialog Dialog = new CustomDialog(mainWindow, "Payer");
        CustomDialogController controller = new CustomDialogController(Dialog, this.socket, this.oos, this.ois, mainWindow.getIndiceFactureSelectionne());
        Dialog.setController(controller);
        Dialog.setVisible(true);

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
                mainWindow.ajouteFactureTable(fact.getIdFacture(), fact.getDate(), fact.getMontant(), fact.getPaye());
            }

        }
        catch (IOException | ClassNotFoundException ex)
        {
            mainWindow.dialogueErreur("Erreur", "Problème de connexion" + ex.getMessage());
        }
    }


    @Override
    public void mousePressed(MouseEvent e) {}
    @Override
    public void mouseReleased(MouseEvent e) {}
    @Override
    public void mouseEntered(MouseEvent e) {}
    @Override
    public void mouseExited(MouseEvent e) {}
}