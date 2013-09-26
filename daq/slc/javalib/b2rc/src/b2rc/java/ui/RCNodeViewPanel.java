package b2rc.java.ui;

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;

import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;

import b2daq.ui.Updatable;
import b2rc.core.RCCommand;
import b2rc.core.RCNode;
import b2rc.core.RCState;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.io.RunControlMessage;

public class RCNodeViewPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private RCNode _node;
	private RCNodeStateLabel _state;

	public RCNodeViewPanel(RCNode node) {
		_node = node;
		_state = new RCNodeStateLabel(node);
	}

	public void init() {
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);

		setGrid(gbc, 0, 0, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_nodename = new JLabel(_node.getName());
		label_nodename.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_nodename, gbc);
		add(label_nodename);

		setGrid(gbc, 1, 0, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		layout.setConstraints(_state, gbc);
		add(_state);

		addMouseListener(new MouseListener() {
			@Override
			public void mouseClicked(MouseEvent e) {
				if (SwingUtilities.isRightMouseButton(e)) {
					JPopupMenu popup_menu = new JPopupMenu();
					ArrayList<RCCommand> command_v = new ArrayList<RCCommand>();
					if (_node.getState().equals(RCState.INITIAL_S)) {
						command_v.add(RCCommand.BOOT);
					} else if (_node.getState().equals(RCState.CONFIGURED_S)) {
						command_v.add(RCCommand.REBOOT);
						command_v.add(RCCommand.LOAD);
					} else if (_node.getState().equals(RCState.READY_S)) {
						command_v.add(RCCommand.RELOAD);
						command_v.add(RCCommand.START);
					} else if (_node.getState().equals(RCState.RUNNING_S)) {
						command_v.add(RCCommand.PAUSE);
					} else if (_node.getState().equals(RCState.PAUSED_S)) {
						command_v.add(RCCommand.RESUME);
					}
					command_v.add(RCCommand.RECOVER);
					JMenu menu = new JMenu("Command");
					popup_menu.add(menu);
					for (RCCommand command : command_v) {
						JMenuItem item = new JMenuItem(command.getAlias());
						item.addActionListener(new CommandMenuItemListener(
								command));
						menu.add(item);
					}
					popup_menu.show(e.getComponent(), e.getX(), e.getY());
				}
			}

			@Override
			public void mousePressed(MouseEvent e) {
			}

			@Override
			public void mouseReleased(MouseEvent e) {
			}

			@Override
			public void mouseEntered(MouseEvent e) {
			}

			@Override
			public void mouseExited(MouseEvent e) {
			}

		});
	}

	public void update() {
		_state.update(_node);
	}

	private void setGrid(GridBagConstraints gbc, int gridwidth, int gridheight,
			int gridx, int gridy, double weightx, double weighty, int fill,
			int anchor, Insets insets) {
		gbc.gridwidth = gridwidth;
		gbc.gridheight = gridheight;
		gbc.gridx = gridx;
		gbc.gridy = gridy;
		gbc.weightx = weightx;
		gbc.weighty = weighty;
		gbc.fill = fill;
		// gbc.anchor = GridBagConstraints.LINE_START;
		gbc.anchor = anchor;
		gbc.insets = insets;
	}

	private void setGrid(GridBagConstraints gbc, int gridx, int gridy,
			double weightx, double weighty, int fill, int anchor, Insets insets) {
		setGrid(gbc, 1, 1, gridx, gridy, weightx, weighty, fill, anchor, insets);
	}

	private class CommandMenuItemListener implements ActionListener {
		private RCCommand _command;

		public CommandMenuItemListener(RCCommand command) {
			_command = command;
		}

		public void actionPerformed(ActionEvent arg0) {
			int [] value_v = new int[1];
			value_v[0] = _node.getIndex();
			try {
				RCServerCommunicator.get().sendMessage(new RunControlMessage(_command, value_v));
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

}
