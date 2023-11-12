package Vue;

import javax.swing.*;
import javax.swing.table.DefaultTableModel;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.*;

import Controleur.ControleurMainWindow;

public class MainWindow extends JFrame {
    private JTextField textFieldLogin;
    private JTextField textFieldMdp;
    private JButton loginButton;
    private JButton logoutButton;
    private JCheckBox nouveauClientCheckBox;
    private JPanel ConnexionPanel;
    private JPanel MainPanel;
    private JPanel MagasinPanel;
    public JTextField ArticleTextField;
    private JTextField PrixUnitaireTextField;
    private JTextField StockTextField;
    private JSpinner QuantiteSpinner;
    private JButton previousArticleButton;
    private JButton nextArticleButton;
    private JPanel PublicitePanel;
    private JPanel PanierPanel;
    private JTable table;
    private JButton viderLePanierButton;
    private JButton supprimerArticleButton;
    private JButton confirmerAchatButton;
    private JTextField TotalTextField;
    private JLabel imageLabel;
    private JButton acheterButton;
    private JScrollPane JScrollPanier;


    public MainWindow() {

        JFrame frame = new JFrame("Le Maraicher en ligne");
        frame.setContentPane(MainPanel);
        frame.setSize(new Dimension(770, 618));
        frame.setVisible(true);
        frame.setResizable(false);

        // Fermeture de l'application
        frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        frame.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                System.out.println("Fermeture de l'application");
                System.exit(0);
            }
        });

        DefaultTableModel model = new DefaultTableModel();
        model.addColumn("Id");
        model.addColumn("Article");
        model.addColumn("Prix à l'unité");
        model.addColumn("Quantité");

        table = new JTable(model);
        JScrollPanier.setViewportView(table);
    }

    public void setController(ControleurMainWindow controleurMainWindow)
    {
        loginButton.addActionListener(controleurMainWindow);
        logoutButton.addActionListener(controleurMainWindow);
        acheterButton.addActionListener(controleurMainWindow);
        previousArticleButton.addActionListener(controleurMainWindow);
        nextArticleButton.addActionListener(controleurMainWindow);
        supprimerArticleButton.addActionListener(controleurMainWindow);
        viderLePanierButton.addActionListener(controleurMainWindow);
        confirmerAchatButton.addActionListener(controleurMainWindow);
    }

    public void setNom(String nom)
    {
        textFieldLogin.setText(nom);
    }

    public String getNom()
    {
        return textFieldLogin.getText();
    }

    public void setMotDePasse(String motDePasse)
    {
        textFieldMdp.setText(motDePasse);
    }

    public String getMotDePasse()
    {
        return textFieldMdp.getText();
    }

    public void setPrix(float prix)
    {
        if(prix >= 0)
            PrixUnitaireTextField.setText(String.valueOf(prix));
        else
            PrixUnitaireTextField.setText("");
    }

    public void setStock(int stock)
    {
        if(stock >= 0)
            StockTextField.setText(String.valueOf(stock));
        else
            StockTextField.setText("");
    }

    public int isNouveauClientChecked()
    {
        if(nouveauClientCheckBox.isSelected())
            return 1;
        else
            return 0;
    }

    public void setArticle(String article, float prix, int stock, String image) {
        ArticleTextField.setText(article);
        setPrix(prix);
        setStock(stock);


        // Load the image from the specified path
        ImageIcon imageIcon = new ImageIcon("src/images/" + image);

        // Create a JLabel to display the image
        imageLabel.setIcon(imageIcon);
    }

    public void setQuantite(int quantite)
    {
        QuantiteSpinner.setValue(quantite);
    }

    public int getQuantite()
    {
        return (int) QuantiteSpinner.getValue();
    }

    public void setTotal(float total)
    {
        if(total >= 0)
            TotalTextField.setText(String.format("%.2f", total));
        else
            TotalTextField.setText("");
    }

    public void LoginOK()
    {
        loginButton.setEnabled(false);
        logoutButton.setEnabled(true);
        textFieldLogin.setEnabled(false);
        textFieldMdp.setEnabled(false);
        nouveauClientCheckBox.setEnabled(false);

        QuantiteSpinner.setEnabled(true);
        previousArticleButton.setEnabled(true);
        nextArticleButton.setEnabled(true);
        acheterButton.setEnabled(true);
        supprimerArticleButton.setEnabled(true);
        viderLePanierButton.setEnabled(true);
        confirmerAchatButton.setEnabled(true);
    }

    public void LogoutOK()
    {
        loginButton.setEnabled(true);
        logoutButton.setEnabled(false);
        textFieldLogin.setEnabled(true);
        textFieldMdp.setEnabled(true);
        nouveauClientCheckBox.setEnabled(true);

        QuantiteSpinner.setEnabled(false);
        previousArticleButton.setEnabled(false);
        nextArticleButton.setEnabled(false);
        acheterButton.setEnabled(false);
        supprimerArticleButton.setEnabled(false);
        viderLePanierButton.setEnabled(false);
        confirmerAchatButton.setEnabled(false);

        setArticle("", -1, -1, "");

        setMotDePasse("");
        setNom("");

        setQuantite(0);

        if(nouveauClientCheckBox.isSelected())
            nouveauClientCheckBox.setSelected(false);

        videTablePanier();
        setTotal(-1);
    }

    public void ajouteArticleTablePanier(int id, String article, float prix, int quantite) {

        DefaultTableModel model = (DefaultTableModel) table.getModel();
        model.addRow(new Object[]{id, article, String.format("%.2f", prix)+"€", quantite});
    }

    public void videTablePanier() {
        DefaultTableModel model = (DefaultTableModel) table.getModel();
        model.setRowCount(0);
    }

    /*public int getIndiceArticleSelectionne()
    {
        return table.getSelectedRow();
    }*/

    public int getIdArticleSelectionne() {
        int selectedRow = table.getSelectedRow();

        if (selectedRow != -1) { // Vérifie que quelque chose est sélectionné
            Object idArticleValue = table.getValueAt(selectedRow, 0);
            if (idArticleValue instanceof Integer) {
                return (int) idArticleValue;
            }
        }

        // Valeur par défaut ou gestion d'erreur
        return -1;
    }

    public void dialogueMessage(String titre, String message) {
        JOptionPane.showMessageDialog(null, message, titre, JOptionPane.INFORMATION_MESSAGE);
    }

    public void dialogueErreur(String titre, String message) {
        JOptionPane.showMessageDialog(null, message, titre, JOptionPane.ERROR_MESSAGE);
    }

}
