package ObjetsVESPAPS;

import RequeteReponse.Requete;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.security.*;

public class RequeteGetFactures implements Requete {
    private int idClient;
    private byte[] signature;

    public RequeteGetFactures(int id, PrivateKey clePriveeClient) throws InvalidKeyException, NoSuchAlgorithmException, NoSuchProviderException, IOException, SignatureException {
        idClient = id;

        Signature s = Signature.getInstance("SHA1withRSA","BC");
        s.initSign(clePriveeClient);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeInt(id);
        s.update(baos.toByteArray());
        signature = s.sign();
    }

    public int getIdClient() {
        return idClient;
    }

    public boolean VerifySignature(PublicKey clePubliqueClient) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, IOException, SignatureException {
        Signature s = Signature.getInstance("SHA1withRSA","BC");
        s.initVerify(clePubliqueClient);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeInt(idClient);
        s.update(baos.toByteArray());

        return s.verify(signature);
    }
}
