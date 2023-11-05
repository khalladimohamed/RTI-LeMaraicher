import controller.MainWindowController;
import Vue.MainWindow;

public class Main {
    public static void main(String[] args) {
        MainWindow mainWindow = new MainWindow();
        MainWindowController mainWindowController = new MainWindowController(mainWindow);
        mainWindow.setController(mainWindowController);


    }
}