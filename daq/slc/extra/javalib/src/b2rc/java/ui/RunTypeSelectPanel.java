package b2rc.java.ui;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;

import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import javax.swing.tree.DefaultMutableTreeNode;

import b2daq.java.io.ConfigFile;
import b2rc.java.io.ConfigTree;
import b2rc.java.io.RunTypeConfigTree;

public class RunTypeSelectPanel extends JPanel {

	private static final long serialVersionUID = 1L;

	private String _selected_runtype = "";
	
	protected class LabeledTreeNode extends DefaultMutableTreeNode {

		private static final long serialVersionUID = 1L;
		private String _label = "";

		public LabeledTreeNode(String title, String label) {
			super(title);
			_label = label;
		}

		public String getLabel() {
			return _label;
		}

	}

	public RunTypeSelectPanel(ConfigTree.Branch branch) {
		DefaultMutableTreeNode root = createNode(branch);
		JTree tree = new JTree(root);
		tree.setFont(new Font("Sans", Font.PLAIN, 13));
		tree.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseReleased(MouseEvent arg0) {
				JTree tree = (JTree) arg0.getSource();
				try {
					LabeledTreeNode node = (LabeledTreeNode)(tree.getLastSelectedPathComponent());
					_selected_runtype = node.getLabel();
				} catch (Exception e) {}
				tree.repaint();
			}
		});
		JScrollPane scrollPane = new JScrollPane();
		scrollPane.getViewport().setView(tree);
		scrollPane.setPreferredSize(new Dimension(220, 200));
		add(scrollPane);
	}
	
	public DefaultMutableTreeNode createNode(ConfigTree.Branch branch) {
		DefaultMutableTreeNode root = new DefaultMutableTreeNode(branch.getName());
		for (ConfigTree.Leaf leaf: branch.getLeafs()) {
			if (leaf.isLeaf()) {
				LabeledTreeNode node = new LabeledTreeNode(leaf.getName(), leaf.getLabel());
				root.add(node);
			} else {
				root.add(createNode((ConfigTree.Branch)leaf));
			}
		}
		return root;
	}

	public String getSelected() {
		return _selected_runtype;
	}

}
