//
// Created by Marian Plivelic on 2017/07/22.
//
#include <iomanip>
#include "interpreter.h"

namespace po = boost::program_options;

int main(int argc, const char* argv[]) {
    using namespace std;

    po::variables_map vm;
    po::options_description desc("This is a simple interpreter for the standard brainfuck bytecode.");
    try {
        po::positional_options_description p;

        p.add("input", -1);
        desc.add_options()
                ("help", "This message")
                ("input,i", po::value<std::string>(), "Input source file")
                ("init", po::value<char>()->default_value(0), "Sets the initial value for all cells in memory")
//                ("wrapping", po::value<bool>()->default_value(true), "Sets the wrapping mode. on|off")
                ("breakpoints,b", po::value<std::vector<size_t>>()->multitoken(), "Program outputs memory and stops at the specified instruction indices")
                ("debug,d", po::value<std::string>()->implicit_value("#"), "Enables printing memory contents to stderr each time before the specified character is executed.")
                ("debug-interrupt,D", po::value<bool>()->implicit_value(false), "If set, interrupts program every time the print character is encountered.")
                ("stdin", po::value<std::vector<std::string>>()->multitoken(), "Uses a constant list of input values. Uses the value of --const if all values in --stdin are consumed.")
                ("const,c", po::value<unsigned>()->default_value(0), "Value used if --stdin is empty")
                ("memory,m", po::value<size_t>()->default_value(1024), "Sets the maximum amount of memory given to the program")
                ("numerical-input,u", "Enables numerical input (e.g.: reads input '64' as 'A')")
                ("numerical-output,U", "Enables numerical output (e.g.: prints '64' instead of 'A')")
                ("verbose,v", "Verbose output");

        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
    } catch (std::exception &e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    bool verbose = false;

    if (vm.count("verbose"))
        verbose = true;

    if (vm.count("help")) {
        cout << desc;
        return EXIT_SUCCESS;
    }

    if (!vm.count("input")) {
        cerr << desc;
        return EXIT_FAILURE;
    }

    bool debug = static_cast<bool>(vm.count("debug"));
    char debugInstruction = debug ? vm["debug"].as<std::string>()[0] : '#';
    if (verbose)
        cout << "Debug mode: " << (debug ? to_string(debugInstruction) : string("off")) << endl;

    bool debugi = static_cast<bool>(vm.count("debug-interrupt"));
    if (verbose)
        cout << "Debug interrupt handle method: " << (debugi ? "Interrupt" : "Continue") << endl;

    bool useBreakpoints = static_cast<bool>(vm.count("breakpoints"));
    std::vector<size_t> breakpoints = useBreakpoints ? vm["breakpoints"].as<vector<size_t>>() : vector<size_t>();
    if (verbose)
        cout << "Breakpoints: " << (useBreakpoints ? string("on") : string("off"));

    bool numericalOutput = static_cast<bool>(vm.count("numerical-output"));
    if (verbose)
        cout << "Unsigned output: " << (numericalOutput ? string("on") : string("off")) << endl;

    bool numericalInput = static_cast<bool>(vm.count("numerical-input"));
    if (verbose)
        cout << "Unsigned input: " << (numericalInput ? string("on") : string("off")) << endl;

    size_t memorySize = vm["memory"].as<size_t>();
    if (verbose)
        cout << "Memory size: " << memorySize << endl;

    char initValue = vm["init"].as<char>();
    if (verbose)
        cout << "Initial value: " << (int) initValue << endl;

    unsigned constValue = vm["const"].as<unsigned>();

    bool useStdin = static_cast<bool>(vm.count("stdin"));
    vector<unsigned char> constInput;
    if (useStdin)
        for (auto str : vm["stdin"].as<vector<string>>())
            constInput.insert(constInput.begin(), (unsigned char) atoi(str.c_str()));

    std::string code;
    {
        ifstream sourcefile(vm["input"].as<string>());

        if (!sourcefile.is_open()) {
            cerr << "Could not open source file " << vm["input"].as<string>();
            return EXIT_FAILURE;
        } else if (verbose)
            cout << "Source file: " << vm["input"].as<string>() << endl;

        std::string line;
        while (std::getline(sourcefile, line)) {
            code += line;
            // todo: optimization
        }
    }

    if (verbose)
        cout << "running " << vm["input"].as<string>() << " ..." << endl;

    struct {
        unsigned char *ptrBegin, *ptrEnd, *ptr, *maximumUsage;
        const char *pcBegin, *pcEnd, *pc;

        void dump() {
            cerr << "ptr: 0x" << hex << uppercase << ptr - ptrBegin << endl;
            cerr << "pc: 0x" << hex << uppercase << pc - pcBegin << endl;
            cerr << "usage: 0x" << hex << uppercase << maximumUsage - ptrBegin << endl;
            const int line = 16;
            for (auto i = ptrBegin; i < ptrEnd; i += line) {
                cerr << setfill('0') << setw(static_cast<int>(to_string(ptrEnd - ptrBegin).size()));
                cerr << hex << uppercase << i - ptrBegin << " |";
                for (auto j = i; j < i + line && j < ptrEnd; j++) {
                    cerr << (j == ptr ? '>' : ' ') << setw(2) << setfill('0') << hex << uppercase << (unsigned)*j;
                }
                cerr << " |";
                for (auto j = i; j < i + line && j < ptrEnd; j++) {
                    cerr << (*j < 0x20 || *j > 0x7E ? '.' : (char) *j);
                }
                cerr << endl;
            }
        }
    } state{};

    state.maximumUsage = state.ptrBegin = state.ptr = new unsigned char[memorySize];
    state.ptrEnd = state.ptrBegin + memorySize;
    std::fill(state.ptr, state.ptr + memorySize, initValue);

    state.pcBegin = state.pc = code.c_str();
    state.pcEnd = state.pcBegin + code.size();

    while (state.pc != state.pcEnd && state.pc >= state.pcBegin) {
        bool debugInstructionEncountered = debug && *state.pc == debugInstruction;
        // write the interpreter state to cout
        if (debugInstructionEncountered) {
            state.dump();
        }
        // interrupt if debug instruction handler is 'interrupt' or if a breakpoint is encountered
        if ((debugInstructionEncountered && debugi) ||
            (useBreakpoints && find(breakpoints.begin(), breakpoints.end(), state.pc - state.pcBegin) != breakpoints.end())) {
            cout << "Breakpoint at " << state.pc - state.pcBegin << " hit" << endl;
            state.dump();
            cerr << "Press enter to continue...";
            cin.get();
        }
        switch (*state.pc) {
            case '+': ++*state.ptr; break;
            case '-': --*state.ptr; break;
            case '>':
                ++state.ptr;
                if (state.ptr >= state.ptrEnd)
                    throw std::runtime_error("pointer overflow at " + to_string(state.pc - state.pcBegin));
                if (state.ptr > state.maximumUsage)
                    state.maximumUsage = state.ptr;
                break;
            case '<':
                --state.ptr;
                if (state.ptr < state.ptrBegin)
                    throw std::runtime_error("pointer underflow at " + to_string(state.pc - state.pcBegin));
                break;
            case '.':
                if (numericalOutput)
                    cout << (unsigned) *state.ptr;
                else
                    cout << *state.ptr;
                break;
            case ',':
                if (useStdin) {
                    if (!constInput.empty()) {
                        *state.ptr = (unsigned char) constInput.back();
                        constInput.pop_back();
                    } else
                        *state.ptr = static_cast<unsigned char>(constValue);
                } else {
                    if (numericalInput) {
                        std::string line;
                        if (std::getline(cin, line))
                            *state.ptr = static_cast<unsigned char>(atoi(line.c_str()));
                    } else
                        cin >> *state.ptr;
                } break;
            case '[':
            if (*state.ptr == 0) {
                int depth = 0;
                do {
                    if (*state.pc == '[')
                        ++depth;
                    else if (*state.pc == ']')
                        --depth;
                    ++state.pc;
                    if (state.pc >= state.pcEnd)
                        throw std::runtime_error("program counter overflow");
                } while (depth != 0);
                --state.pc;
            } break;
            case ']':
            if (*state.ptr != 0) {
                int depth = 0;
                do {
                    if (*state.pc == '[')
                        ++depth;
                    else if (*state.pc == ']')
                        --depth;
                    --state.pc;
                    if (state.pc >= state.pcEnd)
                        throw std::runtime_error("program counter underflow");
                } while (depth != 0);
                ++state.pc;
            } break;
            case '@':
                if (verbose)
                    cout << "Exit instruction encountered" << endl;
                exit(EXIT_SUCCESS);
            default:
                break;
        }
        ++state.pc;
    }
}