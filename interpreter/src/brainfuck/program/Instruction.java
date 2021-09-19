package brainfuck.program;

import java.util.*;
import java.util.stream.IntStream;

import static brainfuck.program.Util.repeat;
import static java.util.Arrays.asList;
import static java.util.Collections.singletonList;

/**
 * An abstraction for bytecode
 * Created by Marian on 21/08/2016.
 */
public interface Instruction {

    /**
     * Compiles a list of instructions
     * @param symbols Symbols to compile with
     * @param instructions List of instructions
     * @return BF bytecode
     */
    static String compileInstructions(SymbolTable symbols, List<Instruction> instructions) {
        StringBuilder stringBuilder = new StringBuilder();
        instructions.forEach(instruction -> stringBuilder.append(instruction.compile(symbols)));
        return stringBuilder.toString();
    }

    /***
     * Writes a comment inside a while loop into the bytecode
     * The zero cell should be zero in order to not execute the comment
     * @param zero A cell that is zero before executing this instruction
     * @param comment Comment to write
     * @return instruction
     */
    static Instruction comment(String zero, String comment) {
        return symbols -> whileLoop(zero, comment).compile(symbols);
    }

    /**
     * directly writes code
     * @param code BF bytecode
     * @return instruction
     */
    static Instruction directWrite(String code) {
        return symbols -> code;
    }

