package Controleur;

import ObjetsVESPAPS.ReponsePayFacture;
import ObjetsVESPAPS.RequetePayFacture;
import Vue.PayementWindow;

import javax.crypto.SecretKey;
import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

public class PayementWindowController implements ActionListener {

    private PayementWindow dialog;
    private Socket socket;
    private ObjectOutputStream oos;
    private ObjectInputStream ois;
    private String username;
    private String cardnumber;
    private int idFacture;
    private SecretKey cleSession;

    public PayementWindowController(PayementWindow dialog, Socket socket, ObjectOutputStream oos, ObjectInputStream ois, int idFacture, SecretKey cleSession)
    {
        this.socket = socket;
        this.dialog = dialog;
        this.oos = oos;
        this.ois = ois;
        this.idFacture = idFacture;
        this.cleSession = cleSession;
    }




    @Override
    public void actionPerformed(ActionEvent e) {

        JButton source = (JButton) e.getSource();

        if (source.getText().equals("Confirmer")) {
            username = dialog.getUserName();
            cardnumber = dialog.getCardNumber();

            System.out.println("Card number : " + cardnumber);

            try {
                RequetePayFacture requete = new RequetePayFacture(idFacture, username, cardnumber, cleSession);
                oos.writeObject(requete);
                ReponsePayFacture reponse = (ReponsePayFacture) ois.readObject();

                if(!reponse.VerifyAuthenticity(cleSession))
                    dialog.dialogueErreur("Payement", "Erreur : VerifyAuthenticity()");

                if (reponse.isValide()) {
                    dialog.dialogueMessage("Payement", reponse.getMessage());
                    dialog.setVisible(false);

                } else {
                    dialog.dialogueErreur("Payement", reponse.getMessage());
                }
            }
            catch (IOException | ClassNotFoundException | NoSuchPaddingException | IllegalBlockSizeException |
                   NoSuchAlgorithmException | BadPaddingException | NoSuchProviderException | InvalidKeyException ex)
            {
                System.out.println(ex.getMessage());
            }
        } else if (source.getText().equals("Annuler")) {
            dialog.setVisible(false);
        }

    }
}
