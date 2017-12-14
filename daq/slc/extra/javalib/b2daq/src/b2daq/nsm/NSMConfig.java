/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;

/**
 *
 * @author tkonno
 */
public class NSMConfig  {

    private final SimpleStringProperty hostname = new SimpleStringProperty();
    private final SimpleIntegerProperty port = new SimpleIntegerProperty();
    private final SimpleStringProperty guiNode = new SimpleStringProperty();
    private final SimpleStringProperty nsmHost = new SimpleStringProperty();
    private final SimpleIntegerProperty nsmPort = new SimpleIntegerProperty();
    private final SimpleStringProperty nsmNode = new SimpleStringProperty();

    public SimpleStringProperty hostnameProperty() {
        return hostname;
    }

    public SimpleIntegerProperty portProperty() {
        return port;
    }

    public SimpleStringProperty guiNodeProperty() {
        return guiNode;
    }

    public SimpleStringProperty nsmHostProperty() {
        return nsmHost;
    }

    public SimpleIntegerProperty nsmPortProperty() {
        return nsmPort;
    }

    public SimpleStringProperty nsmNodeProperty() {
        return nsmNode;
    }

    public String getHostname() {
        return hostname.get();
    }

    public int getPort() {
        return port.get();
    }

    public String getGuiNode() {
        return guiNode.get().toUpperCase();
    }

    public String getNsmHost() {
        return nsmHost.get();
    }

    public int getNsmPort() {
        return nsmPort.get();
    }

    public String getNsmNode() {
        return nsmNode.get().toUpperCase();
    }

    public void setHostname(String hostname) {
        this.hostname.set(hostname);
    }

    public void setPort(int port) {
        this.port.set(port);
    }

    public void setGuimNode(String nsmNode) {
        this.guiNode.set(nsmNode);
    }

    public void setNsmHost(String nsmHost) {
        this.nsmHost.set(nsmHost);
    }

    public void setNsmPort(int nsmPort) {
        this.nsmPort.set(nsmPort);
    }

    public void setNsmNode(String nsmNode) {
        this.nsmNode.set(nsmNode);
    }

}
