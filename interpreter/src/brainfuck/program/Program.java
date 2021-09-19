package brainfuck.program;

import brainfuck.Interpreter;

import java.io.IOException;
import java.util.ArrayList;

import static brainfuck.program.Instruction.*;
import static brainfuck.program.Instruction.zero;
import static java.util.Arrays.asList;
import static java.util.Collections.singletonList;

/**
 * Created by Marian on 21/08/2016.
 */
public class Program extends SymbolTable {
    public static final char SCOPE_SEPARATOR = '.';
    public static final String NATIVE_TYPE_NAME = "cell";
    private final Method main;

    public Program() {
        super(null);
        defineType("Void");
        defineType("Vec2").defineVariable("x").defineVariable("y");
        main = defineMethod("main", singletonList(NATIVE_TYPE_NAME), singletonList(NATIVE_TYPE_NAME), singletonList("arg"));
    }

    public String compile() {
        class Pair {
            ArrayList<Integer> conditions = new ArrayList<>();
            ArrayList<String> bodies = new ArrayList<>();

            public Pair(){};

            public Pair(JumpLabel loc) {
                conditions.add(loc.getLabelName());
                bodies.add(loc.compile());
            }

            public Pair concat(Pair p) {
                conditions.addAll(p.conditions);
                bodies.addAll(p.bodies);
                return this;
            }

            String compile(SymbolTable symbols) {
                return compileInstructions(symbols, asList(
                        inc("%jump"),
                        whileLoop("%jump", asList(
                                switchStatement("%jump", "%reg", conditions, bodies),
                                log("%jump"),
                                set("%jump", "%nextJump"),
                                zero("%reg")
                                //, log("%jump")
                        )),
                        push("%ret0", -Method.baseSize)
                ));
            }
        }
        return  getMethods().stream()
                .flatMap(method -> method.getLabels().stream())
                .sorted((o1, o2) -> o1.getLabelName() - o2.getLabelName())
                .map(jumpLabel -> new Pair(jumpLabel))
                .reduce(new Pair(), Pair::concat).compile(this.main);
    }

    public static void main(String argv[]) throws IOException {
        Interpreter interpreter = new Interpreter(1, 1);
        interpreter.getMemory().memoryLimit = 0x40;
        interpreter.setMaxInstructionCount((int) 1e6);
        interpreter.fullDebugMode();
        Program p = new Program();

        Method sum = p.defineMethod("sum", singletonList("cell"), singletonList("cell"), singletonList("n"));
        sum.defineVariable("sum", "cell", 2);
        sum.addInstructions(asList(
                //log("n"),
                forEach("n", asList(
                        add("sum", "n", "sum.1")
                ))
        )).ret(asList("sum.0"));

        Method dot = p.defineMethod("dot", singletonList("cell"), asList("Vec2", "Vec2"), asList("a", "b"));
        dot.defineVariable("t", "cell", 2);
        dot.addInstructions(asList(
                mult("a.x", "b.y", "t.0", "t.1"),
                mult("a.y", "b.x", "t.0", "t.1"),
                add("a.x", "a.y")
        ))
                .call("sum", singletonList("a.x"), singletonList("a.x"))
                .ret(singletonList("a.x"));

        p.main.defineVariable("v", "Vec2", 2);
        p.main.addInstructions(asList(
//                read("v.0.x"),
//                read("v.0.y"),
//                read("v.1.x"),
//                read("v.1.y")
                set("v", new int[]{4,5,6,7})
        ))
//                .call("sum", singletonList("arg"), singletonList("arg"))
//                .addInstruction(log("arg"))
                .call("dot", asList("v.0", "v.1"), singletonList("arg"))
                .ret(singletonList("arg"));

        String compiled = p.compile();
        interpreter.loadProgramIntoMemory(compiled, true);
        System.out.println(interpreter.getProgram());
        interpreter.run();
        System.out.println(interpreter.dump());
        System.out.println("InstrCount=" + interpreter.getInsructionCount());
    }
}
