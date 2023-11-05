package ServeurGeneriqueTCP.Protocole;
import ServeurGeneriqueTCP.Exception.FinConnexionException;
import VESPAP.*;

public interface Protocole
{
    String getNom();
    Reponse TraiteRequete(Requete requete) throws FinConnexionException;
}
