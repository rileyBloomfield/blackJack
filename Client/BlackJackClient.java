
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

/**
 *
 * @author Riley
 */
public class BlackJackClient extends JFrame{

    //Global text pane
    //Set display box that will appear on right side of JFrame
        JTextArea textField = new JTextArea();
        //change font type and size
        Font font = new Font("Verdana", Font.PLAIN, 16);
        //encapsulate text box in scrollable box
        JScrollPane insideText = new JScrollPane(textField);
        
        //Global buttons
        JButton upButton = new JButton("Hit");
        JButton leftButton = new JButton("Pass");
        JButton connectButton = new JButton("Connect");
        JButton disconnectButton = new JButton("Disconnect");
        
        //Global card values
        String dealerCard, clientCard1, clientCard2;
        int cardTotal = 0;
    
    JLabel jlbHelloWorld;
    private OutputStream socketOutput;
    private InputStream socketInput;
    private Socket theSocket;

    public void SendString(String s) {
        try {
            socketOutput.write(s.getBytes());
            System.out.println("Sent:" + s);
        } catch (IOException e) {
            System.out.println(e.toString());
        }
    }

    public class UpButtonListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            SendString("hit");
        }
    }

    public class LeftButtonListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            upButton.setEnabled(false);
            leftButton.setEnabled(false);
            SendString("pass,"+cardTotal);
        }
    }

    public class ConnectionThread extends Thread {
        
        public ConnectionThread() {

        }
        //Main Game logic
        @Override
        public void run() {
            try {
                byte[] readBuffer = new byte[256];
                while (true) {
                    socketInput.read(readBuffer);
                    String decoded = new String(readBuffer, "UTF-8");
                    System.out.println("Got:" + decoded);
                    //Parse message recieved
                    String[] message = decoded.split(",");
                    if (message[0].equals("Cards")) {
                        textField.setText("");
                        dealerCard = message[1];
                        clientCard1 = message[2];
                        clientCard2 = message[3];
                        textField.append(" Dealer Face Card: "+dealerCard+"\n Your Cards: "+clientCard1+"  "+clientCard2+"\n");
                        //Parse current card total
                        String firstCard = clientCard1.substring(0, clientCard1.length()-1);
                        String secondCard = clientCard2.substring(0, clientCard2.length()-1);
                        int firstCardValue = Integer.parseInt(firstCard);
                        int secondCardValue = Integer.parseInt(secondCard);
                        cardTotal += firstCardValue;
                        cardTotal += secondCardValue;
                        textField.append(" Your card total is: "+cardTotal+"\n");
                    }
                    if (message[0].equals("waiting")) {
                        
                    }
                    if (message[0].equals("yourTurn")) {
                        textField.append(" "+message[1]+"\n\n");
                        leftButton.setEnabled(true);
                        upButton.setEnabled(true);
                    }
                    if (message[0].equals("hit")) {
                        textField.append(" Additional Hit Card: "+message[1]+"\n");
                        String hitCard = message[1].substring(0, message[1].length()-1);
                        int hitCardValue = Integer.parseInt(hitCard);
                        cardTotal += hitCardValue;
                        if (cardTotal>21) {
                            textField.append(" Busted! Your card total was: "+cardTotal+"\n\n");
                            SendString("pass,"+cardTotal);
                            upButton.setEnabled(false);
                            leftButton.setEnabled(false);
                            //cardTotal = 0;
                        }
                        else {
                            textField.append(" Your card total is: "+cardTotal+"\n\n");
                        }
                    }
                    if (message[0].equals("winner")) {
                        textField.setText("You won with "+cardTotal+". The dealer's score was "+message[1]);
                    }
                    if (message[0].equals("loser")) {
                        textField.setText("You lost with "+cardTotal+". The dealer's score was "+message[1]);
                    }
                    if (message[0].equals("waiting")) {
                        textField.setText("waiting for other player...\n");
                    }
                    if (message[0].equals("failed")) {
                    cardTotal = 0;
                    theSocket.close();
                    textField.setText(message[1]+"\n");
                    connectButton.setEnabled(true);
                    disconnectButton.setEnabled(false);
                    upButton.setEnabled(false);
                    leftButton.setEnabled(false);
                    break;
                    }
                    readBuffer = new byte[256];
                }
            } catch (IOException e) {
                System.out.println(e.toString());
            }
        }
    }
    ConnectionThread theConnection;

    public class ConnectButtonListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {
            try {
                theSocket = new Socket(InetAddress.getByName("127.0.0.1"), 2999);
                socketOutput = theSocket.getOutputStream();
                socketInput = theSocket.getInputStream();
                theConnection = new ConnectionThread();
                theConnection.start();
                System.out.println("connect");
                connectButton.setEnabled(false);
                disconnectButton.setEnabled(true);
            } catch (IOException ex) {
                System.out.println(ex.toString());
            }
        }
    }

    class OnWindowClose extends WindowAdapter {

        public void windowClosing(WindowEvent e) {
            try {
                theSocket.close();
            } catch (Exception ex) {
                System.out.println(ex.toString());
            }
            System.exit(0);
        }
    }

    public class DisconnectButtonListener implements ActionListener {

        @Override
        public void actionPerformed(ActionEvent e) {

            //try {
             //   theSocket.close();
           // } catch (Exception ex) {
           //     System.out.println("Could not close.");
            //}
            SendString("done");
            cardTotal = 0;
            textField.setText(" Disconnected. Press connect to begin another game.");
            connectButton.setEnabled(true);
            disconnectButton.setEnabled(false);
            upButton.setEnabled(false);
            leftButton.setEnabled(false);

            System.out.println("Disconnected");
        }
    }

    public static void main(String args[]) {
        BlackJackClient frontEndMain = new BlackJackClient();
    }

    BlackJackClient() {
        
        //setDefaultCloseOperation(EXIT_ON_CLOSE);
        addWindowListener(new OnWindowClose());
        textField.setFont(font);
        textField.setDisabledTextColor(Color.WHITE); //change text color
        textField.setEnabled(false); //make read only text displayed
        textField.setBackground(Color.BLACK);
        leftButton.setEnabled(false);
        upButton.setEnabled(false);
        disconnectButton.setEnabled(false);
        jlbHelloWorld = new JLabel("Hello World");
        
        upButton.addActionListener(new UpButtonListener());
        upButton.setSize(100, 20);

        leftButton.addActionListener(new LeftButtonListener());
        leftButton.setSize(100, 20);

        JPanel directionPanel = new JPanel(new GridBagLayout());
        GridBagConstraints c = new GridBagConstraints();
        c.gridx = 0;
        c.gridy = 1;
        directionPanel.add(upButton, c);
        c.gridx = 1;
        c.gridy = 0;
        directionPanel.add(leftButton, c);

        JPanel connectionPanel = new JPanel(new GridBagLayout());
        
        connectButton.addActionListener(new ConnectButtonListener());
        c.gridx = 0;
        c.gridy = 0;
        connectionPanel.add(connectButton, c);
        
        disconnectButton.addActionListener(new DisconnectButtonListener());
        c.gridx = 1;
        c.gridy = 0;
        connectionPanel.add(disconnectButton, c);

        Rectangle theBounds = getBounds();
        Dimension d = new Dimension();
        d.width = 500;
        d.height = 450;

        setLayout(new GridBagLayout());
        c.gridx = 0;
        c.gridy = 0;
        c.gridheight = 9;
        c.gridwidth = 10;
        c.weightx = 1.0;
        c.weighty = 1.0;
        c.fill = GridBagConstraints.BOTH;
        add(insideText, c);
        c.gridx = 0;
        c.gridy = 9;
        c.gridheight = 1;
        c.gridwidth = 2;
        c.fill = GridBagConstraints.NONE;
        add(connectionPanel, c);
        c.gridx = 7;
        c.gridy = 9;
        c.gridheight = 1;
        c.gridwidth = 2;
        c.fill = GridBagConstraints.NONE;
        add(directionPanel, c);
        this.setSize(500, 500);
        this.setTitle("SE3313 Blackjack");

        setVisible(true);
    }
    
}
