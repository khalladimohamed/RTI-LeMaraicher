package ObjetsVESPAPS;

import RequeteReponse.Requete;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.security.*;
import java.util.Date;

public class RequeteLogin implements Requete {
    private String login;
    private long temps;
    private double alea;
    private byte[]digest;
    public RequeteLogin(String l, String p) throws NoSuchAlgorithmException, NoSuchProviderException, IOException {
        login = l;

        temps = new Date().getTime();
        alea = Math.random();

        Security.addProvider(new BouncyCastleProvider());
        MessageDigest md = MessageDigest.getInstance("SHA-1","BC");
        md.update(l.getBytes());
        md.update(p.getBytes());

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeLong(temps);
        dos.writeDouble(alea);

        md.update(baos.toByteArray());

        digest = md.digest();
    }

    public String getLogin() {
        return login;
    }
    public boolean VerifyPassword(String password) throws NoSuchAlgorithmException, NoSuchProviderException, IOException {
        // Construction du digest local

        Security.addProvider(new BouncyCastleProvider());

        MessageDigest md = MessageDigest.getInstance("SHA-1","BC");
        md.update(login.getBytes());
        md.update(password.getBytes());

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeLong(temps);
        dos.writeDouble(alea);
        md.update(baos.toByteArray());

        byte[] digestLocal = md.digest();

        // Comparaison digest reÃ§u et digest local
        return MessageDigest.isEqual(digest,digestLocal);
    }

}
