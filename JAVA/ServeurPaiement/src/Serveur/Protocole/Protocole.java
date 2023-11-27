package Serveur.Protocole;
import RequeteReponse.Reponse;
import RequeteReponse.Requete;
import Serveur.Exception.FinConnexionException;
import java.io.IOException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;

public interface Protocole
{
    String getNom();
    Reponse TraiteRequete(Requete requete) throws FinConnexionException, NoSuchAlgorithmException, IOException, NoSuchProviderException;

}
