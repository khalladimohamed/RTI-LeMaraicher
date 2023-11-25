package Controleur;

import Modele.Article;
import ObjetsVESPAPS.*;
import Vue.MainWindow;
import com.formdev.flatlaf.themes.FlatMacLightLaf;
import Modele.Facture;
import Vue.PayementWindow;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.security.Security;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import java.security.*;
import java.security.cert.CertificateException;


public class MainWindowController implements ActionListener{

    private MainWindow mainWindow;
    private Socket socket;
    private ObjectOutputStream oos;
    private ObjectInputStream ois;
    private SecretKey cleSession;

    public MainWindowController(MainWindow mainWindow)
    {
        this.mainWindow = mainWindow;
        oos = null;
        ois = null;
        Security.addProvider(new BouncyCastleProvider());
    }



    @Override
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() instanceof JButton)
        {
            JButton source = (JButton) e.getSource();

            if(source.getText().equals("Login"))
            {
                try {
                    Login();
                } catch (CertificateException ex) {
                    throw new RuntimeException(ex);
                } catch (KeyStoreException ex) {
                    throw new RuntimeException(ex);
                } catch (IOException ex) {
                    throw new RuntimeException(ex);
                } catch (NoSuchAlgorithmException ex) {
                    throw new RuntimeException(ex);
                }
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



    private void Login() throws CertificateException, KeyStoreException, IOException, NoSuchAlgorithmException {

        String login = mainWindow.getNom();
        String password = mainWindow.getMotDePasse();

        try
        {
            socket = new Socket("localhost",52000);
            socket.setSoTimeout(2000);
            oos = new ObjectOutputStream(socket.getOutputStream());
            ois = new ObjectInputStream(socket.getInputStream());

            // Envois digest salé
            RequeteLogin requete = new RequeteLogin(login,password);
            oos.writeObject(requete);

            //Reception de la reponse
            ReponseLogin reponse = (ReponseLogin) ois.readObject();

            PrivateKey clePrivee = RecupereClePriveeClient();

            String message = reponse.getMessage(clePrivee);
            boolean valide = reponse.getValide(clePrivee);

            if (valide)
            {
                cleSession = reponse.getCleSession(clePrivee);
                mainWindow.LoginOK();
                mainWindow.dialogueMessage("LOGIN", message);
            }
            else
            {
                mainWindow.dialogueErreur("LOGIN", message);
                socket.close();
            }
        }
        catch (IOException | ClassNotFoundException | NoSuchProviderException | UnrecoverableKeyException |
               NoSuchPaddingException | InvalidKeyException | IllegalBlockSizeException | BadPaddingException ex)
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
        int indice = mainWindow.getIndiceFactureSelectionne();
        if(indice != -1)
        {
            FlatMacLightLaf.setup();

            // Affichage de la fenêtre
            try {
                UIManager.setLookAndFeel(new FlatMacLightLaf());
            } catch (Exception e) {
                e.printStackTrace();
            }

            PayementWindow payementWindow = new PayementWindow(mainWindow, "Payement");
            PayementWindowController controller = new PayementWindowController(payementWindow, this.socket, this.oos, this.ois, indice, cleSession);
            payementWindow.setController(controller);
            payementWindow.setVisible(true);

            GetFactures();
            mainWindow.videTableArticle();
        }
        else
        {
            mainWindow.dialogueErreur("Erreur", "La facture est deja paye !");
        }

    }



    private void GetFactures()
    {
        try
        {
            RequeteGetFactures requete = new RequeteGetFactures(mainWindow.getNumClient(), RecupereClePriveeClient());
            oos.writeObject(requete);
            ReponseGetFactures reponse = (ReponseGetFactures) ois.readObject();

            if(!reponse.getMessage(cleSession).equals("OK"))
                mainWindow.dialogueErreur("GetFactures", reponse.getMessage(cleSession));

            mainWindow.videTableFacture();
            for (Facture fact : reponse.getTableauFactures(cleSession)) {
                mainWindow.ajouteFactureTable(fact.getIdFacture(), fact.getDate(), fact.getMontant(), fact.getPaye());
            }

        }
        catch (IOException | ClassNotFoundException | KeyStoreException | UnrecoverableKeyException |
               NoSuchAlgorithmException | CertificateException | InvalidKeyException | NoSuchProviderException |
               SignatureException | NoSuchPaddingException | IllegalBlockSizeException | BadPaddingException ex)
        {
            mainWindow.dialogueErreur("Erreur", "Problème de connexion " + ex.getMessage());
        }
    }


    private void GetArticles()
    {
        int idFacture = mainWindow.getIdFacture();

        try {
            RequeteGetArticles requete = new RequeteGetArticles(idFacture, RecupereClePriveeClient());

            oos.writeObject(requete);
            ReponseGetArticles reponse = (ReponseGetArticles) ois.readObject();

            if(!reponse.getMessage(cleSession).equals("OK"))
                mainWindow.dialogueErreur("GetArticles", reponse.getMessage(cleSession));

            mainWindow.videTableArticle();

            for (Article art : reponse.getArticles(cleSession))
            {
                mainWindow.ajouteArticleTable(art.getIntitule(), art.getPrixUnitaire(), art.getQuantite());
            }

        }
        catch (IOException | ClassNotFoundException | KeyStoreException | UnrecoverableKeyException |
               NoSuchAlgorithmException | CertificateException | NoSuchProviderException | InvalidKeyException |
               SignatureException | NoSuchPaddingException | IllegalBlockSizeException | BadPaddingException ex)
        {
            mainWindow.dialogueErreur("Erreur", "Error requete Article " + ex.getMessage());
        }
    }



    public static PrivateKey RecupereClePriveeClient() throws KeyStoreException, IOException, UnrecoverableKeyException, NoSuchAlgorithmException, CertificateException {
        KeyStore ks = KeyStore.getInstance("JKS");
        ks.load(new FileInputStream("KeyStoreClient.jks"),"rti2023".toCharArray());
        PrivateKey cle = (PrivateKey) ks.getKey("cleclient","cleclient".toCharArray());
        return cle;
    }

}