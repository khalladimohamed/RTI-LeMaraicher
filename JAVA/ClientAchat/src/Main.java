import Vue.MainWindow;
import Controleur.ControleurMainWindow;

public class Main {
    public static void main(String[] args) {

        MainWindow mainWindow = new MainWindow();
        ControleurMainWindow controleurMainWindow = new ControleurMainWindow(mainWindow);
        mainWindow.setController(controleurMainWindow);

    }
}