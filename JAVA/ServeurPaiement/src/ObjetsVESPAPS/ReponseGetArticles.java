package ObjetsVESPAPS;

import RequeteReponse.Reponse;
import Modele.Article;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import java.io.*;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.util.ArrayList;

public class ReponseGetArticles implements Reponse {
    private byte[] messageCrypte;

    public ReponseGetArticles(ArrayList<Article> tabArticles, String m, SecretKey cleSession) throws IOException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(baos);
        oos.writeUTF(m);
        oos.writeObject(tabArticles);

        byte[] messageClair = baos.toByteArray();

        messageCrypte = MyCrypto.CryptSymDES(cleSession,messageClair);
    }

    public ArrayList<Article> getArticles(SecretKey cleSession) throws IOException, ClassNotFoundException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] messageClair = MyCrypto.DecryptSymDES(cleSession, messageCrypte);

        ByteArrayInputStream bais = new ByteArrayInputStream(messageClair);
        ObjectInputStream ois = new ObjectInputStream(bais);

        ois.readUTF();
        ArrayList<Article> tabArticles = (ArrayList<Article>) ois.readObject();

        return tabArticles;
    }

    public String getMessage(SecretKey cleSession) throws IOException, ClassNotFoundException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] messageClair = MyCrypto.DecryptSymDES(cleSession, messageCrypte);

        ByteArrayInputStream bais = new ByteArrayInputStream(messageClair);
        ObjectInputStream ois = new ObjectInputStream(bais);

        String message = ois.readUTF();

        return message;
    }
}
