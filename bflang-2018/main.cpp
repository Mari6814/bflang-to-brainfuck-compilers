#include <fstream>
#include <sstream>
#include <boost/program_options.hpp>
#include "bf.h"
#include "print.h"

namespace po = boost::program_options;

std::string currentFile;
std::ofstream binary_output_stream;
std::ofstream intermediate_output_stream;
bool verbose;
bool verboseSymbolTable;
bool debug;
bool verboseSymbolNames;

// implementation
void yyerror(const char *s) {
    fprintf(stderr, "%s:%i Error: %s\n", currentFile.c_str(), yylineno, s);
    exit(EXIT_FAILURE);
}

std::string optimize(std::string &buffer, std::string line) {
    for (char c : line) {
        if (buffer.size() == 0 || c == buffer.back())
            // add character to buffer if it is a repetition
            buffer.push_back(c);
        else if ((c == '+' && buffer.back() == '-')
                || (c == '-' && buffer.back() == '+')
                || (c == '>' && buffer.back() == '<')
                || (c == '<' && buffer.back() == '>'))
            // if the characters are opposites, remove both
            buffer.erase(buffer.size() - 1);
        else {
            // if they are unreleated, add the buffer to the output and clear it
            buffer.push_back(c);
        }
    }
    return buffer;
}

int main(int argc, char const* const*argv) {

    po::variables_map vm;
    po::options_description desc("A simple compiler for brainfuck");
    try {
        po::positional_options_description p;

        p.add("input", -1);
        desc.add_options()
                ("help", "this message")
                ("input,i", po::value<std::vector<std::string>>(), "input source file names")
                ("import-path,I", po::value<std::vector<std::string>>()->multitoken(), "Paths to search for input files")
                ("output,o", po::value<std::string>()->default_value("a.b"), "name for the compiled brainfuck binary file")
                ("output-intermediate", po::value<std::string>(), "name for the intermediate bytecode file")
                ("output-symbol-table", po::value<std::string>(), "file for the symbol table")
                ("verbose-symbol-table", "more verbose symbol table output")
                ("verbose,v", "verbose output to std::out")
                ("optimization,O", po::value<unsigned>()->default_value(1), "Optimization level. 0=off, 1=on.")
                ("verbose-symbol-names,V", "displays full path of all symbols")
                ("debug,d", "compiles with debug information");

        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    if (vm.count("help")) {
        println(desc);
        return EXIT_SUCCESS;
    }

    if (!vm.count("input")) {
        errprintln("Define an input with --input <file> or -i <file>");
        println(desc);
        return EXIT_FAILURE;
    }

    verbose = static_cast<bool>(vm.count("verbose"));

    debug = static_cast<bool>(vm.count("debug"));
    if (verbose)
        println("Debug symbols:", debug ? "on" : "off");

    int optimizationLevel = vm["optimization"].as<unsigned>();
    if (verbose)
        println("Optimization level: ", optimizationLevel);

    verboseSymbolTable = static_cast<bool>(vm.count("verbose-symbol-table"));
    if (verbose)
        println("Verbose symbol table:", verboseSymbolTable ? "on" : "off");

    verboseSymbolNames = static_cast<bool>(vm.count("verbose-symbol-names"));
    if (verbose)
        println("Verbose symbol names:", verboseSymbolNames ? "on" : "off");


    CompilationState state;

    auto output_path = vm["output"].as<std::string>();
    if (!output_path.empty()) {
        binary_output_stream.open(output_path);
        if (!binary_output_stream.is_open()) {
            errprintln("Could not create destination for compiled binary file at", output_path);
            return EXIT_FAILURE;
        }
        if (verbose)
            println("Writing binary to", output_path);
    } else if (verbose) {
        errprintln("Invalid output file name");
        return EXIT_FAILURE;
    }

    if (vm.count("output-intermediate")) {
        auto output_path = vm["output-intermediate"].as<std::string>();
        if (!output_path.empty()) {
            intermediate_output_stream.open(output_path);
            if (!intermediate_output_stream) {
                errprintln("Could not create destination for intermediate bytecode file at", output_path);
                return EXIT_FAILURE;
            }
            if (verbose)
                println("Writing intermediate to", output_path);
        } else if (verbose) {
            errprintln("Invalid destination for intermediate output");
            return EXIT_FAILURE;
        }
    } else if (verbose)
        println("No intermediate file will be generated");

    for (const auto &inputFilePath : vm["input"].as<std::vector<std::string>>()) {
        currentFile = inputFilePath;
        yyin = fopen(currentFile.c_str(), "r");

        if ((yyin == nullptr) && vm.count("import-path")) {
            for (const auto &importPath : vm["import-path"].as<std::vector<std::string>>()) {
                auto concatPath = importPath + "/" + inputFilePath;
                currentFile = concatPath;
                if ((yyin = fopen(concatPath.c_str(), "r")) != nullptr)
                    break;
            }
        }

        if (yyin != nullptr) {
            if (verbose)
                println("Compiling:", currentFile);
            yyrestart(yyin);
        } else {
            errprintln("Can't locate input file '" + currentFile + "'");
            return EXIT_FAILURE;
        }

        yyparse();
        fclose(yyin);
        yyin = nullptr;

        if (bisonAST == nullptr) {
            errprintln("Failed to compile", currentFile);
            return EXIT_FAILURE;
        }

        bisonAST->function = state.symbolTable.currentScope();
        bisonAST->compile(state);

        if (!(state.symbolTable.scopeStack.size() == 1 && state.symbolTable.scopeStack[0]->name == "__root__")) {
            errprintln("Scopes not properly deconstructed");
            return EXIT_FAILURE;
        }

        delete bisonAST;
        bisonAST = nullptr;
        yylineno = 0;
    }

    if (binary_output_stream.is_open()) {
        binary_output_stream.flush();
        binary_output_stream.close();
        if (verbose)
            println("Binary output stream closed");
    }

    if (intermediate_output_stream.is_open()) {
        intermediate_output_stream.flush();
        intermediate_output_stream.close();
        if (verbose)
            println("Intermediate output stream closed");
    }

    if (vm.count("output-symbol-table") != 0u) {
        string outfileName = vm["output-symbol-table"].as<std::string>();
        if (outfileName.size() > 0) {
            if (verbose)
                println("Writing symbol table to", outfileName);
            ofstream symbolTableFile(outfileName);
            if (symbolTableFile.is_open()) {
                osprintln(symbolTableFile, state.symbolTable);
            } else {
                errprintln("Could open destination for the symbol table");
            }
        } else if (verbose)
            println("No symbol table written");
    }

    if (state.main == nullptr) {
        errprintln("No main function found!");
        exit(EXIT_FAILURE);
    }

    if (!debug) {
        auto output_path = vm["output"].as<std::string>();
        if (output_path.size() == 0)
            exit(EXIT_SUCCESS);
        ofstream out(output_path + ".tmp");
        assert(out.is_open());
        {
            string line;
            out << ">>" << string(static_cast<unsigned long>(state.main->address), '+') << ">+[";
            ifstream iout(output_path);
            if (optimizationLevel == 1) {
                string buffer;
                while (getline(iout, line))
                    optimize(buffer, line);
                out << buffer;
            } else {
                while (getline(iout, line))
                    out << line;
            }
            out << "]";
            out.flush();
        }
        out.close();
        rename((output_path + ".tmp").c_str(), output_path.c_str());
    }
}