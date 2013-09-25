package b2daq.java.ui;

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

public class TextBoxPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel _label;
	private JTextField _text_field;

	public TextBoxPanel(String title, String text) {
		super();
		setMaximumSize(new Dimension(300, 30));
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		_label = new JLabel(title);
		_label.setMaximumSize(new Dimension(90, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_label);
		_text_field = new JTextField(text);
		_text_field.setMaximumSize(new Dimension(150, 30));
		_text_field.setPreferredSize(new Dimension(150, 30));
		add(Box.createRigidArea(new Dimension(10, 30)));
		add(_text_field);
		add(Box.createRigidArea(new Dimension(10, 30)));
	}

	public void setText(String text) {
		_text_field.setText(text);
	}

	public String getText() {
		return _text_field.getText();
	}

	public void setEnabled(boolean enabled) {
		_text_field.setEnabled(enabled);
	}

}
