import Vue.MainWindow;
import Controleur.ControleurMainWindow;
import com.formdev.flatlaf.themes.FlatMacLightLaf;

import javax.swing.*;

public class Main {
    public static void main(String[] args) {
        FlatMacLightLaf.setup();

        // Affichage de la fenêtre
        try {
            UIManager.setLookAndFeel(new FlatMacLightLaf());
        } catch (Exception e) {
            e.printStackTrace();
        }

        MainWindow mainWindow = new MainWindow();
        ControleurMainWindow controleurMainWindow = new ControleurMainWindow(mainWindow);
        mainWindow.setController(controleurMainWindow);

    }
}