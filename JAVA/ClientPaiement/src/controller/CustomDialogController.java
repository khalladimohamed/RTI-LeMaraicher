package controller;

import VESPAP.ReponsePayFacture;
import VESPAP.RequetePayFacture;
import view.dialog.CustomDialog;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class CustomDialogController implements ActionListener {

    private CustomDialog dialog;
    private Socket socket;
    private ObjectOutputStream oos;
    private ObjectInputStream ois;
    private String username;
    private int cardnumber;
    private int idFacture;

    public CustomDialogController(CustomDialog dialog, Socket socket, ObjectOutputStream oos, ObjectInputStream ois, int idFacture )
    {
        this.socket = socket;
        this.dialog = dialog;
        this.oos = oos;
        this.ois = ois;
        this.idFacture = idFacture;
    }




    @Override
    public void actionPerformed(ActionEvent e) {

        JButton source = (JButton) e.getSource();

        if (source.getText().equals("Confirmer")) {
            username = dialog.getUserName();
            cardnumber = dialog.getCardNumber();

            try {

                RequetePayFacture requete = new RequetePayFacture(idFacture, username, cardnumber);
                oos.writeObject(requete);
                ReponsePayFacture reponse = (ReponsePayFacture) ois.readObject();

                if (reponse.isValide()) {
                    dialog.dialogueMessage("PAYER", reponse.getMessage());
                    dialog.setVisible(false);

                } else {
                    dialog.dialogueErreur("PAYER", reponse.getMessage());
                }
            }
            catch (IOException | ClassNotFoundException ex)
            {
                System.out.println(ex.getMessage());
            }
        } else if (source.getText().equals("Annuler")) {
            dialog.setVisible(false);
        }

    }
}
