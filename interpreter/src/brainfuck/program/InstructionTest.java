package brainfuck.program;

import brainfuck.*;
import junit.framework.TestCase;

import java.util.List;

import static brainfuck.program.Instruction.*;
import static brainfuck.program.Instruction.sub;
import static brainfuck.program.Instruction.zero;
import static java.util.Arrays.asList;
import static java.util.Arrays.sort;
import static java.util.Collections.singletonList;
import static org.junit.Assert.*;

/**
 * Instruction testcases
 * Created by marian on 12/10/16.
 */
public class InstructionTest extends TestCase {

    public void testZero1() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                zero(2)
        ), new long[] {1, 4, 0, 2, 5, 8, 3, 6, 9});
    }

    public void testOne1() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                one(2)
        ), new long[] {1, 4, 1, 2, 5, 8, 3, 6, 9});
    }

    public void testSub5() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                sub(2, 5)
        ), new long[] {1, 4, 2, 2, 5, 8, 3, 6, 9});
    }

    public void testAdd5() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                add(2, 5)
        ), new long[] {1, 4, 12, 2, 5, 8, 3, 6, 9});
    }

    public void testSet5() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                set(2, 2)
        ), new long[] {1, 4, 2, 2, 5, 8, 3, 6, 9});
    }

    public void testPop() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                pop("a", 3)
        ), new long[]{2, 5, 8, 0, 0, 0, 3, 6, 9});
    }

    public void testPush() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                push("a", 3)
        ), new long[]{0 ,0, 0, 1, 4, 7, 3, 6, 9});
    }

    public void testPush2() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                push("a", 3, "c")
        ), new long[]{1, 4, 7, 1, 4, 7, 0, 0, 0});
    }

    public void testDoIfNotZero() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                doIfNotZero("a", singletonList(set("b", 99)))
        ), new long[]{0, 4, 7, 99, 5, 8, 3, 6, 9});

        test(3, 3, asList(
                set("a", new int[]{0, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                doIfNotZero("a", singletonList(set("b", 99)))
        ), new long[]{0, 4, 7, 2, 5, 8, 3, 6, 9});
    }

    public void testDoIfNotZero1() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                doIfNotZero("b", "+")
        ), new long[]{2, 4, 7, 0, 5, 8, 3, 6, 9});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{0, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                doIfNotZero("b", "+")
        ), new long[]{1, 4, 7, 0, 5, 8, 3, 6, 9});
    }

    public void testAdd3() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                add(asList("a", "b", "c"), "b.1")
        ), new long[]{1 + 5, 4, 7, 2 + 5, 0, 8, 3 + 5, 6, 9});
    }

    public void testAdd4() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                add(asList("a", "b", "c"), "b.1", "b.2")
        ), new long[]{1 + 5, 4, 7, 2 + 5, 5, 0, 3 + 5, 6, 9});
    }

    public void testForEach2() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach("c", ">+<")
        ), new long[]{1, 7, 7, 2, 5, 8, 0, 6, 9});
    }

    public void testForEach3() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach("c", ">+<", "a")
        ), new long[]{0, 7, 7, 2, 5, 8, 3, 6, 9});
    }

    public void testForEach4() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach("c", asList(inc("c.1")))
        ), new long[]{1, 4, 7, 2, 5, 8, 0, 9, 9});
    }

    public void testForEach5() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach("c", asList(inc("c.1")), "a")
        ), new long[]{0, 4, 7, 2, 5, 8, 3, 9, 9});
    }

    public void testSub3() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                sub(asList("a", "b", "c"), "b.1")
        ), new long[]{1 + 256 - 5, 4, 7, 2 + 256 - 5, 0, 8, 3 + 256 - 5, 6, 9});
    }

    public void testSub4() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                sub(asList("a", "b", "c"), "b.1", "b.2")
        ), new long[]{1 + 256 - 5, 4, 7, 2 + 256 - 5, 5, 0, 3 + 256 - 5, 6, 9});
    }

    public void testSet3() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{0, 6, 9}),
                set(asList("c.2", "c.1", "c"), "a.1")
        ), new long[]{1, 0, 7, 2, 5, 8, 4, 4, 4});

        test(3, 3, asList(
                set("a", new int[]{1, 0, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{0, 6, 9}),
                set(asList("c.2", "c.1", "c"), "a.1")
        ), new long[]{1, 0, 7, 2, 5, 8, 0, 0, 0});
    }

    public void testSet4() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                set(asList("c.2", "c.1", "c"), "a.1", "a.2")
        ), new long[]{1, 4, 0, 2, 5, 8, 4, 4, 4});

        test(3, 3, asList(
                set("a", new int[]{1, 0, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                set(asList("c.2", "c.1", "c"), "a.1", "a.2")
        ), new long[]{1, 0, 0, 2, 5, 8, 0, 0, 0});
    }

    public void testCat() throws Exception {

    }

    public void testLog() throws Exception {

    }

    public void testLog1() throws Exception {

    }

    public void testPow() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                pow("a.1", "c", "b.0", "b.1", "b.2")
        ), new long[]{1, 64, 7, 4, 0, 0, 0, 6, 9});

        test(5, 1, asList(
                set("a", new int[]{1, 11, 12, 13, 14}),
                pow("b", "a", "c", "d", "e")
        ), new long[]{0, 11, 11, 0, 0});

        test(5, 1, asList(
            set("a", new int[]{0, 11, 12, 13, 14}),
            pow("b", "a", "c", "d", "e")
        ), new long[]{0, 1, 11, 13, 14});
    }

    public void testDiv() throws Exception {
    }

    public void testSwap() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                swap("a", "b.1", "b"),
                swap("b.1", "c.2", "c.1")
        ), new long[]{
                5, 4, 7, 0, 9, 8, 3, 0, 1
        });
    }

    public void testMult() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                mult("b.1", 5, "c")
        ), new long[]{1, 4, 7, 2, 25, 8, 0, 6, 9});

        test(3, 3, asList(
                set("a", new int[]{0, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                mult("a.0", 5, "c.0")
        ), new long[]{0, 4, 7, 2, 5, 8, 0, 6, 9});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                mult("b.1", 0, "c.0")
        ), new long[]{1, 4, 7, 2, 0, 8, 0, 6, 9});
    }

    public void testMult1() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                mult("a.1", "b.1", "c.1", "c.2")
        ), new long[]{1, 20, 7, 2, 5, 8, 3, 0, 0});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 0, 8}),
                set("c", new int[]{3, 0, 9}),
                mult("a.1", "b.1", "c.1", "c.2")
        ), new long[]{1, 0, 7, 2, 0, 8, 3, 0, 0});

        test(3, 3, asList(
                set("a", new int[]{1, 0, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 0}),
                mult("a.1", "b.1", "c.1", "c.2")
        ), new long[]{1, 0, 7, 2, 5, 8, 3, 0, 0});
    }

    public void testGetSizeOf() throws Exception {
        test(3, 3, asList(
            set("a", new int[]{1, 4, 7}),
            set("b", new int[]{2, 5, 8}),
            set("c", new int[]{3, 6, 9}),
                getSizeOf("b", "c"),
                getSizeOf("b.1", "b.1")
        ), new long[]{1, 4, 7, 3, 1, 8, 3, 6, 9});
    }

    public void testGetReference() throws Exception {
        test(3, 1, asList(
                set("c", 5),
                getReference("a", "c")
        ), new long[]{2, 0, 5});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                getReference("a.0", "c.2"),
                getReference("a.1", "b.1"),
                getReference("a.2", "a.2")
        ), new long[]{8, 4, 2, 2, 5, 8, 3, 6, 9});
    }

    public void testForEach() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach(2, asList(inc("b")))
        ), new long[]{1, 4, 7, 3, 6, 8, 3, 6, 9});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach(3, asList(zero("a.1")))
        ), new long[]{1, 0, 0, 0, 5, 8, 3, 6, 9});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach(0, asList(zero("c")))
        ), new long[]{1, 4, 7, 2, 5, 8, 3, 6, 9});
    }

    public void testForEach1() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                forEach(2, asList(inc("a.0")), asList(zero("a.1")), asList(inc("a.1")))
        ), new long[]{3, 2, 0, 2, 5, 8, 3, 6, 9});
    }

    public void testForEachCell() throws Exception {
        List<Instruction> instr = asList(
                forEachCell("a", singletonList(
                        Instruction.movePointer("a", "-"))),
                forEachCell("b", singletonList(Instruction.movePointer("b", "+++")))
        );

        test(2, 3, instr, new long[]{256-1, 256-1, 256-1, 3, 3, 3});

        test(2, 1, instr, new long[]{256-1, 3});

        test(3, 3, asList(
                set("a", new int[]{0, 1, 2}),
                set("b", new int[]{3, 4, 5}),
                set("c", new int[]{6, 7, 8}),
                forEachCell("c", singletonList(
                        Instruction.movePointer("c", "----"))),
                forEachCell("b", singletonList(Instruction.movePointer("b", "<[->+<]>")))
        ), new long[]{0, 1, 0, 0, 0, 14, 2, 3, 4});
    }

    public void testForEachCell1() throws Exception {

    }

    public void testDirectWrite() throws Exception {

    }

    public void testMove() throws Exception {
        test(2, 3, asList(
                set("a.0", 5),
                set("a.1", 4),
                set("a.2", 3),
                set("b.0", 2),
                set("b.1", 6),
                set("b.2", 3),
                move("a", "b")
        ), new long[]{2, 6, 3, 0, 0, 0});

        test(4, 3, asList(
                set("a.0", 5),
                set("a.1", 4),
                set("a.2", 3),
                set("b.0", 2),
                set("b.1", 6),
                set("b.2", 3),
                set("c.0", 15),
                set("c.1", 17),
                set("c.2", 19),
                set("d.0", 14),
                set("d.1", 15),
                set("d.2", 16),
                move("a", "c")
        ), new long[]{15, 17, 19, 2, 6, 3, 0, 0, 0, 14, 15, 16});

        test(3, 2, asList(
                set("a.0", 5),
                set("a.1", 4),
                set("b.0", 2),
                set("b.1", 6),
                set("c.0", 8),
                set("c.1", 5),
                move("c", "a")
        ), new long[]{0, 0, 2, 6, 5, 4});

        test(3, 2, asList(
                set("a.0", 1),
                set("a.1", 3),
                set("b.0", 0),
                set("b.1", 0),
                set("c.0", 0),
                set("c.1", 0),
                move("c", "b")
        ), new long[]{1, 3, 0, 0, 0, 0});
    }

    public void testMove1() throws Exception {
        test(3, 3, asList(
                set("a.0", 5),
                set("a.1", 4),
                set("a.2", 3),
                set("b.0", 10),
                set("b.1", 11),
                set("b.2", 12),
                set("c.0", 2),
                set("c.1", 6),
                set("c.2", 3),
                move("a", "c", "b")
        ), new long[]{2, 6, 3, 0, 11, 12, 2, 6, 3});

        test(3, 3, asList(
                set("a.0", 0),
                set("a.1", 0),
                set("a.2", 0),
                set("b.0", 0),
                set("b.1", 1),
                set("b.2", 2),
                set("c.0", 0),
                set("c.1", 0),
                set("c.2", 0),
                move("c", "b", "a")
        ), new long[]{0, 0, 0, 0, 1, 2, 0, 1, 2});
    }

    public void testMove2() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                move(3, asList("a"), asList("a"))
        ), new long[]{2, 5, 8, 0, 0, 0, 3, 6, 9});
    }

    public void testMove3() throws Exception {
        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                move(3, "a")
        ), new long[]{0, 0, 0, 1, 4, 7, 3, 6, 9});
    }

    public void testEqual1() throws Exception {
        test(3, 1, asList(
                set("a", 0), set("b", 0), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{1,0,0});

        test(3, 1, asList(
                set("a", 6), set("b", 6), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{1,0,0});
        test(3, 1, asList(
                set("a", 250), set("b", 250), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{1,0,0});

        test(3, 1, asList(
                set("a", 10), set("b", 0), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 16), set("b", 6), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 20), set("b", 250), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 10), set("b", 20), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 16), set("b", 26), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 20), set("b", 5), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 20), set("b", 0), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});

        test(3, 1, asList(
                set("a", 0), set("b", 100), set("c", 3),
                equal("a", "b", "c")
        ), new long[]{0,0,0});
    }

    public void testMovePointer() throws Exception {
        test(3, 1, asList(
                movePointer("c", false), directWrite("+++++"), movePointer("c", true),
                movePointer("b", false), directWrite("++++++++"), movePointer("b", true),
                movePointer("a", false), directWrite("++++++++++"), movePointer("a", true)
        ), new long[]{10, 8, 5});
    }

    public void testMovePointer1() throws Exception {
        test(3, 1, asList(
                movePointer(2), directWrite("+++++"), movePointer(-2),
                movePointer(1), directWrite("++++++++"), movePointer(-1),
                directWrite("++++++++++")
        ), new long[]{10, 8, 5});
    }

    public void testComment() throws Exception {

    }

    public void testComment1() throws Exception {

    }

    public void testWriteString() throws Exception {
        char[] hw =  "Hello, World!".toCharArray();
        long[] hwl = new long[hw.length];
        for (int i = 0 ; i < hwl.length; i++) hwl[i] = hw[i];
        test(hw.length, 1, asList(writeString("a", new String(hw), hw.length)), hwl);
    }

    public void testCompileInstructions() throws Exception {

    }

    public void testMovePointerToVariableAndInsert() throws Exception {
        test(20, 1, asList(
                Instruction.movePointer("f", "+++"),
                Instruction.movePointer("b", "++"),
                Instruction.movePointer("f", "+++")),
                new long[]{0, 2, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
    }

    public void testIsZero() throws Exception {
        test(3, 1, asList(
                set("a", 5), isZero("a")
        ), new long[]{5, 0, 0});

        test(3, 1, asList(
                set("a", 0), isZero("a")
        ), new long[]{1, 0, 0});
    }

    public void testPrint() throws Exception {

    }

    public void testRead() throws Exception {

    }

    public void testCompile() throws Exception {

    }

    void test(int variableCount, int cellSize, List<Instruction> program, long[] resultMemory) {
        Interpreter interpreter = new Interpreter(variableCount * cellSize);
        SymbolTable symbols = new SymbolTable(null);
        String names = "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0 ; i < variableCount; i ++) {
            char name = names.charAt(i);
            if (cellSize != 1)
                symbols.defineVariable(String.valueOf(name), "cell", cellSize);
            else symbols.defineVariable(String.valueOf(name));
        }

        String compiledProgram = compileInstructions(symbols, program);
        interpreter.loadProgramIntoMemory(compiledProgram, true);
        interpreter.run();

        assertArrayEquals(resultMemory, interpreter.getMemory().memory);
    }

    public void testWhileLoop() {
        test(2, 1, asList(set("a", 10), whileLoop("a", "[->++<]")), new long[]{0, 20});
    }

    public void testWhileLoop1() {
        test(2, 1, asList(set("a", 10), whileLoop("a", asList(dec("a"), add("b", 10)))), new long[]{0, 100});
    }

    public void testIfClause() {
        /*
        c = 50
        d = 60
        if a:
            c = 1
            if b:
                d = 1
         */
        test(4, 1, asList(set("a", 4), set("b", 2), set("c", 50), set("d", 60),
                ifClause("a", asList(
                        one("c"),
                        ifClause("b", singletonList(one("d"))))
                )), new long[]{0,0,1,1});

        test(4, 1, asList(set("a", 0), set("b", 2), set("c", 50), set("d", 60),
                ifClause("a", asList(
                        one("c"),
                        ifClause("b", singletonList(one("d"))))
                )), new long[]{0,2,50,60});

        test(4, 1, asList(set("a", 12), set("b", 0), set("c", 50), set("d", 60),
                ifClause("a", asList(
                        one("c"),
                        ifClause("b", singletonList(one("d"))))
                )), new long[]{0,0,1,60});

        test(4, 1, asList(set("a", 0), set("b", 0), set("c", 50), set("d", 60),
                ifClause("a", asList(
                        one("c"),
                        ifClause("b", singletonList(one("d"))))
                )), new long[]{0,0,50,60});
    }

    public void testIfElse() {
        /*
        c = 50
        d = 60
        e = 70
        f = 80 // aux
        if a:
            c = 1
            if b:
                d = 1
            else:
                e = 1
        else
            c = 0
            if b:
                d = 0
            else:
                e = 0
         */

        test(6, 1, asList(
                set("a", 3), set("b", 4), set("c", 50), set("d", 60), set("e", 70), set("f", 80),
                ifElse("a", "f",
                        asList(
                                one("c"),
                                ifElse("b", "f", asList(
                                        one("d")
                                ), asList(one("e")))
                        ), asList(
                                zero("c"),
                                ifElse("b", "f", asList(zero("d")), asList(zero("e")))
                        ))
        ), new long[]{0, 0, 1, 1, 70, 0});

        test(6, 1, asList(
                set("a", 0), set("b", 4), set("c", 50), set("d", 60), set("e", 70), set("f", 80),
                ifElse("a", "f",
                        asList(
                                one("c"),
                                ifElse("b", "f", asList(
                                        one("d")
                                ), asList(one("e")))
                        ), asList(
                                zero("c"),
                                ifElse("b", "f", asList(zero("d")), asList(zero("e")))
                        ))
        ), new long[]{0, 0, 0, 0, 70, 0});

        test(6, 1, asList(
                set("a", 3), set("b", 0), set("c", 50), set("d", 60), set("e", 70), set("f", 80),
                ifElse("a", "f",
                        asList(
                                one("c"),
                                ifElse("b", "f", asList(
                                        one("d")
                                ), asList(one("e")))
                        ), asList(
                                zero("c"),
                                ifElse("b", "f", asList(zero("d")), asList(zero("e")))
                        ))
        ), new long[]{0, 0, 1, 60, 1, 0});

        test(6, 1, asList(
                set("a", 0), set("b", 0), set("c", 50), set("d", 60), set("e", 70), set("f", 80),
                ifElse("a", "f",
                        asList(
                                one("c"),
                                ifElse("b", "f", asList(
                                        one("d")
                                ), asList(one("e")))
                        ), asList(
                                zero("c"),
                                ifElse("b", "f", asList(zero("d")), asList(zero("e")))
                        ))
        ), new long[]{0, 0, 0, 60, 0, 0});
    }

    public void testZero() {
        test(1, 1, asList(zero("a")), new long[]{0});
        test(1, 1, asList(set("a", 5), zero("a")), new long[]{0});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                zero("b")), new long[]{1, 4, 7, 0, 5, 8, 3, 6, 9});
    }

    public void testOne() {
        test(1, 1, asList(one("a")), new long[]{1});
        test(1, 1, asList(set("a", 5), one("a")), new long[]{1});
        test(1, 2, asList(set("a", new int[]{5, 6}), one("a")), new long[]{1, 6});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                one("b")), new long[]{1, 4, 7, 1, 5, 8, 3, 6, 9});
    }

    public void testInc() {
        test(1, 1, asList(set("a", 10), inc("a")), new long[]{11});
        test(1, 1, asList(set("a", 0), inc("a")), new long[]{1});
        test(1, 1, asList(set("a", 1), inc("a")), new long[]{2});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                inc("b")), new long[]{1, 4, 7, 3, 5, 8, 3, 6, 9});
    }

    public void testAdd() {
        test(1, 1, asList(add("a", 0), add("a", 0)), new long[]{0});

        test(1, 1, asList(add("a", 1), add("a", 0)), new long[]{1});
        test(1, 1, asList(add("a", 0), add("a", 1)), new long[]{1});

        test(1, 1, asList(add("a", -1), add("a", 0)), new long[]{255});
        test(1, 1, asList(add("a", 0), add("a", -1)), new long[]{255});

        test(1, 1, asList(add("a", 1), add("a", 256)), new long[]{1});
        test(1, 1, asList(add("a", 256), add("a", 1)), new long[]{1});

        test(1, 1, asList(add("a", -1), add("a", 256)), new long[]{255});
        test(1, 1, asList(add("a", 256), add("a", -1)), new long[]{255});

        test(1, 1, asList(add("a", 10), add("a", 5)), new long[]{15});
        test(1, 1, asList(add("a", 5), add("a", 10)), new long[]{15});

        test(1, 1, asList(add("a", -10), add("a", 5)), new long[]{256 - 5});
        test(1, 1, asList(add("a", 5), add("a", -10)), new long[]{256 - 5});

        test(1, 1, asList(add("a", 10), add("a", -5)), new long[]{5});
        test(1, 1, asList(add("a", -5), add("a", 10)), new long[]{5});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                add("b", 5)), new long[]{1, 4, 7, 7, 5, 8, 3, 6, 9});
    }

    public void testAdd1() {
        test(2, 1, asList(set("a", 0), set("b", 0), add("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 1), set("b", 0), add("a", "b")), new long[]{1, 0});
        test(2, 1, asList(set("a", 10), set("b", 0), add("a", "b")), new long[]{10, 0});
        test(2, 1, asList(set("a", -1), set("b", 0), add("a", "b")), new long[]{256 - 1, 0});
        test(2, 1, asList(set("a", -10), set("b", 0), add("a", "b")), new long[]{256 - 10, 0});

        test(2, 1, asList(set("a", 0), set("b", 1), add("a", "b")), new long[]{1, 0});
        test(2, 1, asList(set("a", 0), set("b", 10), add("a", "b")), new long[]{10, 0});
        test(2, 1, asList(set("a", 0), set("b", -1), add("a", "b")), new long[]{255, 0});
        test(2, 1, asList(set("a", 0), set("b", -10), add("a", "b")), new long[]{256-10, 0});

        test(2, 1, asList(set("a", 1), set("b", 1), add("a", "b")), new long[]{2, 0});
        test(2, 1, asList(set("a", 1), set("b", 10), add("a", "b")), new long[]{11, 0});
        test(2, 1, asList(set("a", 1), set("b", -1), add("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 1), set("b", -10), add("a", "b")), new long[]{256 - 9, 0});

        test(2, 1, asList(set("a", 10), set("b", 1), add("a", "b")), new long[]{11, 0});
        test(2, 1, asList(set("a", 10), set("b", 10), add("a", "b")), new long[]{20, 0});
        test(2, 1, asList(set("a", 10), set("b", -1), add("a", "b")), new long[]{9, 0});
        test(2, 1, asList(set("a", 10), set("b", -10), add("a", "b")), new long[]{0, 0});

        test(2, 1, asList(set("a", -1), set("b", 1), add("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", -1), set("b", 10), add("a", "b")), new long[]{9, 0});
        test(2, 1, asList(set("a", -1), set("b", -1), add("a", "b")), new long[]{256-2, 0});
        test(2, 1, asList(set("a", -1), set("b", -10), add("a", "b")), new long[]{256-11, 0});

        test(2, 1, asList(set("a", -10), set("b", 1), add("a", "b")), new long[]{256 - 9, 0});
        test(2, 1, asList(set("a", -10), set("b", 10), add("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", -10), set("b", -1), add("a", "b")), new long[]{256 - 11, 0});
        test(2, 1, asList(set("a", -10), set("b", -10), add("a", "b")), new long[]{256-20, 0});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                add("a", "b.1")), new long[]{6, 4, 7, 2, 0, 8, 3, 6, 9});
    }

    public void testAdd2() {
        test(3, 1, asList(set("a", 0), set("b", 0), set("c", -1), add("a", "b", "c")), new long[]{0, 0, 0});
        test(3, 1, asList(set("a", 1), set("b", 0), set("c", -1), add("a", "b", "c")), new long[]{1, 0, 0});
        test(3, 1, asList(set("a", -1), set("b", 0), set("c", -1), add("a", "b", "c")), new long[]{255, 0, 0});
        test(3, 1, asList(set("a", 10), set("b", 0), set("c", -1), add("a", "b", "c")), new long[]{10, 0, 0});
        test(3, 1, asList(set("a", -10), set("b", 0), set("c", -1), add("a", "b", "c")), new long[]{256 - 10, 0, 0});

        test(3, 1, asList(set("a", 0), set("b", 0), set("c", -1), add("b", "a", "c")), new long[]{0, 0, 0});
        test(3, 1, asList(set("a", 1), set("b", 0), set("c", -1), add("b", "a", "c")), new long[]{1, 1, 0});
        test(3, 1, asList(set("a", -1), set("b", 0), set("c", -1), add("b", "a", "c")), new long[]{255, 255, 0});
        test(3, 1, asList(set("a", 10), set("b", 0), set("c", -1), add("b", "a", "c")), new long[]{10, 10, 0});
        test(3, 1, asList(set("a", -10), set("b", 0), set("c", -1), add("b", "a", "c")), new long[]{256-10, 256-10, 0});

        test(3, 1, asList(set("a", 0), set("b", 1), set("c", -1), add("a", "b", "c")), new long[]{1, 1, 0});
        test(3, 1, asList(set("a", 1), set("b", 1), set("c", -1), add("a", "b", "c")), new long[]{2, 1, 0});
        test(3, 1, asList(set("a", -1), set("b", 1), set("c", -1), add("a", "b", "c")), new long[]{0, 1, 0});
        test(3, 1, asList(set("a", 10), set("b", 1), set("c", -1), add("a", "b", "c")), new long[]{11, 1, 0});
        test(3, 1, asList(set("a", -10), set("b", 1), set("c", -1), add("a", "b", "c")), new long[]{256-9, 1, 0});

        test(3, 1, asList(set("a", 0), set("b", 1), set("c", -1), add("b", "a", "c")), new long[] {0, 1, 0});
        test(3, 1, asList(set("a", 1), set("b", 1), set("c", -1), add("b", "a", "c")), new long[] {1, 2, 0});
        test(3, 1, asList(set("a", -1), set("b", 1), set("c", -1), add("b", "a", "c")), new long[]{255, 0, 0});
        test(3, 1, asList(set("a", 10), set("b", 1), set("c", -1), add("b", "a", "c")), new long[]{10, 11, 0});
        test(3, 1, asList(set("a", -10), set("b", 1), set("c", -1), add("b", "a", "c")), new long[]{256-10, 256-9, 0});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                add("a", "b.1", "c.2")), new long[]{6, 4, 7, 2, 5, 8, 3, 6, 0});
    }

    public void testDec() {
        test(1, 1, asList(set("a", 10), dec("a")), new long[]{9});
        test(1, 1, asList(set("a", 0), dec("a")), new long[]{255});
        test(1, 1, asList(set("a", 1), dec("a")), new long[]{0});
    }

    public void testSub() {
        test(1, 1, asList(set("a", 10), sub("a", 5)), new long[]{5});
        test(1, 1, asList(set("a", 10), sub("a", -5)), new long[]{15});
        test(1, 1, asList(set("a", 5), sub("a", -5)), new long[]{10});
        test(1, 1, asList(set("a", 5), sub("a", 5)), new long[]{0});
        test(1, 1, asList(set("a", 0), sub("a", 0)), new long[]{0});
        test(1, 1, asList(set("a", 10), sub("a", 0)), new long[]{10});
        test(1, 1, asList(set("a", 0), sub("a", -5)), new long[]{5});
        test(1, 1, asList(set("a", 0), sub("a", 5)), new long[]{251});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                sub("a", 5)), new long[]{256 - 4, 4, 7, 2, 5, 8, 3, 6, 9});
    }

    public void testSub1() {
        test(2, 1, asList(set("a", 0), set("b", 0), sub("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 1), set("b", 0), sub("a", "b")), new long[]{1, 0});
        test(2, 1, asList(set("a", 10), set("b", 0), sub("a", "b")), new long[]{10, 0});
        test(2, 1, asList(set("a", -1), set("b", 0), sub("a", "b")), new long[]{256 - 1, 0});
        test(2, 1, asList(set("a", -10), set("b", 0), sub("a", "b")), new long[]{256 - 10, 0});

        test(2, 1, asList(set("a", 0), set("b", 1), sub("a", "b")), new long[]{255, 0});
        test(2, 1, asList(set("a", 0), set("b", 10), sub("a", "b")), new long[]{256 - 10, 0});
        test(2, 1, asList(set("a", 0), set("b", -1), sub("a", "b")), new long[]{1, 0});
        test(2, 1, asList(set("a", 0), set("b", -10), sub("a", "b")), new long[]{10, 0});

        test(2, 1, asList(set("a", 1), set("b", 1), sub("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 1), set("b", 10), sub("a", "b")), new long[]{256 - 9, 0});
        test(2, 1, asList(set("a", 1), set("b", -1), sub("a", "b")), new long[]{2, 0});
        test(2, 1, asList(set("a", 1), set("b", -10), sub("a", "b")), new long[]{11, 0});

        test(2, 1, asList(set("a", 10), set("b", 1), sub("a", "b")), new long[]{9, 0});
        test(2, 1, asList(set("a", 10), set("b", 10), sub("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 10), set("b", -1), sub("a", "b")), new long[]{11, 0});
        test(2, 1, asList(set("a", 10), set("b", -10), sub("a", "b")), new long[]{20, 0});

        test(2, 1, asList(set("a", -1), set("b", 1), sub("a", "b")), new long[]{256 - 2, 0});
        test(2, 1, asList(set("a", -1), set("b", 10), sub("a", "b")), new long[]{256 - 11, 0});
        test(2, 1, asList(set("a", -1), set("b", -1), sub("a", "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", -1), set("b", -10), sub("a", "b")), new long[]{9, 0});

        test(2, 1, asList(set("a", -10), set("b", 1), sub("a", "b")), new long[]{256 - 11, 0});
        test(2, 1, asList(set("a", -10), set("b", 10), sub("a", "b")), new long[]{256 - 20, 0});
        test(2, 1, asList(set("a", -10), set("b", -1), sub("a", "b")), new long[]{256 - 10 + 1, 0});
        test(2, 1, asList(set("a", -10), set("b", -10), sub("a", "b")), new long[]{0, 0});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                sub("a", "b.1")), new long[]{256 - 4, 4, 7, 2, 0, 8, 3, 6, 9});
    }

    public void testSub2() {
        test(3, 1, asList(set("a", 0), set("b", 0), set("c", -1), sub("a", "b", "c")), new long[]{0, 0, 0});
        test(3, 1, asList(set("a", 1), set("b", 0), set("c", -1), sub("a", "b", "c")), new long[]{1, 0, 0});
        test(3, 1, asList(set("a", -1), set("b", 0), set("c", -1), sub("a", "b", "c")), new long[]{255, 0, 0});
        test(3, 1, asList(set("a", 10), set("b", 0), set("c", -1), sub("a", "b", "c")), new long[]{10, 0, 0});
        test(3, 1, asList(set("a", -10), set("b", 0), set("c", -1), sub("a", "b", "c")), new long[]{256 - 10, 0, 0});

        test(3, 1, asList(set("a", 0), set("b", 0), set("c", -1), sub("b", "a", "c")), new long[]{0, 0, 0});
        test(3, 1, asList(set("a", 1), set("b", 0), set("c", -1), sub("b", "a", "c")), new long[]{1, 255, 0});
        test(3, 1, asList(set("a", -1), set("b", 0), set("c", -1), sub("b", "a", "c")), new long[]{255, 1, 0});
        test(3, 1, asList(set("a", 10), set("b", 0), set("c", -1), sub("b", "a", "c")), new long[]{10, 246, 0});
        test(3, 1, asList(set("a", -10), set("b", 0), set("c", -1), sub("b", "a", "c")), new long[]{246, 10, 0});

        test(3, 1, asList(set("a", 0), set("b", 1), set("c", -1), sub("a", "b", "c")), new long[]{255, 1, 0});
        test(3, 1, asList(set("a", 1), set("b", 1), set("c", -1), sub("a", "b", "c")), new long[]{0, 1, 0});
        test(3, 1, asList(set("a", -1), set("b", 1), set("c", -1), sub("a", "b", "c")), new long[]{254, 1, 0});
        test(3, 1, asList(set("a", 10), set("b", 1), set("c", -1), sub("a", "b", "c")), new long[]{9, 1, 0});
        test(3, 1, asList(set("a", -10), set("b", 1), set("c", -1), sub("a", "b", "c")), new long[]{256-11, 1, 0});

        test(3, 1, asList(set("a", 0), set("b", 1), set("c", -1), sub("b", "a", "c")), new long[] {0, 1, 0});
        test(3, 1, asList(set("a", 1), set("b", 1), set("c", -1), sub("b", "a", "c")), new long[] {1, 0, 0});
        test(3, 1, asList(set("a", -1), set("b", 1), set("c", -1), sub("b", "a", "c")), new long[]{255, 2, 0});
        test(3, 1, asList(set("a", 10), set("b", 1), set("c", -1), sub("b", "a", "c")), new long[]{10, 256-9, 0});
        test(3, 1, asList(set("a", -10), set("b", 1), set("c", -1), sub("b", "a", "c")), new long[]{256-10, 11, 0});

        test(3, 3, asList(
                set("a", new int[]{1, 4, 7}),
                set("b", new int[]{2, 5, 8}),
                set("c", new int[]{3, 6, 9}),
                sub("a", "b.1", "c.2")), new long[]{256 - 4, 4, 7, 2, 5, 8, 3, 6, 0});
    }

    public void testSet() {
        test(1, 2, asList(set("a", 10)), new long[]{10, 0});

        test(3, 3, asList(
                set("a", 1),
                set("b.1", 2),
                set("c.2", 3)
        ), new long[]{1, 0, 0, 0, 2, 0, 0, 0, 3});
    }

    public void testSet1() {
        test(2, 1, asList(set("a", 10), set("b", "a")), new long[]{0, 10});

        test(3, 3, asList(
                set("a", 1),
                set("b.1", 2),
                set("c.2", 3),
                set("b", "a")
        ), new long[]{0, 0, 0, 1, 2, 0, 0, 0, 3});
    }

    public void testSet2() throws Exception {
        test(1, 2, asList(set("a", new int[]{3, 4})), new long[]{3, 4});

        test(2, 2, asList(set("a", new int[]{3, 4}), set("b", new int[]{5, 6})), new long[]{3, 4, 5, 6});

        test(3, 3, asList(
                set("c", new int[]{7, 8, 9}),
                set("a", new int[]{1, 2, 3}),
                set("b", new int[]{4, 5, 6})
        ), new long[] {1, 2, 3, 4, 5, 6, 7, 8, 9});

        test(3, 3, asList(
                set("a", 1),
                set("b.1", 2),
                set("c.2", 3),
                set("b", "a", "c.2")
        ), new long[]{1, 0, 0, 1, 2, 0, 0, 0, 0});
    }

    public void testSwitchStatement() throws Exception {
        for (int a = 0; a < 10; a++) {
            for (int b = 0; b < 10; b++) {
                test(3, 1, asList(
                        set("a", a), set("b", b), set("c", 3),
                        switchStatement(
                                "a", "b", asList(0, 1, 2, 3), asList(">>[-]+<<", ">>[-]++<<", ">>[-]++++<<", ">>[-]++++++++<<", ">>[-]++++++++++++++++<<")
                        )
                ), new long[]{0, 0, a > 3 ? (long) Math.pow(2, 4) : (long) Math.pow(2, a)});

                test(3, 1, asList(
                        set("a", a), set("b", b), set("c", 3),
                        switchStatement(
                                "a", "b", asList(0, 1, 2, 3), asList(">>[-]+<<", ">>[-]++<<", ">>[-]++++<<", ">>[-]++++++++<<")
                        )
                ), new long[]{0, 0, a >= 4 ? 3 : (long) Math.pow(2, a)});
            }
        }
    }

    public void testEqual() {
        test(2, 1, asList(set("a", 0), equal("a", 0, "b")), new long[]{1, 0});

        test(2, 1, asList(set("a", 1), equal("a", 0, "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 0), equal("a", 1, "b")), new long[]{0, 0});

        test(2, 1, asList(set("a", 1), equal("a", 1, "b")), new long[]{1, 0});
        test(2, 1, asList(set("a", 1), equal("a", 1, "b")), new long[]{1, 0});

        test(2, 1, asList(set("a", 1), equal("a", 5, "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 5), equal("a", 1, "b")), new long[]{0, 0});

        test(2, 1, asList(set("a", 5), equal("a", 5, "b")), new long[]{1, 0});
        test(2, 1, asList(set("a", 5), equal("a", 5, "b")), new long[]{1, 0});

        test(2, 1, asList(set("a", 5), equal("a", 3, "b")), new long[]{0, 0});
        test(2, 1, asList(set("a", 3), equal("a", 5, "b")), new long[]{0, 0});
    }
}