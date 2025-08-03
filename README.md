# bflang: High-Level Language to Brainfuck Compiler Collection

## Project History & Structure

`bflang` is a collection of experimental compilers and interpreters for custom high-level languages that target Brainfuck as a form of "bytecode." The project documents my journey in language design and compiler construction, spanning from my transition from school to university, through my year abroad at the University of Tokyo, and later attempts to improve upon my work after reading the "Dragon Book" (Compilers: Principles, Techniques, and Tools).
I am creating this documentation many years after the original work, so some details may be wrong or missing.
I was very into C++ and very convinced that C++ was the best thing ever, though over time this opinion has changed significantly... I still had a lot of fun implementing this project and I believe that it is my favorite project that I ever wrote with the most impressive results.

### Directories

- **bflang/**: The original implementation, started circa 2014/2015, during my transition from school to university. In 2017, while at the University of Tokyo, I extended and completed this compiler as a course project. It compiles a custom high-level language into Brainfuck code.
- **bflang-2018/**: A historical copy of `bflang`, kept for legacy reasons. Both directories are essentially the same.
- **bflang-2020/**: A new attempt started in 2020, after my year at the University of Tokyo and reading the Dragon Book. This version introduces a more sophisticated compiler pipeline, including intermediate representations and ANTLR grammars.

### Language features

- default type of identifiers is the built-in `cell` (`type ttt = player, a00, ...`, where `player` and `a00` become `player:cell` and `a00:cell`)
- `cell`s are just unsigned bytes (because this is brainfuck)
- has newtypes (`type ttt =`) that, when used, tell the compile to allocate the expected amount on the stack
- can define function on custom types (`fun ttt.Print self` allows `var game:ttt; ...; game.Print()`)
- can define functions on builtin types (`fun cell.eq ...` allows `5.eq(...)`)
- functions have to define the implicit `receiver` as first argument (`fun cell.eq self` allows `5.eq...` while `fun eq self` would only allow `eq(5...)`)
- functions accept arguments and can be called when providing them (`cell.eq self, that` allow calling `5.eq(6)`)
- you have named return values (`-> out:cell` = `-> out`)
- you can define multipe return types (`-> value1:type1, value2:type2`)
- you can optionally `return` to set the output without naming it (`fun f -> out { return 5; }` is the same as `fun f -> out { out = 5; }`)
- the underlying brainfuck bytecode implements a stack that does support recursion! See [recursion.bl](bflang/examples/recursion.bl) and its output [recursion.b](bflang/examples/dst/recursion.b).
- and many more features...

## Example: bflang (2014-2018)

Below is an excerpt from the source file [ttt.bl](bflang/examples/ttt.bl), which implements a playable Tic-Tac-Toe game in the custom high-level language.

```bl
fun cell.eq self, that -> out:cell {
	out = 1;
	if (self - that)
		out = 0;
}

type ttt = player, a00, a01, a02, a10, a11, a12, a20, a21, a22;

fun init -> self:ttt {
	self.player = 'x';
	self.a00 = '1'; self.a01 = '2'; self.a02 = '3';
	self.a10 = '4'; self.a11 = '5'; self.a12 = '6';
	self.a20 = '7'; self.a21 = '8'; self.a22 = '9';
}

fun ttt.other self:ttt -> next_player:cell {
	next_player = 'o';
	if (self.player - 'x')
		next_player = 'x';
}

fun ttt.Print self:ttt {
	print self.player, "'s Turn\n",
		  self.a00, self.a01, self.a02, '\n',
		  self.a10, self.a11, self.a12, '\n',
		  self.a20, self.a21, self.a22, '\n';
}

// Check the rest of the code in [ttt.bl.b](bflang/examples/ttt.bl.b) for the complete implementation
// it has been omitted for brevity but does actually work!

fun main {
	var state:ttt;
	state = init();
	var winner;
	winner = 1;
	while (winner) {
		state.Print();
		var x,y;
		x, y = getInput();
		state = state.set(x, y);
		winner = state.winner(state.other());
		if (winner) {
			print "Winner=", winner, '\n';
			winner = 0;
		} else {
			winner = 1;
		}
	}
}
```

The compiler produces a corresponding Brainfuck output file [ttt.bl.b](bflang/examples/ttt.bl.b). _You can use this output file in any Brainfuck simulator to play Tic-Tac-Toe interactively!_ E.g. you can paste ttt.b into https://copy.sh/brainfuck/, but you have to pre-insert your input at the bottom. The input `17589` (includes x and o players!) would yield the final board one turn before x wins by inserting their final mark at slot 9. It looks like I didn't program that the final board should be shown back then...
```
x's Turn
x23
4x6
oo9
Winner=x
```

## Example: bflang-2020 (2020)

After reading the Dragon Book, I started a new project with a more advanced architecture. The new pipeline uses ANTLR grammars (`grammars/BrainfuckLang.g4` and `grammars/BrainfuckLangAssembly.g4`) to parse the source language and generate intermediate files (`examples/basm/`).

Here is an example of a source file from [alias.bl](bflang-2020/examples/bl/alias.bl):

```bflang
???
```

The compiler then generates an intermediate representation, for example, [relativeJump.basm](bflang-2020/examples/basm/relativeJump.basm):

```basm
???
```

These intermediate files are intended to be compiled further into Brainfuck, similar to the output of the original project. However, this new pipeline was not fully completed.

## How to Use

1. Explore the `bflang-2018/examples/` directories for source files and outputs.
2. Use the output `.b` files in [examples](bflang/examples) or [dst](bflang/examples/dst/) in a Brainfuck interpreter or simulator to run the compiled programs.
3. For the 2020 version, inspect the intermediate `.basm` files to see the new compilation approach, though I did not complete this attempt.
