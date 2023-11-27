package Serveur.Protocole;

import Serveur.Exception.FinConnexionException;
import Serveur.Logger.Logger;
import RequeteReponse.Requete;
import RequeteReponse.Reponse;
import JDBC.BeanMetier;
import Modele.Article;
import Modele.Facture;
import ObjetsVESPAPS.*;

import javax.crypto.*;
import java.io.*;
import java.security.*;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;

public class VESPAPS implements Protocole {

    private Logger logger;
    private BeanMetier beanMetier;
    private SecretKey cleSession;

    public VESPAPS(Logger log, BeanMetier beanMetier)
    {
        logger = log;
        this.beanMetier = beanMetier;
    }

    @Override
    public String getNom() {
        return "VESPAPS";
    }



    @Override
    public synchronized Reponse TraiteRequete(Requete requete) throws FinConnexionException, NoSuchAlgorithmException, IOException, NoSuchProviderException {

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



    private synchronized Reponse TraiteRequeteLogin(RequeteLogin requeteLogin) throws FinConnexionException {
        logger.Trace("RequeteLOGIN reçue de " + requeteLogin.getLogin());

        try {
            boolean valide = false;
            String message = "Null";
            String motDePasse = "";
            motDePasse = beanMetier.getPassword(requeteLogin.getLogin());

            if(motDePasse.equals("")) {
                message = "Utilisateur inexistant";
            }
            else if(requeteLogin.VerifyPassword(motDePasse)) {
                message = "Login OK";
                valide = true;
            }
            else{
                message = "Mauvais mot de passe";
            }

            if(valide)
            {
                logger.Trace("OK: ReponseLogin envois");

                KeyGenerator cleGen = KeyGenerator.getInstance("DES","BC");
                cleGen.init(new SecureRandom());
                cleSession = cleGen.generateKey();

                ReponseLogin reponseLogin = new ReponseLogin(message, valide, cleSession, RecupereClePubliqueClient());
                return reponseLogin;
            }


            ReponseLogin reponseLogin = new ReponseLogin(message, valide, null, RecupereClePubliqueClient());
            throw new FinConnexionException(reponseLogin);

        } catch (NoSuchAlgorithmException | IOException | NoSuchProviderException | NoSuchPaddingException |
                 InvalidKeyException | IllegalBlockSizeException | BadPaddingException | KeyStoreException |
                 CertificateException e) {
            logger.Trace("Server: Error Login() "+ e);
            throw new FinConnexionException(new ReponseLogin());
        }
    }



    private synchronized Reponse TraiteRequeteGetFactures(RequeteGetFactures requete) {
        logger.Trace("RequeteGETFACTURES reçue de " + requete.getIdClient());

        int clientId = requete.getIdClient();
        try {
            PublicKey clePublique = RecupereClePubliqueClient();

            if (!requete.VerifySignature(clePublique))
                return new ReponseGetFactures(new ArrayList<>(), "Mauvaise signature", cleSession);

            ArrayList<Facture> factures = beanMetier.getFactures(clientId);
            ReponseGetFactures reponse = new ReponseGetFactures(factures, "OK", cleSession);
            return reponse;
        }
        catch (Exception e)
        {
            logger.Trace("Server: Error TraiteRequeteGetFactures() "+e);
        }
        return null;
    }



    private synchronized Reponse TraiteRequetePayFacture(RequetePayFacture requete)
    {
        try {
            logger.Trace("RequetePAYFACTURE reçue de " + requete.getNom(cleSession));

            if(!beanMetier.verifyClientName(requete.getNom(cleSession)))
                return new ReponsePayFacture(false, "Le client n'existe pas !", cleSession);

            if(!LuhnAlgorithm.isValidLuhnNumber(requete.getNumeroCarte(cleSession)))
                return new ReponsePayFacture(false, "Carte bancaire invalide", cleSession);

            String message = beanMetier.payFacture(requete.getIdFacture(cleSession));
            if(message.equals("OK"))
                return new ReponsePayFacture(true, "PAYER", cleSession);
            else
                return new ReponsePayFacture(false, "Probleme BD", cleSession);
        }
        catch (Exception e)
        {
            try {
                return new ReponsePayFacture(false, "Probleme BD", cleSession);
            } catch (NoSuchAlgorithmException | NoSuchProviderException | InvalidKeyException | IOException ex) {
                logger.Trace("Error: TraiteRequetePayFacture "+e);
            }
        }

        return null;
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

        int factureId = requete.getIdFacture();
        try {
            PublicKey clePublique = RecupereClePubliqueClient();

            if (!requete.VerifySignature(clePublique))
                return new ReponseGetArticles(new ArrayList<>(), "Mauvaise signature", cleSession);

            ArrayList<Article> articles = beanMetier.getArticles(factureId);
            ReponseGetArticles reponse = new ReponseGetArticles(articles, "OK",cleSession);
            return reponse;
        }
        catch (Exception e)
        {
            logger.Trace("Server: Error TraiteRequeteGetArticles() "+e);
        }
        return null;
    }



    public static PublicKey RecupereClePubliqueClient() throws KeyStoreException, IOException, CertificateException, NoSuchAlgorithmException {
        KeyStore ks = KeyStore.getInstance("JKS");
        ks.load(new FileInputStream("KeyStoreServeur.jks"),"rti2023".toCharArray());
        X509Certificate certif = (X509Certificate)ks.getCertificate("cerclient");
        PublicKey cle = certif.getPublicKey();
        return cle;
    }
}
