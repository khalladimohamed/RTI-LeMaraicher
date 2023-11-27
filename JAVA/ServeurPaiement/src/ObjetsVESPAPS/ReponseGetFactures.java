package ObjetsVESPAPS;

import RequeteReponse.Reponse;
import Modele.Facture;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import java.io.*;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.util.ArrayList;

public class ReponseGetFactures implements Reponse {
    private byte[] messageCrypte;

    public ReponseGetFactures(ArrayList<Facture> tabFactures, String m, SecretKey cleSession) throws IOException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(baos);
        oos.writeUTF(m);
        oos.writeObject(tabFactures);

        byte[] messageClair = baos.toByteArray();

        messageCrypte = MyCrypto.CryptSymDES(cleSession,messageClair);
    }


    public ArrayList<Facture> getTableauFactures(SecretKey cleSession) throws IOException, ClassNotFoundException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] messageClair = MyCrypto.DecryptSymDES(cleSession, messageCrypte);

        ByteArrayInputStream bais = new ByteArrayInputStream(messageClair);
        ObjectInputStream ois = new ObjectInputStream(bais);

        ois.readUTF();
        ArrayList<Facture> tabFactures = (ArrayList<Facture>) ois.readObject();

        return tabFactures;
    }

    public String getMessage(SecretKey cleSession) throws IOException, ClassNotFoundException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] messageClair = MyCrypto.DecryptSymDES(cleSession, messageCrypte);

        ByteArrayInputStream bais = new ByteArrayInputStream(messageClair);
        ObjectInputStream ois = new ObjectInputStream(bais);

        String message = ois.readUTF();

        return message;
    }
}
