/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Modality;
import javafx.stage.Stage;

/**
 *
 * @author tkonno
 */
public class PowerConfigDialog {

    public static String showDialog(Scene pscene, String title, 
            String message, String prompt, String... namelist) {
        try {
            FXMLLoader loader = new FXMLLoader(PowerConfigDialog.class.getResource("PowerConfigDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            PowerConfigDialogController controller = loader.getController();
            controller.set(message, prompt, namelist);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene == null) dialog.initOwner(pscene.getWindow());
            dialog.initModality(Modality.WINDOW_MODAL);
            dialog.setResizable(false);
            dialog.setTitle(title);
            dialog.showAndWait();
            return controller.getText();
        } catch (IOException ex) {
            Logger.getLogger(PowerConfigDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