    /**
     * Writes an array of characters at the destination
     * @param dst Location of the string
     * @param string String to write
     * @return instruction
     */
    static Instruction writeString(String dst, String string, int length) {
        return symbols -> {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < length; i++)
                sb.append(movePointer(dst, repeat('>', i) + repeat('+', string.charAt(i)) + repeat('<', i)).compile(symbols));
            return sb.toString();
        };
    }

    /**
     * Use with caution!
     * Moves pointer to a variable, but not back
     * @param variable Variable to move to
     * @param invert Inverts the offset sign
     * @return instruction
     */
    static Instruction movePointer(String variable, boolean invert) {
        return symbols -> {
            int pos = symbols.getVariablePosition(Util.splitPath(variable));
            return repeat((pos < 0 != invert) ? '<' : '>', pos);
        };
    }

    /**
     * Use with caution!
     * Moves the pointer by a constant offset, but not back
     * @param offset offset to move pointer by
     * @return instruction
     */
    static Instruction movePointer(int offset) {
        return symbols -> repeat(offset < 0 ? '<' : '>', offset);
    }

    /**
     * Moves a pointer by the offset and executes operations
     * Returns to its original position afterwards
     * @param offset Offset
     * @param operations Executeable
     * @return instruction
     */
    static Instruction movePointer(int offset, String operations) {
        return symbols -> compileInstructions(symbols, asList(
                movePointer(offset),
                directWrite(operations),
                movePointer(-offset)));
    }

    /**
     * Moves the pointer by an offset, executes a list of instructions, then returns to its original position
     * @param offset Offset to execute instructions at
     * @param instructions Executed instructions
     * @return instruction
     */
    static Instruction movePointer(int offset, List<Instruction> instructions) {
        return symbols -> movePointer(offset, compileInstructions(symbols, instructions)).compile(symbols);
    }

    /**
     * Moves the pointer to a variable and returns to its original position after executing operations
     * @param targetVariable Variable to move pointer to
     * @param operations Operations to do at the variables positions
     * @return MovePointer instruction
     */
    static Instruction movePointer(String targetVariable, String operations) {
        return symbols ->  movePointer(symbols.getVariablePosition(Util.splitPath(targetVariable)), operations).compile(symbols);
    }

    /**
     * Writes the address of variable into dst
     * @param dst pointer
     * @param variable Variable to get address of
     * @return instruction
     */
    static Instruction getReference(String dst, String variable) {
        return symbols -> set(dst, symbols.getVariablePosition(Util.splitPath(variable))).compile(symbols);
    }

    /**
     * Writes the size of a variable
     * @param dst Cell to write into
     * @param variable Variable to get size of
     * @return instruction
     */
    static Instruction getSizeOf(String dst, String variable) {
        return symbols -> set(dst, symbols.getVariable(Util.splitPath(variable)).getType().getSizeOnStack()).compile(symbols);
    }

    /**
     * Executes a list of instuctions with an offset for each cell in a struct
     * @param struct Struct to execute the instructions for
     * @param instructions Executed instructions
     * @return instruction
     */
    static Instruction forEachCell(String struct, List<Instruction> instructions) {
        return forEachCell(struct, new ArrayList<>(), instructions, new ArrayList<>());
    }

    /**
     * Executes instructions at each cell
     * The pre instructions are executed for each cell before the offset is applied
     * The post instructions are executed after the offset is reversed and before the next cells' pre instructions are called
     * @param struct Struct to execute instructions for each cell
     * @param pre Instructions that are executed before the offset is applied
     * @param instructions Instructions that are executed while the offset is applied
     * @param post Instructions that are executed after the offset is reversed
     * @return instruction
     */
    static Instruction forEachCell(String struct, List<Instruction> pre, List<Instruction> instructions, List<Instruction> post) {
        return symbols ->
                forEach(symbols.getVariable(Util.splitPath(struct)).getType().getSizeOnStack(), pre, instructions, post).compile(symbols);
    }

    /**
     * Executes instructions n times with and without an offset of range(0..n-1)
     * @param n Recursions
     * @param instructions Executed while offset applied
     * @return instruction
     */
    static Instruction forEach(int n, List<Instruction> instructions) {
        return forEach(n, new ArrayList<>(), instructions, new ArrayList<>());
    }

    /**
     * Executes instructions n times with and without an offset of range(0..n-1)
     * @param n Recursions
     * @param pre Executed before offset applied
     * @param instructions Executed while offset applied
     * @param post Executed after offset removed
     * @return instruction
     */
    static Instruction forEach(int n, List<Instruction> pre, List<Instruction> instructions, List<Instruction> post) {
        return symbols ->
                IntStream.range(0, n)
                        .mapToObj(i ->  compileInstructions(symbols, pre) +
                                        movePointer(i).compile(symbols) +
                                        compileInstructions(symbols, instructions) +
                                        movePointer(-i).compile(symbols) +
                                        compileInstructions(symbols, post))
                        .reduce("", (s, s2) -> s + s2);
    }

    /**
     * Loops over a predicate, executing the body and decreasing the predicate each loop by 1
     * predicate is set to 0
     * @param predicate Decreasing predicate
     * @param body Body to execute
     * @return instruction
     */
    static Instruction forEach(String predicate, String body) {
        return symbols -> compileInstructions(symbols, asList(
                whileLoop(predicate, body + dec(predicate).compile(symbols))
        ));
    }

    /**
     * Loops over the predicate and restores it afterwards
     * predicate is preserved
     * aux is set to 0
     * @param predicate  Predicate to loop over
     * @param body Loop body
     * @param aux Helper
     * @return instruction
     */
    static Instruction forEach(String predicate, String body, String aux) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux, predicate),
                forEach(aux, body + inc(predicate).compile(symbols))
        ));
    }

    /**
     * Loops over a predicate
     * Executes the bodies instructions each loop, then decreasing the predicate
     * predicate is not preserved
     * @param predicate Predicate to decrease
     * @param body Body to execute
     * @return instruction
     */
    static Instruction forEach(String predicate, List<Instruction> body) {
        return symbols -> forEach(predicate, compileInstructions(symbols, body)).compile(symbols);
    }

    /**
     * Loops over a predicate
     * Executes the bodies instructions each loop, then decreasing the predicate
     * predicate is preserved
     * aux is set to 0
     * @param predicate Predicate to decrease
     * @param body Body to execute
     * @aux Helper
     * @return instruction
     */
    static Instruction forEach(String predicate, List<Instruction> body, String aux) {
        return symbols -> forEach(predicate, compileInstructions(symbols, body), aux).compile(symbols);
    }

    /**
     * Tests the while loop with the predicate variable but returns to its initial position inside the loop
     * @param predicateVariable Predicate for the while loop
     * @param whileBody BF bytecode inside the loop
     * @return While loop instruction
     */
    static Instruction whileLoop(String predicateVariable, String whileBody) {
        return symbols ->
            movePointer(predicateVariable, "[").compile(symbols)
            + whileBody
            + movePointer(predicateVariable, "]").compile(symbols);
    }

    /**
     * Tests the while loop with the predicate variable but returns to its initial position inside the loop
     * @param predicateVariable Predicate for the while loop
     * @param whileBody List of instructions
     * @return While loop instruction
     */
    static Instruction whileLoop(String predicateVariable, List<Instruction> whileBody) {
        return symbols -> movePointer(predicateVariable, "[").compile(symbols)
                + compileInstructions(symbols, whileBody)
                + movePointer(predicateVariable, "]").compile(symbols);
    }

    /**
     * Executes the body if the predicate is not zero
     * The predicate is set to zero
     * @param predicate Predicate variable
     * @param body Instructions to execute if predicate is not zero
     * @return instruction
     */
    static Instruction ifClause(String predicate, List<Instruction> body) {
        return symbols -> whileLoop(predicate, zero(predicate).compile(symbols) +
                compileInstructions(symbols, body)).compile(symbols);
    }

    /**
     * Executes ifTrue if predicate is not zero, otherwise 'otherwise' is executed
     * the predicate is set to zero
     * aux is set to zero
     * @param predicate Predicate variable
     * @param ifTrue Instructions if true
     * @param otherwise Instructions otherwise
     * @return instruction
     */
    static Instruction ifElse(String predicate, String aux, List<Instruction> ifTrue, List<Instruction> otherwise) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux, predicate), // aux = pred
                inc(predicate), // p = 1
                whileLoop(aux, asList(zero(aux), dec(predicate))), // if (pred) p = 0
                inc(aux), // aux = 1
                whileLoop(predicate, dec(predicate).compile(symbols) + dec(aux).compile(symbols) + compileInstructions(symbols, otherwise)),
                whileLoop(aux, dec(aux).compile(symbols) + compileInstructions(symbols, ifTrue))
        ));
    }

    /**
     * Implements a switch statement with constant conditions in ascending order
     * switchVar and flag variable are set to 0
     * @param switchVar Cell to switch on
     * @param flag Helper cell
     * @param conditions Conditions to each case
     * @param cases Code to execute in each body
     *                   There can be one more body than cases, which is then used as the default case
     * @return instruction
     */
    static Instruction switchStatement(String switchVar, String flag, List<Integer> conditions, List<String> cases) {
        conditions.sort(Integer::compareTo);
        /*
            flag = 1
            stmt -= deltaCost
            stmt[
                stmt -= deltaCost
                stmt[
                    stmt -= deltaCost
                    stmt[
                        defaultCase; flag = 0; stmt = 0;
                    ]
                    flag[thirdCase; flag = 0; stmt = 0]
                ]
                flag[second case; flag = 0; stmt = 0]
            ]
            flag[first case; stmt = 0; stmt = 0]
         */
        if (cases.size() - conditions.size() != 0 && cases.size() - conditions.size() != 1)
            throw new IllegalArgumentException("There may be a maximum of conditions + 1 cases!");
        class Recurse {
            String c(SymbolTable s, int depth, int cost) {
                String cstr = "";
                if (depth < conditions.size()) {
                    cstr += Instruction.sub(switchVar, conditions.get(depth) - cost).compile(s);
                    cstr += Instruction.whileLoop(switchVar,
                            c(s, depth + 1, conditions.get(depth))).compile(s);
                }
                if (depth < cases.size()) {
                    cstr += Instruction.whileLoop(flag,
                            Instruction.dec(flag).compile(s) +
                            cases.get(depth)
                    ).compile(s);
                } else {
                    // no default case
                    cstr += Instruction.dec(flag).compile(s);
                }
                return cstr + Instruction.zero(switchVar).compile(s);
            }
        }
        return symbols -> String.format("%s%s",
                one(flag).compile(symbols),
                new Recurse().c(symbols, 0, 0));
    }

    /**
     * Sets the value at the address to 0
     * @param dst Variable to set
     * @return instruction
     */
    static Instruction zero(String dst) {
        return symbols -> movePointer(dst, "[-]").compile(symbols);
    }

    /**
     * Sets the value at the address to 1
     * @param dst Variable to set
     * @return instruction
     */
    static Instruction one(String dst) {
        return symbols -> compileInstructions(symbols, asList(zero(dst), inc(dst)));
    }

    /**
     * Sets the value at the address to 0
     * @param offset cell to set
     * @return instruction
     */
    static Instruction zero(int offset) {
        return symbols -> movePointer(offset, "[-]").compile(symbols);
    }

    /**
     * Sets the value at the address to 1
     * @param offset cell to set
     * @return instruction
     */
    static Instruction one(int offset) {
        return symbols -> compileInstructions(symbols, asList(zero(offset), inc(offset)));
    }

    /**
     * Increases the value at the address
     * @param dst Variable to increase
     * @return instruction
     */
    static Instruction inc(String dst) {
        return add(dst, 1);
    }

    /**
     * Adds 1 to the cell at the offset
     * @param offset Offset of the cell
     * @return instruction
     */
    static Instruction inc(int offset) { return add(offset, 1); }

    /**
     * Adds a number to the cell at the offset
     * @param offset Offset of the cell
     * @return instruction
     */
    static Instruction add(int offset, int value) { return symbols -> movePointer(offset, repeat(value < 0 ? '-' : '+', value)).compile(symbols); }

    /**
     * Adds a constant at the address of the cell
     * @param dst Variable to add / subtract to / from
     * @param value Constant
     * @return instruction
     */
    static Instruction add(String dst, int value) {
        return symbols -> movePointer(dst, repeat(value < 0 ? '-' : '+', value)).compile(symbols);
    }

    /**
     * Adds the value of src to the value and src
     * The value of src is 0 after this instruction
     * @param dst Variable to save the result into
     * @param src Variable to add to the value of dst
     * @return instruction
     */
    static Instruction add(String dst, String src) {
        return symbols -> whileLoop(src, asList(dec(src), inc(dst))).compile(symbols);
    }

    /**
     * computes:
     * dst += src
     * src = src
     * aux = 0
     * @param dst Target location
     * @param src Source value
     * @param aux Auxiliary
     * @return instruction
     */
    static Instruction add(String dst, String src, String aux) {
        return symbols -> compileInstructions(symbols,
                asList(zero(aux), whileLoop(src, asList(dec(src), inc(dst), inc(aux))),
                        set(src, aux)));
    }

    /**
     * Adds the value of a source to all destinations
     * src is set to 0
     * @param dsts Addition destinations
     * @param src Source value
     * @return instruction
     */
    static Instruction add(List<String> dsts, String src) {
        return symbols -> forEach(src, dsts.stream().reduce("",
                (acc, dst) -> acc + inc(dst).compile(symbols))).compile(symbols);
    }

    /**
     * Adds the value of a source to all destinations
     * src is preserved
     * aux is set to 0
     * @param dsts Addition destinations
     * @param src Source value
     * @param aux Helper
     * @return instruction
     */
    static Instruction add(List<String> dsts, String src, String aux) {
        return symbols -> forEach(src, dsts.stream().reduce("",
                (acc, dst) -> acc + inc(dst).compile(symbols)), aux).compile(symbols);
    }

    /**
     * Decreases the value of a cell by 1
     * @param dst Variable to decrease
     * @return instruction
     */
    static Instruction dec(String dst) {
        return sub(dst, 1);
    }

    /**
     * Decreases the cell at the offset
     * @param offset
     * @return instruction
     */
    static Instruction dec(int offset) { return symbols -> add(offset, -1).compile(symbols); }

    /**
     * Subtracts a constant off a cell
     * @param dst Variable to subtract from
     * @param value Constant to subtract
     * @return instruction
     */
    static Instruction sub(String dst, int value) {
        return add(dst, -value);
    }

    /**
     * Subtracts a constant off a cell
     * @param offset Cell to subtract at
     * @param value Constant to subtract
     * @return instruction
     */
    static Instruction sub(int offset, int value) {
        return add(offset, -value);
    }


    /**
     * Subtracts src from dst
     * computes:
     * dst -= src
     * src = 0
     * @param dst
     * @param src
     * @return
     */
    static Instruction sub(String dst, String src) {
        return sub(asList(dst), src);
    }

    /**
     * Subtracts a value from a list of destinations
     * for each dst in dsts: dst -= src
     * src = 0
     * @param dsts Destinations
     * @param src Source
     * @return instruction
     */
    static Instruction sub(List<String> dsts, String src) {
        return symbols -> forEach(src, dsts.stream().reduce("",
                (acc, dst) -> acc + dec(dst).compile(symbols))).compile(symbols);
    }

    /**
     * Subtract src from dst
     * computes:
     * dst -= src
     * src = src
     * aux = 0
     * @param dst
     * @param src
     * @param aux
     * @return
     */
    static Instruction sub(String dst, String src, String aux) {
        return symbols -> compileInstructions(symbols,
                asList(zero(aux),
                        whileLoop(src, asList(dec(src), dec(dst), inc(aux))),
                        set(src, aux)));
    }

    /**
     * Subtracts a value from a list of destinations
     * src is preserved
     * aux = 0
     * @param dsts Destinations
     * @param src Source
     * @param aux Helper
     * @return instruction
     */
    static Instruction sub(List<String> dsts, String src, String aux) {
        return symbols -> forEach(src, dsts.stream().reduce("",
                (acc, dst) -> acc + dec(dst).compile(symbols)), aux).compile(symbols);
    }

    /**
     * Sets the value of a variable
     * dst = value
     * @param dst Variable to set
     * @param value Value to set the variable to
     * @return instruction
     */
    static Instruction set(String dst, int value) {
        return symbols -> compileInstructions(symbols, asList(zero(dst), add(dst, value)));
    }

    /**
     * Sets the value of the cell at the offset
     * @param offset Offset of the cell
     * @param value Value to set
     * @return instruction
     */
    static Instruction set(int offset, int value) {
        return symbols -> movePointer(offset, "[-]" + repeat(value < 0 ? '-' : '+', value)).compile(symbols);
    }

    /**
     * Sets cells adjacent to dst
     * @param dst Anchor
     * @param values Values set to the left
     * @return instruction
     */
    static Instruction set(String dst, int[] values) {
        return symbols -> IntStream.range(0, values.length)
                .mapToObj(i -> asList(
                        movePointer(i),
                        movePointer(dst, "[-]" + repeat(values[i] < 0 ? '-' : '+', values[i])),
                        movePointer(-i)))
                .map(instructions ->
                        compileInstructions(symbols, instructions)).reduce("", String::concat);
    }

    /**
     * Sets the value of dst to the value of src
     * computes:
     * dst = src
     * src = 0
     * @param dst Variable to override
     * @param src Value to set to
     * @return instruction
     */
    static Instruction set(String dst, String src) {
        return set(asList(dst), src);
    }

    /**
     * Moves a value into each dst
     * src is set to 0
     * @param dsts List of destinations
     * @param src Source value
     * @return instruction
     */
    static Instruction set(List<String> dsts, String src) {
        return symbols -> compileInstructions(symbols, asList(
                directWrite(dsts.stream().reduce("", (acc, s) -> acc + zero(s).compile(symbols))),
                whileLoop(src, asList(
                        dec(src),
                        directWrite(dsts.stream().reduce("", (acc, s) -> acc + inc(s).compile(symbols)))
                ))
        ));
    }

    /**
     * Moves a value into each dst
     * src is preserved
     * aux is set to 0
     * @param dsts List of destinations
     * @param src Source value
     * @param aux Helper
     * @return instruction
     */
    static Instruction set(List<String> dsts, String src, String aux) {
        return symbols ->
                dsts.stream().reduce("",
                        (acc, dst) -> acc + zero(dst).compile(symbols)) +
                                forEach(src, dsts.stream().reduce("",
                (acc2, dst2) -> acc2 + inc(dst2).compile(symbols)), aux).compile(symbols);
    }

    /**
     * Copies the value of src to dst
     * computes:
     * dst = src
     * src = src
     * aux = 0
     * @param dst Destination address
     * @param src Source address
     * @param aux Helper
     * @return instruction
     */
    static Instruction set(String dst, String src, String aux) {
        return set(asList(dst), src, aux);
    }

    /**
     * Multiplies dst with scale and saves the result in dst
     * aux is set to 0
     * @param dst Factor 1
     * @param scale Scale
     * @param aux Helper
     * @return instruction
     */
    static Instruction mult(String dst, int scale, String aux) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux, dst),
                whileLoop(aux, asList(
                    dec(aux),
                    movePointer(dst, directWrite(repeat(scale < 0 ? '-' : '+', scale)).compile(symbols))
                ))
        ));
    }

    /**
     * Multiplies two variables
     * aux0 and aux1 are not preserved
     * dst = dst * src
     * src = src
     * aux0 = 0
     * aux1 = 0
     * @param dst Calculation result; factor 1
     * @param src Factor 2
     * @param aux0 Helper
     * @param aux1 Helper
     * @return instruction
     */
    static Instruction mult(String dst, String src, String aux0, String aux1) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux0, dst),
                zero(aux1),
                whileLoop(aux0, asList(
                        dec(aux0),
                        whileLoop(src, asList(
                            dec(src),
                            inc(dst),
                            inc(aux1)
                        )),
                        whileLoop(aux1, asList(
                            dec(aux1),
                            inc(src)
                        ))
                ))
        ));
    }

    /**
     * Raises base by power of exp saving the result in base
     * exp is set to 0
     * t1 and t2 are set to zero if exponent is not 0
     * t0 is set to base
     * @param base Base; Destination
     * @param exp Power
     * @param t0 Helper
     * @param t1 Helper
     * @param t2 Helper
     * @return instruction
     */
    static Instruction pow(String base, String exp, String t0, String t1, String t2) {
        return symbols -> compileInstructions(symbols, asList(
                set(t0, base),
                inc(base),
                whileLoop(exp, asList(
                        zero(t1),
                        set(t2, base),
                        whileLoop(t2, asList(
                                whileLoop(t0, asList(
                                        inc(base),
                                        inc(t1),
                                        dec(t0)
                                )),
                                whileLoop(t1, asList(
                                        inc(t0),
                                        dec(t1)
                                )),
                                dec(t2)
                        )),
                        dec(exp)
                ))
        ));
    }

    /**
     * Swaps cells a and b
     * aux set to 0
     * @param a A cell
     * @param b Other cell
     * @param aux Helper
     * @return instruction
     */
    static Instruction swap(String a, String b, String aux) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux, a),
                whileLoop(b, asList(dec(b), inc(a))),
                set(b, aux)
        ));
    }

    /**
     * Moves all cells from src to dst
     * all cells of src are set to 0
     * @param dst Destination struct
     * @param src Source struct
     * @return instruction
     */
    static Instruction move(String dst, String src) {
        return symbols -> {
            Type dstType = symbols.getVariable(Util.splitPath(dst)).getType();
            Type srcType = symbols.getVariable(Util.splitPath(src)).getType();

            int dstSize = dstType.getSizeOnStack();
            int srcSize = srcType.getSizeOnStack();

            if (dstSize != srcSize)
                System.out.println("Warning: Moving from type " + srcType.toString() + " to " + dstType.toString());

            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < dstSize && i < srcSize; i++) {
                sb.append(movePointer(i).compile(symbols));
                sb.append(zero(dst).compile(symbols));
                sb.append(whileLoop(src, asList(dec(src), inc(dst))).compile(symbols));
                sb.append(movePointer(-i).compile(symbols));
            }
            return sb.toString();
        };
    }

    /**
     * Moves all cells of src to dst
     * The helper variable only needs to be size 1
     * src is preserved
     * aux is set to 0
     * @param dst Destination struct
     * @param src Source struct
     * @param aux Helper cell
     * @return instruction
     */
    static Instruction move(String dst, String src, String aux) {
        return symbols -> {
            int dstSize = symbols.getVariable(Util.splitPath(dst)).getType().getSizeOnStack();
            int srcSize = symbols.getVariable(Util.splitPath(src)).getType().getSizeOnStack();

            if (dstSize != srcSize)
                System.out.println("Warning: Moving value of size " + srcSize + " struct into size " + dstSize + " struct");

            StringBuilder stringBuilder = new StringBuilder();
            for (int i = 0; i < dstSize && i < srcSize; i++) {
                stringBuilder.append(zero(aux).compile(symbols));
                stringBuilder.append(movePointer(i).compile(symbols));
                stringBuilder.append(zero(dst).compile(symbols));
                stringBuilder.append(whileLoop(src, asList(
                    dec(src),
                    inc(dst),
                    movePointer(-i, inc(aux).compile(symbols))
                )).compile(symbols));
                stringBuilder.append(movePointer(-i).compile(symbols));
                stringBuilder.append(whileLoop(aux, asList(
                    dec(aux), movePointer(i, inc(src).compile(symbols))
                )).compile(symbols));
            }
            return stringBuilder.toString();
        };
    }

    /**
     * Moves a lists of offseted srouce structures into the not offseted destination structure variables
     * The sources and destinations viewed pairwise
     * @param offset Offset of the sources
     * @param dsts Destination variables
     * @param srcs Source variables
     * @return instruction
     */
    static Instruction move(int offset, List<String> dsts, List<String> srcs) {
        return symbols -> compileInstructions(symbols, asList(
                symbols2 -> dsts.stream().reduce("", (acc, dst) -> acc + forEachCell(dst, singletonList(zero(dst))).compile(symbols2)),
                movePointer(offset),
                symbols1 -> Util.zip(srcs == null ? dsts : srcs, dsts).stream()
                        .map(pair -> forEachCell(pair.getValue(), asList(
                                    whileLoop(pair.getKey(), asList(
                                            dec(pair.getKey()),
                                            movePointer(-offset),
                                            inc(pair.getValue()),
                                            movePointer(offset)
                                    ))
                                )).compile(symbols)
                        ).reduce("", String::concat),
                movePointer(-offset)
        ));
    }

    /**
     * Moves all cells of a struct by an offset
     * @param offset Offset
     * @param struct Struct to move
     * @return instruction
     */
    static Instruction move(int offset, String struct) {
        return symbols -> forEachCell(struct, asList(
                zero(offset),
                whileLoop(struct, asList(
                        dec(struct),
                        inc(offset)
                ))
        )).compile(symbols);
    }

    /**
     * Pulls all cells of a structure offsetted by sourceOffset from dst into dst
     * All cells at the srcOffset are set to 0
     * @param dst Destination of pulling
     * @param srcOffset Offset to the souce structure relative to the compiling symbol table
     * @return instruction
     */
    static Instruction pop(String dst, int srcOffset) {
        return symbols -> forEachCell(dst, asList(
                zero(dst),
                movePointer(srcOffset, asList(
                        whileLoop(dst, asList(
                                dec(dst),
                                movePointer(-srcOffset, singletonList(inc(dst))))
                        )
                ))
        )).compile(symbols);
    }

    /**
     * Pushes all cells of a structure by some offset
     * All cells of src are set to 0
     * @param src Structure to push
     * @param dstOffset Offset to the destination structure relative to the compiling symbol table
     * @return instruction
     */
    static Instruction push(String src, int dstOffset) {
        return symbols -> forEachCell(src, asList(
                    movePointer(dstOffset, singletonList(zero(src))),
                    whileLoop(src, asList(
                            dec(src),
                            movePointer(dstOffset, singletonList(inc(src)))
                    ))
        )).compile(symbols);
    }

    /**
     * Pushes a source along an offset
     * Src is preserved
     * Cells of aux are set to 0
     * @param src Source structure
     * @param dstOffset Offset of the target
     * @param aux Memory location with the same size of src
     * @return instruction
     */
    static Instruction push(String src, int dstOffset, String aux) {
        return symbols -> forEachCell(src, asList(
                movePointer(dstOffset, singletonList(zero(src))),
                set(aux, src),
                whileLoop(aux, asList(
                        dec(aux),
                        inc(src),
                        movePointer(dstOffset, singletonList(inc(src)))
                ))
        )).compile(symbols);
    }

    /**
     * Tests if dst is equal to value
     * dst = dst == value
     * aux = 0
     *
     * dst is 1 if true 0 if false
     * aux is set to 0
     * @param dst Target
     * @param value Test value
     * @param aux Auxiliary
     * @return instruction
     */
    static Instruction equal(String dst, int value, String aux) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux, dst),
                inc(dst),
                sub(aux, value),
                whileLoop(aux, asList(zero(aux), dec(dst)))));
    }

    /**
     * Tests if dst and src are equal and saves the result in dst
     * aux and src are not preserved
     *
     * dst = dst == src
     * src = 0
     * aux = 0
     * @param dst Left hand side and destination for the result of the comparison
     * @param src Right hand side of the comparision
     * @param aux Helper variable
     * @return instruction
     */
    static Instruction equal(String dst, String src, String aux) {
        return symbols -> compileInstructions(symbols, asList(
                set(aux, dst),
                inc(dst),
                sub(src, aux),
                whileLoop(src, asList(zero(src), dec(dst)))));
    }

    /**
     * Sets dst to 1 if it is zero, sets it to zero otherwise
     * The 2 cells right to dst are zeroed in this process
     * @param dst Tests if the variable is zero
     * @return instruction
     */
    static Instruction isZero(String dst) {
        return symbols -> movePointer(dst, ">[-]+>[-]<<[>-]>[>]<[<+>-]<").compile(symbols);
    }

    /**
     * Executes the isntructions if predicate is not zero
     * Predicate is set to 0
     * @param predicate
     * @param instructions
     * @return instruction
     */
    static Instruction doIfNotZero(String predicate, List<Instruction> instructions) {
        return symbols -> doIfNotZero(predicate, compileInstructions(symbols, instructions)).compile(symbols);
    }

    /**
     * Executes the isntructions if predicate is not zero
     * Predicate is set to 0
     * @param predicate
     * @param instructions
     * @return instruction
     */
    static Instruction doIfNotZero(String predicate, String instructions) {
        return symbols -> compileInstructions(symbols, singletonList(
                whileLoop(predicate, zero(predicate).compile(symbols) + instructions)
        ));
    }

    /**
     * Prints the value of a cell to stdout
     * @param variable Variable to print
     * @return instruction
     */
    static Instruction print(String variable) {
        return symbols -> movePointer(variable, ".").compile(symbols);
    }

    /**
     * Reads one cell from stdin and writes it into the variable
     * @param variable Variable to override with input
     * @return instruction
     */
    static Instruction read(String variable) {
        return symbols -> movePointer(variable, ",").compile(symbols);
    }

    /**
     * Sample cat program
     * @param var Variable to use as cel
     * @return cat program
     */
    static Instruction cat(String var) {
        return symbols -> compileInstructions(symbols, asList(
                    read(var),
                    whileLoop(var, asList(
                            print(var),
                            read(var)
                    ))
            ));
    }

    /**
     * Writes a log character into the sourcecode
     * @return instruction
     */
    static Instruction log() {
        return symbols -> directWrite("#").compile(symbols);
    }

    /**
     * Moves the pointer to the variable and logs
     * @param variable
     * @return
     */
    static Instruction log(String variable) {
        return symbols -> movePointer(variable, directWrite("#").compile(symbols)).compile(symbols);
    }

    /**
     * Compiles the instruction into bf bytecode
     * @param symbols The symbols the Instruction can access
     * @return BF bytecode
     */
    String compile(SymbolTable symbols);
}
