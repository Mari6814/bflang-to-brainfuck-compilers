package brainfuck;

/**
 * Interface the Interpreter uses for its IO methods used while interpreting
 * Created by Marian on 21/08/2016.
 */
public class InterpreterIO {
    /**
     * Requests the interface to print something
     * @param msg Message to print
     */
    void print(long msg) {
        System.out.print((char)msg);
    }

    /**
     * Requests a long from stdin
     * @return long from stdin
     */
    long nextLong() { return 0; }

    /**
     * Logs a unlabeled message
     * @param msg Message to log
     */
    void log(String msg) {
        System.out.print(msg);
    }

    /**
     * Gives a log message a label
     * @param mode Label to log it with
     * @param msg Message to log
     * @param label
     */
    void log(String msg, String label) {
        log(msg);
    }

    /**
     * Logs the unlabeled message
     * @param msg Message to print
     */
    void logln(String msg) { log(msg + '\n'); }

    /**
     * Logs the labeled message and appends a newline character
     * @param msg Message to print
     * @param label Message label
     */
    void logln(String msg, String label) { logln(msg); }

    /**
     * Resets the logging object
     */
    void reset() {};
}

