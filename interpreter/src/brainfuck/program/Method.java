package brainfuck.program;

import com.sun.istack.internal.Nullable;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

import static brainfuck.program.Instruction.*;
import static brainfuck.program.Instruction.movePointer;
import static brainfuck.program.Instruction.push;
import static java.util.Collections.singletonList;

/**
 * Method call class
 *
 * Method Call Structure in memory:
 *
 * ------------------------------------------------------------------------------------------------------ Memory ->
 * jump | nextJump | returnAddress | reg[0, 1, 2] | ReturnedValues | params | vars ... (next in stack) ... jump | nextJump ...
 * ------------------------------------------------------------------------------------------------------ Memory ->
 *
 * jump:
 *  Only used by the jump arbiter
 *
 * NextJump:
 *  Address of the label to next jump
 *
 * ReturnAddress:
 *  The return address is set by the calling method to its next label
 *  When a method returns, it writes the
 *
 * Registers:
 *  reg.0: ne
 * Method Call Procedure:
 *
 * Created by Marian on 21/08/2016.
 */
public class Method extends SymbolTable {
    private final String name;
    private final List<String> returnTypes;
    private final List<String> parameterTypes;
    private final List<String> paramNames;
    private final ArrayList<JumpLabel> labels;

    /**
     * Size of the cells each method shares
     */
    public static final int baseSize = 6;

    /**
     * Size of all return registers
     */
    private int returnSize;

    /**
     * Size of all parameters
     */
    private int paramSize;

    public Method(SymbolTable scope, String name, @Nullable List<String> returnTypes, @Nullable List<String> parameterTypes, @Nullable List<String> paramNames) {
        super(scope);
        this.name = name;
        this.returnTypes = returnTypes == null ? new ArrayList<>() : returnTypes;
        this.parameterTypes = parameterTypes == null ? new ArrayList<>() : parameterTypes;
        this.paramNames = paramNames == null ? new ArrayList<>() : paramNames;
        if (parameterTypes.size() < paramNames.size())
            throw new IllegalArgumentException("Each named parameter has to have a type!");
        this.labels = new ArrayList<>();
        labels.add(new JumpLabel(this, getNewLabelName()));
        constructStack("%");
    }

    private void constructStack(String prefix) {
        defineVariable(prefix + "jump");
        defineVariable(prefix + "nextJump");
        defineVariable(prefix + "returnAddress");
        defineVariable(prefix + "reg", "cell", 3);

        returnSize = 0;
        for (int i = 0; i < returnTypes.size(); i++) {
            defineVariable(prefix + "ret" + i, returnTypes.get(i));
            returnSize += getType(Util.splitPath(returnTypes.get(i))).getSizeOnStack();
        }

        paramSize = 0;
        for (int i = 0; i < parameterTypes.size(); i++) {
            defineVariable(paramNames.get(i), parameterTypes.get(i));
            paramSize += getType(Util.splitPath(parameterTypes.get(i))).getSizeOnStack();
        }
    }

    public String getName() {
        return name;
    }

    public JumpLabel getCurrent() {
        return labels.get(labels.size() - 1);
    }

