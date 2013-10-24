import java.lang.Thread;
import java.util.Scanner;
import java.io.Console;

class TestShell {
  public static void main(String[] args) {
    int loop = 1;

    Scanner scanner = new Scanner(System.in);
    System.out.println("TestShell started.");

    while(loop == 1) {
      String i = scanner.nextLine();

      System.out.println("Received: " + i);

      if (i.equals("q")) {
          loop = 0;
      } else if (i.equals("upload")) {
        System.out.print("0% |");
        for (int j=0; j<50;j++) {
          System.out.print ("#");
          try {
            Thread.sleep(20);
          } catch(InterruptedException ex) {
            Thread.currentThread().interrupt();
          }
        }
        System.out.print("| 100%\n");
      }
    }
  }
}
