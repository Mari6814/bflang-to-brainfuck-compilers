# bflang: High-Level Language to Brainfuck Compiler Collection

## Project History & Structure

`bflang` is a collection of experimental compilers and interpreters for custom high-level languages that target Brainfuck as a form of "bytecode." The project documents my journey in language design and compiler construction, spanning from my transition from school to university, through my year abroad at the University of Tokyo, and later attempts to improve upon my work after reading the "Dragon Book" (Compilers: Principles, Techniques, and Tools).
I am creating this documentation many years after the original work, so some details may be wrong or missing.
I was very into C++ and very convinced that C++ was the best thing ever, though over time this opinion has changed significantly... I still had a lot of fun implementing this project and I believe that it is my favorite project that I ever wrote with the most impressive results.

### Directories

- **bflang/**: The original implementation, started circa 2014/2015, during my transition from school to university. In 2017, while at the University of Tokyo, I extended and completed this compiler as a course project. It compiles a custom high-level language into Brainfuck code.
- **bflang-2018/**: A historical copy of `bflang`, kept for legacy reasons. Both directories are essentially the same.
- **bflang-2020/**: A new attempt started in 2020, after my year at the University of Tokyo and reading the Dragon Book. This version introduces a more sophisticated compiler pipeline, including intermediate representations and ANTLR grammars.

## Example: bflang (2014-2018)

Below is an excerpt from the source file `examples/ttt.bl`, which implements a playable Tic-Tac-Toe game in the custom high-level language:

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

// Check the rest of the code in `examples/ttt.bl` for the complete implementation
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

The compiler produces a corresponding Brainfuck output file (`examples/dst/ttt.bl.b`). _You can use this output file in any Brainfuck simulator to play Tic-Tac-Toe interactively!_

## Example: bflang-2020 (2020)

After reading the Dragon Book, I started a new project with a more advanced architecture. The new pipeline uses ANTLR grammars (`grammars/BrainfuckLang.g4` and `grammars/BrainfuckLangAssembly.g4`) to parse the source language and generate intermediate files (`examples/basm/`).

Here is an example of a source file from `examples/bl/alias.bl`:

```bflang
???
```

The compiler then generates an intermediate representation, for example, `examples/basm/relativeJump.basm`:

```basm
???
```

These intermediate files are intended to be compiled further into Brainfuck, similar to the output of the original project. However, this new pipeline was not fully completed.

## How to Use

1. Explore the `bflang-2018/examples/` directories for source files and outputs.
2. Use the output `.b` files in `bflang-2018/examples/dst/` in a Brainfuck interpreter or simulator to run the compiled programs.
3. For the 2020 version, inspect the intermediate `.basm` files to see the new compilation approach, though I did not complete this attempt.