    /**
     * Writes code for the method call procedure
     * @param method Method to jump to
     * @param arguments Name of the variables to give as parameters
     * @param returnVariables List of variables to save the returned values in
     * @return this
     */
    public Method call(String method, List<String> arguments, List<String> returnVariables) {
        Method called = getMethod(method);
        int retSize = called.getReturnRegistersSize();
        int size = getSizeOnStack();

        /**
         * Errorhandling
         */
        if (arguments.size() != called.parameterTypes.size())
            throw new IllegalArgumentException("Parameter count mismatch when calling " + method + ". " +
                    "Expected " + called.getParameterTypes().size() + ", given " + arguments);

        if (returnVariables.size() != called.returnTypes.size())
            throw new IllegalArgumentException("Return value mismatch when calling " + method + ". " +
                    "Expected " + called.getReturnTypes().size() + ", given " + returnVariables.size());

        Util.zip(arguments, called.getParameterTypes()).forEach(
                argPair -> {
                    if (!getVariable(Util.splitPath(argPair.getKey())).getType().getName().equals(argPair.getValue()))
                        throw new IllegalArgumentException("Argument type mismatch when calling " + method + ". " +
                                "Types expected: " + Arrays.toString(called.getParameterTypes().toArray()) + ", " +
                                "give types were: " + Arrays.toString(arguments.stream().map(__a -> getVariable(Util.splitPath(__a)).getType().getName()).toArray()));
                }
        );

        Util.zip(returnVariables, called.getReturnTypes()).forEach(
                retPair -> {
                    if (!getVariable(Util.splitPath(retPair.getKey())).getType().getName().equals(retPair.getValue()))
                        throw new IllegalArgumentException("Return type mismatch when calling " + method + ". " +
                                "Types expected: " + Arrays.toString(called.getParameterTypes().toArray()) + ", " +
                                "give types were: " + Arrays.toString(arguments.stream().map(__b -> getVariable(Util.splitPath(__b)).getType().getName()).toArray()));
                }
        );

        // 1. open new stack for callee
        // 2. set callee.nextJump = method.address
        // 3. set callee.returnAddress = this.current.nextLabel
        // 4. copy parameters into their positions
        // 5. leave with pointer on callee.begin

        // 4. copy parameters

        List<Integer> argumentTypeSums = IntStream.range(0, arguments.size())
                .mapToObj(
                        i -> arguments.stream().limit(i)
                                .map(s -> getVariable(Util.splitPath(s)).getType().getSizeOnStack())
                                .reduce(0, Integer::sum)).collect(Collectors.toList());

        Util.zip(argumentTypeSums, arguments).forEach(kvPair -> {
            int offset = size + baseSize + retSize + kvPair.getKey();
            //System.out.println("push=" + kvPair.getValue() + ", offset=" + offset);
            addInstruction(push(kvPair.getValue(), offset - getVariablePosition(Util.splitPath(kvPair.getValue()))));
        });

        // 1. open new stack
        addInstruction(movePointer(size));

        // 2. set next jump
        addInstruction(
                set("%nextJump", getMethodAddress(method))
        );

        // 3. set return
        addInstruction(
                set("%returnAddress", getNewLabelName())
        );

        // 5. leave & gen new label
        labels.add(new JumpLabel(this, getNewLabelName()));

        // 7. return pointer from callee
        addInstruction(movePointer(-size));

        // 6. move return values from callee
        List<Integer> returnTypeSums = IntStream.range(0, returnVariables.size())
                .mapToObj(
                        i -> returnVariables.stream().limit(i)
                                .map(s -> getVariable(Util.splitPath(s)).getType().getSizeOnStack())
                                .reduce(0, Integer::sum)).collect(Collectors.toList());

        Util.zip(returnTypeSums, returnVariables).forEach(kvPair -> {
            int offset = size + baseSize + kvPair.getKey();
            //System.out.println("pop=" + kvPair.getValue() + ", offset=" + offset);
            addInstruction(pop(kvPair.getValue(), offset - getVariablePosition(Util.splitPath(kvPair.getValue()))));
        });

        // 8. execute this label
        return this;
    }

    /**
     * Moves the specified returnVariables into the return registers
     * @param returnVariables returnVariables to return
     * @return this
     */
    public Method ret(List<String> returnVariables) {
        // 1. set next jump register to return address register
        addInstruction(set("%nextJump", "%returnAddress"));

        if (returnVariables.size() != returnTypes.size())
            throw new IllegalArgumentException("Insufficient return values provided in method " + getName() + ". Expected " + returnTypes.size() + ", given " + returnVariables.size());

        // 2. move return values into the return registers
        Util.zip(IntStream.range(0, returnVariables.size()).mapToObj(i -> "%ret" + i).collect(Collectors.toList()), returnVariables)
            .forEach(pair -> {
                Type returnRegisterType = getVariable(Util.splitPath(pair.getKey())).getType();
                Type returnVariableType = getVariable(Util.splitPath(pair.getValue())).getType();
                if (!returnRegisterType.equals(returnVariableType))
                    throw new IllegalArgumentException("Type mismatch when returning from method " + getName() + "." +
                            " Tried to return a \"" + returnVariableType.getName() + "\" where \"" + returnRegisterType.getName() + "\" was expected");
                addInstruction(move(pair.getKey(), pair.getValue()));
            });
        return this;
    }

    /**
     * Adds instructions to the current label
     * @param instructions Instructions to execute in the label
     * @return this
     */
    public Method addInstructions(List<Instruction> instructions) {
        getCurrent().getInstructions().addAll(instructions);
        return this;
    }

    /**
     * Adds a single instruction to the current label
     * @param instruction
     * @return this
     */
    public Method addInstruction(Instruction instruction) {
        return addInstructions(singletonList(instruction));
    }

    public List<String> getReturnTypes() {
        return returnTypes;
    }

    public List<String> getParameterTypes() {
        return parameterTypes;
    }

    public ArrayList<JumpLabel> getLabels() {
        return labels;
    }

    public int getBaseSize() {
        return baseSize;
    }

    public int getReturnRegistersSize() {
        return returnSize;
    }

    public int getParamSize() {
        return paramSize;
    }
}
