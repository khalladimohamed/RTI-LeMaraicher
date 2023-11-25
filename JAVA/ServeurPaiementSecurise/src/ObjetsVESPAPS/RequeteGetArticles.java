package ObjetsVESPAPS;

import RequeteReponse.Requete;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.security.*;

public class RequeteGetArticles implements Requete {

    private int idFacture;
    private byte[] signature;

    public RequeteGetArticles(int idFacture, PrivateKey cleprivee) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, IOException, SignatureException {
        this.idFacture = idFacture;

        Signature s = Signature.getInstance("SHA1withRSA","BC");
        s.initSign(cleprivee);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeInt(idFacture);
        s.update(baos.toByteArray());
        signature = s.sign();
    }

    public int getIdFacture()
    {
        return idFacture;
    }

    public boolean VerifySignature(PublicKey clePubliqueClient) throws NoSuchAlgorithmException, NoSuchProviderException, InvalidKeyException, IOException, SignatureException {
        Signature s = Signature.getInstance("SHA1withRSA","BC");
        s.initVerify(clePubliqueClient);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        dos.writeInt(idFacture);
        s.update(baos.toByteArray());

        return s.verify(signature);
    }
}
