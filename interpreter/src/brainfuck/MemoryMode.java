package brainfuck;

/**
 * Created by Marian on 20/08/2016.
 */
public final class MemoryMode {
    // The memory doubles every time the pointer accesses invalid memory
    public static final int UNBOUND = 0;
    // An exception is thrown if invalid memory is accessed
    public static final int FIXED = 1;
    // The pointer wraps around if invalid memory is accessed
    public static final int WRAP = 2;
}
