package brainfuck;

import brainfuck.program.DebugMode;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Iterator;

import static java.util.Arrays.copyOfRange;

/**
 * Created by Marian on 20/08/2016.
 */
public class Interpreter {
    private Memory memory;
    private char[] program;
    private int pc;
    private int debugMode = 0;
    private InterpreterIO io = StdIO.getInstance();
    private JumpTable jumpTable;
    private int maxInstructionCount = (int) 1e7;
    private long instructionCount;

    public Interpreter(int memorySize, int cellSizeInBytes) {
        memory = new Memory(memorySize, cellSizeInBytes);
    }

    /**
     * Interpreter with cell width of 1 byte
     * @param memory
     */
    public Interpreter(int memory) {
        this(memory, 1);
    }

    /**
     * Initial memory size of 16 bytes and cell width of 1 byte
     */
    public Interpreter() {
        this(16, 1);
    }

    public void loadProgramFromFile(String path) throws IOException {
        File f = new File(path);
        StringBuilder sb = new StringBuilder();
        Iterator<String> i = Files.lines(Paths.get(path)).iterator();
        while (i.hasNext())
            sb.append(i.next());
        loadProgramIntoMemory(sb.toString());
    }

    public int run() {
        io.reset();
        pc = 0;
        memory.ptr = 0;
        instructionCount = 0;
        long logCounter = 0;
        if (program.length != 0) {
            do {
                switch (program[pc]) {
                    case '+':
                        memory.inc();
                        break;
                    case '-':
                        memory.dec();
                        break;
                    case '>':
                        memory.next();
                        break;
                    case '<':
                        memory.previous();
                        break;
                    case '[':
                        if (memory.read() == 0)
                            pc = jumpTable.jump(pc) - 1;
                        break;
                    case ']':
                        if (memory.read() != 0)
                            pc = jumpTable.jump(pc) - 1;
                        break;
                    case '.':
                        io.print(memory.read());
                        break;
                    case ',':
                        memory.write(io.nextLong());
                        break;
                    case '#':
                        if ((debugMode & DebugMode.DUMP_ON_LOG_INSTRUCTION) > 0) {
                            ++logCounter;
                            int delta = 20;
                            io.logln("log:" + logCounter + " instruction:" + pc + " instructionCount:" + instructionCount + " ptr:" + Integer.toHexString(memory.ptr).toUpperCase(), "dump");
                            io.logln(String.format("%s %s %s", String.valueOf(getProgramRegion(pc - delta, pc)), program[pc], String.valueOf(getProgramRegion(pc + 1, pc + delta))));
                            io.logln(memory.dump(Math.max(0, memory.ptr - 8), memory.ptr + 16), "dump");
                        }
                        break;
                    default:
                        if ((debugMode & DebugMode.UNKNOWN_OPCODE) > 0)
                            io.logln("Unknown opcode: " + program[pc], "unknown_opcode");
                }
                ++instructionCount;
                if (maxInstructionCount > 0 && instructionCount > maxInstructionCount)
                    throw new RuntimeException("Timeout (max=" + maxInstructionCount + ")");
            } while (++pc < program.length);
        } else {
            io.logln("Program is empty", "empty");
        }
        return 0;
    }

    public void loadProgramIntoMemory(String programRaw, boolean optimize){
        if (optimize)
            this.program = optimize(programRaw).toCharArray();
        else this.program = programRaw.toCharArray();
        this.jumpTable = new JumpTable(this.program);
        this.pc = 0;
    }

    public void loadProgramIntoMemory(String programRaw) {
        loadProgramIntoMemory(programRaw, true);
    }

    private String optimize(String programRaw) {
        Optimizer removeInverse = new Optimizer() {
            private boolean isInSigma(char c) {
                return c == '+' || c == '-' || c == '>' || c == '<' || c == '[' || c == ']' || c == '.' || c == ',' || c == '#';
            }

            private boolean isOpposite(char a, char b) {
                if (a == '+' && b == '-') return true;
                if (a == '-' && b == '+') return true;
                if (a == '>' && b == '<') return true;
                if (a == '<' && b == '>') return true;
                if (a == '[' && b == ']') return true;
                return  false;
            }

            @Override
            public String optimize(String unoptimized) {
                StringBuilder opt = new StringBuilder();
                for (char c : unoptimized.toCharArray()) {
                    if (!isInSigma(c))
                        continue;
                    if (opt.length() == 0)
                        opt.append(c);
                    else if (isOpposite(opt.charAt(opt.length() - 1), c))
                        opt.deleteCharAt(opt.length() - 1);
                    else opt.append(c);
                }
                return opt.toString();
            }
        };
        return removeInverse.optimize(programRaw);
    }

    public String dump() {
        return memory.dump();
    }

    public String dump(int from, int to) {
        return memory.dump(from, to);
    }


    /**
     * Returns a region of the source code
     * @param from Start
     * @param to End
     * @return instructions in the region
     */
    public char[] getProgramRegion(int from, int to) {
        return copyOfRange(program, Math.max(0, Math.min(from, program.length)), Math.max(0, Math.min(to, program.length)));
    }

    public void setMemoryMode(int memoryMode) {
        this.memory.mode = memoryMode;
    }

    public void setDebugMode(int debugMode) {
        this.debugMode = debugMode;
    }

    public void addDebugMode(int debugMode) { this.debugMode |= debugMode; }

    public void removeDebugMode(int debugMode) { this.debugMode ^= debugMode; }

    /**
     * Enables debug mode on everything
     */
    public void fullDebugMode() {
        setDebugMode(-1);
    }

    public void setIo(InterpreterIO io) {
        this.io = io;
    }

    public char[] getProgram() {
        return program;
    }

    public Memory getMemory() {
        return memory;
    }

    /**
     * Sets the maximum count of operations the program may do
     * A value equal or smaller to zero means no limit
     * @param maxInstructionCount Instruction count limit
     */
    public void setMaxInstructionCount(int maxInstructionCount) {
        this.maxInstructionCount = maxInstructionCount;
    }

    /**
     * @return Number of instructions executed during the last execution
     */
    public long getInsructionCount() {
        return instructionCount;
    }
}