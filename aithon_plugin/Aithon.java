// {{{ imports
import java.awt.image.BufferedImage;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Color;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

import javax.imageio.ImageIO;
import javax.swing.BoxLayout;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JButton;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.JScrollPane;
import javax.swing.JLabel;
import javax.swing.ImageIcon;

import org.gjt.sp.jedit.Buffer;
import org.gjt.sp.jedit.EBComponent;
import org.gjt.sp.jedit.EBMessage;
import org.gjt.sp.jedit.EditBus;
import org.gjt.sp.jedit.GUIUtilities;
import org.gjt.sp.jedit.View;
import org.gjt.sp.jedit.jEdit;
import org.gjt.sp.jedit.gui.DefaultFocusComponent;
import org.gjt.sp.jedit.gui.DockableWindowManager;
import org.gjt.sp.jedit.msg.PropertiesChanged;
import org.gjt.sp.util.Log;
import org.gjt.sp.util.StandardUtilities;
// }}}

// {{{ Aithon_Plugin class
/**
 * 
 * Aithon Plugin
 *
 */
public class Aithon extends JPanel
implements ActionListener, EBComponent, AithonActions, 
           DefaultFocusComponent {

  // {{{ Instance Variables
  private String filename;
  private String defaultFilename;
  private View view;
  private boolean floating;

  private JButton detectButton;
  private JButton uploadButton;
  private JButton compileButton;
  private JToggleButton showSerialTerminal;
  private JTextArea console_area;
  private JScrollPane console_scrollbars;
  private Runtime r;
  private Process p;
  private Thread t;
  private BufferedWriter out;
    // }}}

    // {{{ Constructor
  /**
   * 
   * @param view the current jedit window
   * @param position a variable passed in from the script in actions.xml,
   * 	which can be DockableWindowManager.FLOATING, TOP, BOTTOM, LEFT, RIGHT, etc.
   * 	see @ref DockableWindowManager for possible values.
   */
  public Aithon(View view, String position) throws IOException{
    //The top level layout is a BoxLayout with the boxes side by side.
    //The buttons on the left and the console on the right.
    this.setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
    this.view = view;
    this.floating = position.equals(DockableWindowManager.FLOATING);

    if (floating)
      this.setPreferredSize(new Dimension(500, 250));

    //The buttons panel is a FlowLayout with a fixed maximum size which
    //forces the buttons to be arranged in a single column.
    BufferedImage aithon_logo = ImageIO.read(getClass().getResource("/images/AithonEmblemRedClear.png"));
    JLabel picLabel = new JLabel(new ImageIcon(aithon_logo));
    add(picLabel);
    
    Dimension button_size = new Dimension(120, 25);
    JPanel buttons = new JPanel();
    buttons.setLayout(new FlowLayout());
    buttons.setPreferredSize(new Dimension(150,180));
    buttons.setMaximumSize(new Dimension(150, 180));
    
    detectButton = new JButton("Detect\nBoard");
    detectButton.setText("Detect Board");
    detectButton.setPreferredSize(button_size);
    detectButton.addActionListener(this);
    
    compileButton = new JButton("Compile");
    compileButton.setText("<html><center>"+"Compile"+"</center></html>");
    compileButton.setPreferredSize(button_size);
    compileButton.addActionListener(this);
    
    uploadButton = new JButton("Upload");
    uploadButton.setText("<html><center>"+"Upload"+"</center></html>");
    uploadButton.setPreferredSize(button_size);
    uploadButton.addActionListener(this);
    
    showSerialTerminal = new JToggleButton("Serial Terminal");
    showSerialTerminal.setPreferredSize(button_size);
    
    buttons.add(detectButton);
    buttons.add(compileButton);
    buttons.add(uploadButton);
    buttons.add(showSerialTerminal);

    add(buttons);

    //create the console area
    console_area = new JTextArea("Aithon console area:\n");
    console_area.setLineWrap(true);
    console_area.setWrapStyleWord(true);
    console_area.setEditable(false);

    console_scrollbars = new JScrollPane (console_area,
    	    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, 
    	    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    Color color=new Color(0,0,0); //set background to black
    console_area.setBackground(color);

    Color color2=new Color(180,180,180); //set foreground to gray
    console_area.setForeground(color2);

    add(console_scrollbars);

    r = Runtime.getRuntime();
    
    //If property for any of the paths hasn't been set (is null or empty string)
    //Calls functions to find a default directory and sets the property
    String prop = jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "gcc-filepath");
    if (prop == null || prop.equals("")) {
      autoDetectGcc();
    }
    prop = jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "library-filepath");
    if (prop == null || prop.equals("")) {
      aithonLibraryPath();
    }
    prop = jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "programmer-filepath");
    if (prop == null || prop.equals("")) {
      aithonProgrammerPath();
    }
    
  }

  //starts a thread that reads an OutputStream and displays it to the console
  void inputStreamToOutputStream(final InputStream inputStream) {
    t = new Thread(new Runnable() {
          public void run() {
            try {
              int d;
              //if there is nothing to read then block, otherwise print
              while ((d = inputStream.read()) != -1) {
                console_area.append(Character.toString((char)d));
                console_area.setCaretPosition (console_area.getDocument().getLength());
              }
            } catch (IOException e) {
              System.err.println("Caught IOException: " + e.getMessage());
            }
          }
        });
    t.setDaemon(true);  //make this a daemon thread so the JVM will exit
    t.start();
  }

  //invoked when the buttons are clicked
  public void actionPerformed(ActionEvent evt) {
    Buffer curr_buffer = jEdit.getLastBuffer();
    Process compile;
    String line, makefile_path, lib_path;
    
    lib_path = jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "library-filepath");
    //Location of makefile and main.c - make general with working directory somehow
    makefile_path = lib_path + "/../ProjectTemplate/";
    //File dir = new File("C:\\Users\\Justine Dunham\\Documents\\GitHub\\aithon\\ProjectTemplate");
    //File dir = new File("/Users/jseng/Desktop/jEdit.app/Contents/Resources/Java/ProjectTemplate");
    File dir = new File(makefile_path);    
    Object src = evt.getSource();
    
    if (src == uploadButton) { //check if upload clicked
      //Using for testing - prints out current directories from property values
      console_area.append("Compiler: " + jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "gcc-filepath") + "\n");
      console_area.append("Library: " + jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "library-filepath") + "\n");
      console_area.append("Programmer: " + jEdit.getProperty(AithonPlugin.OPTION_PREFIX + "programmer-filepath") + "\n");
      //scroll the area
      console_area.setCaretPosition (console_area.getDocument().getLength());
    } else if (src == detectButton) { //check if detect clicked
      //console_area.append("Detect board\n");
      detectAithonBoard();
      //scroll the area
      console_area.setCaretPosition (console_area.getDocument().getLength());
    } else if (src == compileButton) { //check if compile clicked
      try {
      	//Path environment variables - required for mac/linux, use null for windows
        //String env[] = {"PATH=/usr/bin:/bin:/usr/sbin:/Users/jseng/gccarm/bin"};
        String env[] = null;
        String user_src = "USERFOLDER=\"" + curr_buffer.getDirectory().replace(" ", "\\s") + "\"";
        
        String make_cmd[] = {"make", user_src};
        console_area.append(user_src + "\n");
      	compile = r.exec(make_cmd, env, dir);
      	inputStreamToOutputStream(compile.getInputStream());
      	inputStreamToOutputStream(compile.getErrorStream());
        //scroll the area
        console_area.setCaretPosition (console_area.getDocument().getLength());
      } catch (IOException e) {
        System.err.println("Caught IOException: " + e.getMessage());
      }
    }
  }

  //Finds default directory for compiler
  private String detectAithonBoard() {
     Process detect;
     String path = "";
     String os = System.getProperty("os.name").toLowerCase();
     String userDir = System.getProperty("user.dir");

     if (os.indexOf("win") >= 0) {
        path = userDir + "/Windows";
     } else if (os.indexOf("mac") >= 0) {
        try { 
        String env[] = {"PATH=/usr/bin:/bin:/usr/sbin:."};
        path = userDir + "/jEdit.app/Contents/Resources/Java/AithonLibrary/Programmer/MacOSX/AithonProgrammer.app/Contents/MacOS";
        File dir = new File(path);    
        String detect_cmd[] = {path + "/AithonProgrammer", "detect"};

        //console_area.append(path + "\n");
        detect = r.exec(detect_cmd, env, dir);
        inputStreamToOutputStream(detect.getInputStream());
        inputStreamToOutputStream(detect.getErrorStream());
        } catch (IOException e) {
           System.err.println("Caught IOException: " + e.getMessage());
        }

        //scroll the area
        console_area.setCaretPosition (console_area.getDocument().getLength());
     } else if (os.indexOf("nux") >= 0) {
        path = userDir + "/Linux";
     }

     //jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "gcc-filepath", path);
     return path;
  }

    //Finds default directory for compiler
  private String autoDetectGcc() {
    String path = "";
    String os = System.getProperty("os.name").toLowerCase();
    String userDir = System.getProperty("user.dir");

    if (os.indexOf("win") >= 0) {
      path = userDir + "/Windows";
    } else if (os.indexOf("mac") >= 0) {
      path = userDir + "/MacOSX";
    } else if (os.indexOf("nux") >= 0) {
      path = userDir + "/Linux";
    }

    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "gcc-filepath", path);
    return path;
  }

  //Finds default directory for library
  private String aithonLibraryPath() {
    String path = jEdit.getJEditHome() + "/AithonLibrary/";
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "library-filepath", path);
    return path;
  }

  //Finds default directory for programmer/uploader
  private String aithonProgrammerPath() {
    String path = jEdit.getJEditHome() + "/AithonLibrary/Programmer";
    jEdit.setProperty(AithonPlugin.OPTION_PREFIX + "programmer-filepath", path);    
    return path;
  }
  // }}}

    // {{{ Member Functions
    
    // {{{ focusOnDefaultComponent
	public void focusOnDefaultComponent() {
	}
    // }}}

    // {{{ getFileName
	public String getFilename() {
		return filename;
	}
    // }}}

	// EBComponent implementation
	
    // {{{ handleMessage
	public void handleMessage(EBMessage message) {
		if (message instanceof PropertiesChanged) {
			propertiesChanged();
		}
	}
    // }}}
    
    // {{{ propertiesChanged
	private void propertiesChanged() {
		String propertyFilename = jEdit
				.getProperty(AithonPlugin.OPTION_PREFIX + "filepath");
		if (!StandardUtilities.objectsEqual(defaultFilename, propertyFilename)) {
			saveFile();
			//toolPanel.propertiesChanged();
			defaultFilename = propertyFilename;
			filename = defaultFilename;
		}
		Font newFont = AithonOptionPane.makeFont();
		//if (!newFont.equals(textArea.getFont())) {
		//	textArea.setFont(newFont);
		//}
	}
    // }}}

	// These JComponent methods provide the appropriate points
	// to subscribe and unsubscribe this object to the EditBus.

    // {{{ addNotify
	public void addNotify() {
		super.addNotify();
		EditBus.addToBus(this);
	}
     // }}}
     
    // {{{ removeNotify
	public void removeNotify() {
		saveFile();
		super.removeNotify();
		EditBus.removeFromBus(this);
	}
    // }}}
    
	// AithonActions implementation

    // {{{
	public void saveFile() {
		if (filename == null || filename.length() == 0)
			return;
		try {
			FileWriter out = new FileWriter(filename);
			//out.write(textArea.getText());
			out.close();
		} catch (IOException ioe) {
			Log.log(Log.ERROR, Aithon.class,
					"Could not write notepad text to " + filename);
		}
	}
    // }}}
    
    // {{{ chooseFile
	public void chooseFile() {
		String[] paths = GUIUtilities.showVFSFileDialog(view, null,
				JFileChooser.OPEN_DIALOG, false);
		if (paths != null && !paths[0].equals(filename)) {
			saveFile();
			filename = paths[0];
			//toolPanel.propertiesChanged();
		}
	}
    // }}}

    // {{{ copyToBuffer
	public void copyToBuffer() {
		jEdit.newFile(view);
	}
    // }}}
    // }}}
}
// }}}
