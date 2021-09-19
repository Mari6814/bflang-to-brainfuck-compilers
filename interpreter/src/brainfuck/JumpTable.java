package brainfuck;

/**
 * A class that records from where to where to jump
 * Created by Marian on 20/08/2016
 */
public class JumpTable {
    private int table[];

    public JumpTable(char program[]) {
        this.table = new int[program.length];

        for (int i = 0, depth = 0, pc; i < program.length; i++) {
            switch (program[i]) {
                case '[':
                    pc = i;
                    do {
                        if (program[pc] == '[')
                            depth++;
                        else if (program[pc] == ']')
                            depth--;
                        ++pc;
                    } while (depth != 0);
                    while (pc < program.length && program[pc] == ']') ++pc;
                    table[i] = pc >= program.length ? program.length - 1 : pc;
                    break;
                case ']':
                    pc = i;
                    do {
                        if (program[pc] == '[')
                            depth++;
                        else if (program[pc] == ']')
                            depth--;
                        --pc;
                    } while (depth != 0);
                    pc++;
                    while (program[pc] == '[') ++pc;
                    table[i] = pc;
                    break;
                default:
                    table[i] = i;
            }
        }
    }

    public int jump(int from) {
        return table[from];
    }
}