package brainfuck.program;

/**
 * Created by Marian on 21/08/2016.
 */
public class Variable {
    // Name of the variable
    private String name;
    // Name of the type of the variable
    private Type type;

    /**
     * An array of a type
     * @param name Name of the array
     * @param type Type of the array
     */
    public Variable(String name, Type type) {
        this.name = name;
        this.type = type;
    }

    public String getName() {
        return name;
    }

    public Type getType() {
        return type;
    }

    @Override
    public String toString() {
        return "Variable name=" + name + " type=" + type.getName();
    }
}
