import Serveur.Logger.MonLogger;
import Serveur.ThreadServeurPool;
import Serveur.Protocole.VESPAP;
import JDBC.BeanGenerique;
import JDBC.BeanMetier;

import java.sql.*;


public class Main {
    public static void main(String[] args) {

        try {
            BeanGenerique beanGenerique = new BeanGenerique(BeanGenerique.MYSQL, "192.168.109.130", "PourStudent", "Student", "PassStudent1_");
        }
        catch (ClassNotFoundException | SQLException e)
        {
            e.printStackTrace();
            System.exit(1);
        }

        MonLogger logger = new MonLogger();

        BeanMetier beanMetier = new BeanMetier(logger);
        VESPAP vespap = new VESPAP(logger, beanMetier);

        try {
            ThreadServeurPool threadServeurPool = new ThreadServeurPool(6666, vespap, 5, logger);
            threadServeurPool.start();
        }
        catch (Exception e)
        {
            System.out.println("Erreur lors du lancement du serveur");
            e.printStackTrace();
            System.exit(1);
        }

    }
}