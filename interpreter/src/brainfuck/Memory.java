package brainfuck;

import com.sun.javafx.binding.StringFormatter;

import java.util.Arrays;
import java.util.stream.IntStream;
import java.util.stream.LongStream;

import static java.util.Arrays.*;

/**
 * Created by Marian on 20/08/2016.
 */
public class Memory {
    public long memory[];
    public long mask;
    public int cellSize;
    public int ptr;
    public int mode = MemoryMode.UNBOUND;
    public int memoryLimit = 2048;

    public Memory(int size, int cellSizeInBytes) {
        this.memory = new long[size];
        if (cellSizeInBytes < 8)
            this.mask = (1L << (cellSizeInBytes << 3L)) - 1L;
        else if (cellSizeInBytes == 8) this.mask = -1;
        else throw new IllegalArgumentException("Maximum cell size is 8 bytes");
        this.cellSize = cellSizeInBytes;
        ptr = 0;
    }

    public long read() { return read(ptr); }

    public long read(int address) { return memory[address]; }

    public void inc() { write(read() + 1); }

    public void dec() { write(read() - 1); }

    public void add(long value) {
        write(read() + value);
    }

    public void sub(long value) {
        write(read() - value);
    }

    public void write(long value) {
        memory[ptr] = value & mask;
    }

    public void reset() {
        memory = new long[memory.length];
        ptr = 0;
    }

    public void previous() {
        if (--ptr < 0) {
            if (mode == MemoryMode.WRAP) {
                ptr = memory.length - 1;
            } else
                throw new RuntimeException("Memory access underflow");
        }
    }

    public void next() {
        if (++ptr >= memory.length) {
            if (mode == MemoryMode.WRAP)
                ptr = 0;
            else if (mode == MemoryMode.UNBOUND) {
                int newLength = memory.length << 1;
                if (newLength > memoryLimit)
                    throw new RuntimeException("Memory limit of " + memoryLimit + " reached!");
                long[] m = memory;
                memory = new long[newLength];
                System.arraycopy(m, 0, memory, 0, m.length);
            } else {
                throw new RuntimeException("Memory access overflow");
            }
        }
    }

    public String dump(int from, int to) {
        int len = Integer.toHexString(memory.length).length();
        int cellsPerRow = 8 / cellSize;
        int cellWidth = 2 * cellSize;
        StringBuilder sb = new StringBuilder();
        for (int i = from; i < to; i += cellsPerRow) {
            sb.append(String.format("%0"+len+"X |", i));
            for (int j = 0; j < cellsPerRow; j++) {
                if (i + j == ptr)
                    sb.append('>');
                else sb.append(' ');
                if (i + j < to) {
                    String val = Long.toHexString(read(i + j)).toUpperCase();
                    sb.append((new String(new char[cellWidth]).replace('\0', '0') + val).substring(val.length()));
                } else sb.append(new String(new char[cellWidth]).replace('\0', '?'));
            }
            sb.append(" | ");
            for (int j = 0; j < cellsPerRow; j++) {
                if (i + j < to) {
                    for (int w = cellSize - 1; w >= 0; --w) {
                        char c = (char)(read(i + j) >> (2 * w));
                        sb.append(c > 0x20 && c < 0x7F ? c : '.');
                    }
                } else sb.append(new String(new char[cellSize]).replace('\0','?'));
            }
            sb.append('\n');
        }
        return sb.toString();
    }

    public int getLength() {
        return memory.length;
    }

    public String dump() {
        return dump(0, memory.length);
    }
}
