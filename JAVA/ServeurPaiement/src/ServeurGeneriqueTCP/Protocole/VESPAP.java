package ServeurGeneriqueTCP.Protocole;

import ServeurGeneriqueTCP.Exception.FinConnexionException;
import ServeurGeneriqueTCP.Logger.Logger;
import db.DatabaseUseCase;
import model.Facture;
import VESPAP.*;

import java.sql.SQLException;
import java.util.ArrayList;

public class VESPAP implements Protocole {

    private Logger logger;
    private DatabaseUseCase databaseUseCase;

    public VESPAP(Logger log, DatabaseUseCase databaseUseCase)
    {
        logger = log;
        this.databaseUseCase = databaseUseCase;
    }


    @Override
    public String getNom() {
        return "VESPAP";
    }


    @Override
    public synchronized Reponse TraiteRequete(Requete requete) throws FinConnexionException {

        logger.Trace("Traitement requete");

        if(requete instanceof RequeteLogin)
            return TraiteRequeteLogin((RequeteLogin)requete);
        else if(requete instanceof RequeteGetFactures)
            return TraiteRequeteGetFactures((RequeteGetFactures)requete);
       else if(requete instanceof RequetePayFacture)
            return TraiteRequetePayFacture((RequetePayFacture)requete);
       else if(requete instanceof RequeteLogout)
            TraiteRequeteLogout((RequeteLogout)requete);
       else if(requete instanceof RequeteGetArticles)
            return TraiteRequeteGetArticle((RequeteGetArticles)requete);
        return null;
    }



    private synchronized Reponse TraiteRequeteLogin(RequeteLogin requeteLogin) throws FinConnexionException
    {
        logger.Trace("RequeteLOGIN reçue de " + requeteLogin.getLogin());

        ReponseLogin reponseLogin = new ReponseLogin();

        boolean isNewClient = requeteLogin.isNewEmploye();
        String username = requeteLogin.getLogin();
        String password = requeteLogin.getPassword();

        if(!isNewClient)
        {
            if(databaseUseCase.isLoginOk(username, password))
            {
                reponseLogin.setMessage("Login OK");
                reponseLogin.setValide(true);
                return reponseLogin;
            }
            else
            {
                reponseLogin.setValide(false);

                if(databaseUseCase.isUsernameExists(username))
                {
                    reponseLogin.setMessage("Mauvais mot de passe");
                    throw new FinConnexionException(reponseLogin);

                }
                else
                {
                    reponseLogin.setMessage("Utilisateur inexistant");
                    throw new FinConnexionException(reponseLogin);
                }
            }
        }
        else if(!databaseUseCase.isUsernameExists(username))
        {
            databaseUseCase.addEmploye(username, password);
            reponseLogin.setMessage("Utilisateur créé");
            reponseLogin.setValide(true);
            return reponseLogin;
        }
        else
        {
            throw new FinConnexionException(new ReponseLogin(false, "Nom de Client deja existant"));
        }
    }



    private synchronized Reponse TraiteRequeteGetFactures(RequeteGetFactures requete)
    {
        logger.Trace("RequeteGETFACTURES reçue de " + requete.getIdClient());
        try {
            ArrayList<Facture> factures = databaseUseCase.getFactures(requete.getIdClient());
            ReponseGetFactures reponse = new ReponseGetFactures(factures, "OK");
            return reponse;
        }
        catch (Exception e)
        {
            return new ReponseGetFactures(new ArrayList<>(), "Probleme BD");
        }


    }



    private synchronized Reponse TraiteRequetePayFacture(RequetePayFacture requete)
    {
        logger.Trace("RequetePAYFACTURE reçue de " + requete.getNom());

        if(!databaseUseCase.isUsernameExists(requete.getNom()))
            return new ReponsePayFacture(false, "Le client n'existe pas !");

        if(!LuhnAlgorithm.isValidLuhnNumber(requete.getNumeroCarte()))
            return new ReponsePayFacture(false, "Carte bancaire invalide");

        String message = null;
        try {
            message = databaseUseCase.payFacture(requete.getIdFacture());
        } catch (SQLException e) {
            throw new RuntimeException(e);
        }

        if(message.equals("OK"))
            return new ReponsePayFacture(true, "PAYER");
        else
            return new ReponsePayFacture(false, "Erreur paiement");
    }



    private synchronized void TraiteRequeteLogout(RequeteLogout requete) throws FinConnexionException
    {
        logger.Trace("RequeteLOGOUT reçue de " + requete.getLogin());
        logger.Trace(requete.getLogin() + " correctement déloggé");
        throw new FinConnexionException(null);
    }



    private synchronized Reponse TraiteRequeteGetArticle(RequeteGetArticles requete)
    {
        logger.Trace("RequeteGETARTICLES reçue de " + requete.getIdFacture());

        ReponseGetArticles reponse = new ReponseGetArticles(databaseUseCase.getArticles(requete.getIdFacture()));

        return reponse;
    }
}
