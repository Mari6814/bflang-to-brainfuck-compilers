package brainfuck.program;

/**
 * Types are a collection of other types
 * Created by Marian on 21/08/2016.
 */
public class Type extends SymbolTable {
    private String name;

    public Type(String name, SymbolTable scope) {
        super(scope);
        this.name = name;
    }

    /**
     * The name of the type
     * @return The local name of the type
     */
    public String getName() {
        return name;
    }

    @Override
    public int getSizeOnStack() {
        if (name.equals(Program.NATIVE_TYPE_NAME))
            return 1;
        return super.getSizeOnStack();
    }

    @Override
    public String toString() {
        return "Type name=" + name + " sizeOnStack=" + getSizeOnStack();
    }
}
