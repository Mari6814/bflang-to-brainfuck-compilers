package brainfuck;

import java.util.Scanner;

/**
 * The implementation of the IO the Interpreter uses on default
 * Created by Marian on 21/08/2016.
 */
public class StdIO extends InterpreterIO {
    private static StdIO instance = new StdIO();
    private Scanner in;
    private StringBuilder sb;
    private boolean directOutput = true;

    private StdIO(){}

    public static StdIO getInstance() {
        return instance;
    }

    @Override
    public void reset() {
        instance = this;
        in = new Scanner(System.in);
        sb = new StringBuilder();
    }

    @Override
    public void print(long msg) {
        System.out.print((char)msg);
    }

    @Override
    public long nextLong() {
        return in.nextLong();
    }

    @Override
    public void log(String msg, String label) {
        if (directOutput)
            System.out.print(msg);
        sb.append(msg);
    }

    // Switches bettween saving the log in an StringBuilder or directly printing it to stdout
    public void setDirectOutput(boolean value) {
        directOutput = value;
    }

    // Returns the saved log
    public String getLog() {
        return sb.toString();
    }
}
