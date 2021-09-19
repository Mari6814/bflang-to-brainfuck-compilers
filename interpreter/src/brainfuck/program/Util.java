package brainfuck.program;

import javafx.util.Pair;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Pattern;

/**
 * Created by marian on 12/10/16.
 */
final class Util {

    /**
     * repeats a character count times
     * @param c Charakter to repeat
     * @param count Repeat count
     * @return string
     */
    static String repeat(char c, int count) {
        return new String(new char[Math.abs(count)]).replace('\0', c);
    }

    /**
     * Repeats a string count times
     * @param str String to repeat
     * @param count Repeat count
     * @return repeated string
     */
    static String repeat(String str, int count) {
        StringBuilder sb = new StringBuilder();
        count = Math.abs(count);
        while (count-- > 0)
            sb.append(str);
        return sb.toString();
    }

    /**
     * Test if a string can be fully converted into an integer
     * @param str String to match
     * @return true if integer, false otherwise
     */
    static boolean isInteger(String str) {
        return isInt.matcher(str).matches();
    }

    /**
     * Splits a string by the programs scope separator
     * @param path Path with SCOPE_SEPERATORs
     * @return Splitted identifiers
     */
    static List<String> splitPath(String path) {
        return Arrays.asList(path.split("\\" + Program.SCOPE_SEPARATOR));
    }

    /**
     * Zips two lists
     * @param l0 Left
     * @param l1 Right
     * @return zipped list
     */
    static <K, V> List<Pair<K, V>> zip(List<K> l0, List<V> l1) {
        if (l0.size() != l1.size())
            throw new IllegalArgumentException("Lists have to be the same size in order to zip them");
        ArrayList<Pair<K, V>> pairs = new ArrayList<>();
        Iterator<K> i0 = l0.iterator();
        Iterator<V> i1 = l1.iterator();
        while (i0.hasNext() && i1.hasNext())
            pairs.add(new Pair<>(i0.next(), i1.next()));
        return pairs;
    }


    private static final Pattern isInt = Pattern.compile("^-?\\d+$");
}
