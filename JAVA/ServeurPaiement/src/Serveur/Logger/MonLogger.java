package Serveur.Logger;

public class MonLogger implements Logger {

    public MonLogger()
    {

    }
    @Override
    public void Trace(String message) {
        System.out.println(message);
    }
}
