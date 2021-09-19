package brainfuck.program;

import java.util.ArrayList;
import java.util.List;

import static brainfuck.program.Instruction.*;
import static brainfuck.program.Instruction.move;
import static java.util.Arrays.asList;

/**
 * A class that represents a jumpable address
 * Created by Marian on 14/10/2016.
 */
public class JumpLabel {
    private final List<Instruction> instructions;
    private final Method method;
    private int labelName;

    public JumpLabel(Method method, int labelName) {
        this.method = method;
        this.labelName = labelName;
        this.instructions = new ArrayList<>();
        instructions.add(comment("%reg", method.getName() + "@" + labelName));
    }

    public void jump(int target) {
        int ret = method.getNewLabelName();
        method.getLabels().add(new JumpLabel(method, ret));
        jump(target, ret);
    }

    public void jump(int target, int ret) {
        method.addInstruction(set("%nextJump", target));
        method.addInstruction(set("%returnAddress", ret));
    }

    public void jumpzero(int target, String condition) {
        jump(target);
        // TODO: Think about conditional jump
        method.addInstruction(whileLoop(condition, asList(
                zero(condition),
                set("%nextJump", 0)
        )));
    }

    public Method getMethod() {
        return method;
    }

    public List<Instruction> getInstructions() {
        return instructions;
    }

    public int getLabelName() {
        return labelName;
    }

    public String compile() {
        return Instruction.compileInstructions(method, instructions);
    }
}
