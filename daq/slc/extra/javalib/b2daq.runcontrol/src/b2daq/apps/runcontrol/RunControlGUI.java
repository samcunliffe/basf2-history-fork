/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.NSMListenerService;
import b2daq.ui.NSM2Socket;
import b2daq.ui.NetworkConfigPaneController;
import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.fxml.JavaFXBuilderFactory;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.Stage;

/**
 *
 * @author tkonno
 */
public class RunControlGUI extends Application {

    private FXMLLoader loader;
    static private String[] arguments;

    @Override
    public void start(Stage stage) {
        try {
            NSM2Socket socket = NSM2Socket.connect((arguments.length>1?arguments[1]:".runcontrol.init"),
                    null, 9090, null, 8122, "RC_GUI", "ARICH_RC", 
                    new String [] {"ARICH_RC_STATUS:rc_status:1"},
                    "Login to Belle II Run control", 
                    "NSM set up for run control");
            if (socket == null) return;
            URL location = getClass().getResource("RunControlMainPane.fxml");
            loader = new FXMLLoader();
            loader.setLocation(location);
            loader.setBuilderFactory(new JavaFXBuilderFactory());
            Parent root = (Parent) loader.load(location.openStream());
            RunControlMainPaneController controller
                    = ((RunControlMainPaneController) loader.getController());
            NetworkConfigPaneController netconf = controller.getNetworkConfig();
            netconf.setConfig(socket.getConfig());
            for (NSMDataProperty data : socket.getNSMConfig().getNSMDataProperties()) {
                netconf.add(data);
            }
            NSMListenerService.add(controller);
            NSMListenerService.add(netconf);
            NSMListenerService.restart();
            Scene scene = new Scene(root);
            scene.getStylesheets().add(LogMessage.getCSSPath());
            scene.getStylesheets().add(DataFlowMonitorController.class.getResource("DataFlowMonitor.css").toExternalForm());
            stage.setTitle("Belle II Run Control GUI");
            stage.getIcons().add(new Image(RunControlGUI.class.getResource("runcontrol.png").toExternalForm()));
            stage.setScene(scene);
            stage.show();
        } catch (Exception ex) {
            Logger.getLogger(RunControlGUI.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    public void stop() {
        try {
            NSMListenerService.close();
            System.exit(1);
        } catch (Exception e) {

        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        arguments = args;
        launch(args);
    }

}
