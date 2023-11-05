package VESPAP;

public class RequetePayFacture implements Requete
{
    private int idFacture;
    private String nom;
    private int numeroCarte;

    public RequetePayFacture(int id, String n, int numCarte)
    {
        idFacture = id;
        nom = n;
        numeroCarte = numCarte;
    }

    public int getIdFacture() {
        return idFacture;
    }

    public String getNom() {
        return nom;
    }

    public int getNumeroCarte() {
        return numeroCarte;
    }
}
