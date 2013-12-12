import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JTextField;

import org.gjt.sp.jedit.AbstractOptionPane;
import org.gjt.sp.jedit.GUIUtilities;
import org.gjt.sp.jedit.jEdit;
import org.gjt.sp.jedit.gui.FontSelector;
import org.gjt.sp.jedit.browser.VFSBrowser;

public class AithonOptionPane extends AbstractOptionPane implements
ActionListener {
  private JCheckBox showPath;
  private FontSelector font;
  private JTextField gccPath, libPath, progPath;
  private JButton gccPickPath, libPickPath, progPickPath;

  public AithonOptionPane() {
    super(AithonPlugin.NAME);
  }

  public void _init() {
    String prop;
    showPath = new JCheckBox(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX
          + "show-filepath.title"), jEdit.getProperty(
            AithonPlugin.OPTION_PREFIX + "show-filepath").equals(
              "true"));
    addComponent(showPath);

    //Textfield for finding the compiler's location
    gccPath = new JTextField(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX + "gcc-filepath"));
    gccPickPath = new JButton(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX + "choose-file"));
    gccPickPath.addActionListener(this);
    JPanel gccPanel = new JPanel(new BorderLayout(0, 0));
    gccPanel.add(gccPath, BorderLayout.CENTER);
    gccPanel.add(gccPickPath, BorderLayout.EAST);

    addComponent(jEdit.getProperty(AithonPlugin.OPTION_PREFIX
          + "gcc-label"), gccPanel);

    //Textfield for finding the library's location
    libPath = new JTextField(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX + "library-filepath"));
    libPickPath = new JButton(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX + "choose-file"));
    libPickPath.addActionListener(this);
    JPanel libPanel = new JPanel(new BorderLayout(0, 0));
    libPanel.add(libPath, BorderLayout.CENTER);
    libPanel.add(libPickPath, BorderLayout.EAST);

    addComponent(jEdit.getProperty(AithonPlugin.OPTION_PREFIX
          + "library-label"), libPanel);   
    
    //Textfield for finding the programmer's location
    progPath = new JTextField(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX + "programmer-filepath"));
    progPickPath = new JButton(jEdit
        .getProperty(AithonPlugin.OPTION_PREFIX + "choose-file"));
    progPickPath.addActionListener(this);
    JPanel progPanel = new JPanel(new BorderLayout(0, 0));
    progPanel.add(progPath, BorderLayout.CENTER);
    progPanel.add(progPickPath, BorderLayout.EAST);

    addComponent(jEdit.getProperty(AithonPlugin.OPTION_PREFIX
          + "prog-label"), progPanel);

    font = new FontSelector(makeFont());
    addComponent(jEdit.getProperty(AithonPlugin.OPTION_PREFIX
          + "choose-font"), font);
  }

  //Set properties of fields
  public void _save() {
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "gcc-filepath",
        gccPath.getText());
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "library-filepath",
        libPath.getText());
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "programmer-filepath",
        progPath.getText());
    Font _font = font.getFont();
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "font", _font
        .getFamily());
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "fontsize", String
        .valueOf(_font.getSize()));
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "fontstyle",
        String.valueOf(_font.getStyle()));
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "show-filepath",
        String.valueOf(showPath.isSelected()));
  }



  // end AbstractOptionPane implementation

  // begin ActionListener implementation
  public void actionPerformed(ActionEvent evt) {
    Object src = evt.getSource();
    String[] paths;
    if (src == gccPickPath) {
      paths = GUIUtilities.showVFSFileDialog(null, null,
        JFileChooser.OPEN_DIALOG, false);
      
      if (paths != null) {
        gccPath.setText(paths[0]);
      }
    }
    else {
      paths = GUIUtilities.showVFSFileDialog(null, null,
        VFSBrowser.CHOOSE_DIRECTORY_DIALOG, false);
      if (paths != null) {
        if (src == libPickPath) {
          libPath.setText(paths[0]);
        }
        else if (src == progPickPath) {
          progPath.setText(paths[0]);
        }
      }
    }
  }

  // helper method to get Font from plugin properties
  static public Font makeFont() {
    int style, size;
    String family = jEdit.getProperty(AithonPlugin.OPTION_PREFIX
        + "font");
    try {
      size = Integer
        .parseInt(jEdit
            .getProperty(AithonPlugin.OPTION_PREFIX
              + "fontsize"));
    } catch (NumberFormatException nf) {
      size = 14;
    }
    try {
      style = Integer
        .parseInt(jEdit
            .getProperty(AithonPlugin.OPTION_PREFIX
              + "fontstyle"));
    } catch (NumberFormatException nf) {
      style = Font.PLAIN;
    }
    return new Font(family, style, size);
  }

}
