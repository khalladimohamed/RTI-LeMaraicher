package ObjetsVESPAPS;

import RequeteReponse.Reponse;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.io.*;
import java.security.*;

public class ReponseLogin implements Reponse {
    private byte[] cleDeSession;  // cryptée asymétriquement
    private byte[] message; // cryptée asymétriquement
    public ReponseLogin(){

    }


    public ReponseLogin(String msg, boolean valide, SecretKey clesession, PublicKey clepublic) throws NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException, IOException {

        if(clesession != null)
            cleDeSession = MyCrypto.CryptAsymRSA(clepublic,clesession.getEncoded());


        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeUTF(msg);
        dos.writeBoolean(valide);
        byte[] messageClair = baos.toByteArray();

        message = MyCrypto.CryptAsymRSA(clepublic,messageClair);
    }


    public SecretKey getCleSession(PrivateKey clePrivee) throws NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] cleSessionDecryptee = MyCrypto.DecryptAsymRSA(clePrivee, cleDeSession);
        return new SecretKeySpec(cleSessionDecryptee,"DES");
    }

    public boolean getValide(PrivateKey clePrivee) throws IOException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] messageDecrypte = MyCrypto.DecryptAsymRSA(clePrivee, message);

        ByteArrayInputStream bais = new ByteArrayInputStream(messageDecrypte);
        DataInputStream dis = new DataInputStream(bais);
        String message = dis.readUTF();

        return dis.readBoolean();
    }

    public String getMessage(PrivateKey clePrivee) throws IOException, NoSuchPaddingException, IllegalBlockSizeException, NoSuchAlgorithmException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
        byte[] messageDecrypte = MyCrypto.DecryptAsymRSA(clePrivee, message);

        ByteArrayInputStream bais = new ByteArrayInputStream(messageDecrypte);
        DataInputStream dis = new DataInputStream(bais);
        return dis.readUTF();
    }
}

