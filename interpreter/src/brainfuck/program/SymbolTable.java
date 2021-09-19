package brainfuck.program;

import com.sun.istack.internal.Nullable;

import java.util.*;

import static java.util.Arrays.spliterator;

/**
 * The Symbol table records the delta jump distances from this table to the table of one of its records
 * Created by Marian on 21/08/2016.
 */
public class SymbolTable {
    private SymbolTable parentScope;
    private final List<Type> types = new ArrayList<>();
    private final List<Variable> localVariables = new ArrayList<>();
    private final List<Method> methods;

    public SymbolTable(SymbolTable parent) {
        this.parentScope = parent;
        if (parent == null) {
            methods = new ArrayList<>();
            defineType("cell");
        } else methods = null;
    }

    /**
     * Adds a new variable
     * @param var Variable to add
     * @return this
     */
    public SymbolTable defineVariable(Variable var) {
        if (localVariables.stream().anyMatch(variable -> variable.getName().equals(var.getName())))
            throw new IllegalArgumentException("Multiple definitions vor variable " + var.getType());
        localVariables.add(var);
        return this;
    }

    /**
     * Defines a local variable
     * @param name Variable name
     * @param qualifiedTypeName Variable type
     * @return this
     */
    public SymbolTable defineVariable(String name, String qualifiedTypeName) {
        return defineVariable(new Variable(name, getType(Util.splitPath(qualifiedTypeName))));
    }

    /**
     * Defines an array of variables
     * The created variables are named like: name[0..length-1]
     * @param name Name of the array
     * @param qualifiedTypeName Type of the array
     * @param length Object count
     * @return this
     */
    public SymbolTable defineVariable(String name, String qualifiedTypeName, int length) {
        if (!isTypeDefined(Util.splitPath(qualifiedTypeName + "[" + length + "]")))
            defineType(qualifiedTypeName, length);
        return defineVariable(name, getType(Util.splitPath(qualifiedTypeName)).getName() + "[" + length + "]");
    }

    /**
     * Defines a variable of type cell
     * @param name Name of the cell
     * @return this
     */
    public SymbolTable defineVariable(String name) {
        return defineVariable(name, "cell");
    }

    /**
     * Gets a local variable
     * If no local is defined the global table will be searched
     * @param qualifiedVariableName Qualified name to the variable
     * @return Variable
     */
    public Variable getVariable(List<String> qualifiedVariableName) {
        for (Variable v : localVariables) {
            if (v.getName().equals(qualifiedVariableName.get(0))) {
                if (qualifiedVariableName.size() == 1)
                    return v;
                return v.getType().getVariable(qualifiedVariableName.subList(1, qualifiedVariableName.size()));
            }
        }
        throw new IllegalArgumentException("No variable " + qualifiedVariableName.get(0) + " defined in this scope");
    }

    /**
     * Gets the position of the variable
     * @param qualifiedVariableName Name of the variable
     * @return offset
     */
    public int getVariablePosition(List<String> qualifiedVariableName) {
        int sum = 0;
        for (Variable v : localVariables) {
            if (v.getName().equals(qualifiedVariableName.get(0))) {
                if (qualifiedVariableName.size() == 1)
                    return sum;
                return sum + v.getType().getVariablePosition(qualifiedVariableName.subList(1, qualifiedVariableName.size()));
            }
            sum += v.getType().getSizeOnStack();
        }
        throw new IllegalArgumentException("No variable " + qualifiedVariableName.get(0) + " defined in this scope");
    }

    /**
     * Undefines a local variable
     * @param localVariable Name of the variable to undefine
     * @return this
     */
    public SymbolTable undefineVariable(String localVariable) {
        localVariables.remove(getVariable(Util.splitPath(localVariable)));
        return this;
    }

    /**
     * Defines a type in this scope
     * @param newLocalTypeName Name of the new type
     * @return Created type
     */
    public Type defineType(String newLocalTypeName) {
        if (types.stream().anyMatch(type -> type.getName().equals(newLocalTypeName)))
            throw new IllegalArgumentException("Multiple type definitions of type " + newLocalTypeName);
        Type type = new Type(newLocalTypeName, this);
        types.add(type);
        return type;
    }

    /**
     * Defines an constant length array type of an already existing type
     * @param qualifiedTypeName Qualified named of the existing type
     * @param length Length of the array
     * @return Type of the form type[length]
     */
    public Type defineType(String qualifiedTypeName, int length) {
        Type type = getType(Util.splitPath(qualifiedTypeName));
        String baseTypeName = type.getName();
        type = type.getParentScope().defineType(baseTypeName + "[" + length + "]");
        for (int i = 0; i < length; i++)
            type.defineVariable(String.valueOf(i), baseTypeName);
        return type;
    }

    /**
     * Gets a type from this or any parent symbol table
     * @param qualifiedTypeName Qualified name to the searched type
     * @return Type
     */
    public Type getType(List<String> qualifiedTypeName) {
        for (Type t : types) {
            if (t.getName().equals(qualifiedTypeName.get(0))) {
                if (qualifiedTypeName.size() == 1)
                    return t;
                return t.getType(qualifiedTypeName.subList(1, qualifiedTypeName.size()));
            }
        }

        if (null != parentScope)
            return parentScope.getType(qualifiedTypeName);
        throw new IllegalArgumentException("No type '" + qualifiedTypeName.get(0) + "' defined");
    }

    /**
     * Checks if a type is defined in this scope
     * @param qualifiedTypeName Type to check
     * @return True if the type is defined in this or a parent scope, false otherwise
     */
    public boolean isTypeDefined(List<String> qualifiedTypeName) {
        for (Type t : types) {
            if (t.getName().equals(qualifiedTypeName.get(0))) {
                if (qualifiedTypeName.size() == 1)
                    return true;
                return t.isTypeDefined(qualifiedTypeName.subList(1, qualifiedTypeName.size()));
            }
        }

        if (null != parentScope)
            return parentScope.isTypeDefined(qualifiedTypeName);
        return false;
    }

    /**
     * Defines a new jumpable label
     * @param name Name of the method
     * @param returnTypes Expected names of the return type
     * @return Created method object
     */
    public Method defineMethod(String name, @Nullable List<String> returnTypes, @Nullable List<String> paramTypes, @Nullable List<String> paramNames) {
        if (parentScope != null)
            return parentScope.defineMethod(name, returnTypes, paramTypes, paramNames);
        if (returnTypes == null)
            returnTypes = new ArrayList<>();
        Method m = new Method(this, name, returnTypes, paramTypes, paramNames);
        methods.add(m);
        return m;
    }

    /**
     * Gets the specified method object
     * @param name Name of the method to get
     * @return Method
     */
    public Method getMethod(String name) {
        if (parentScope != null)
            return parentScope.getMethod(name);
        for (Method m : methods)
            if (m.getName().equals(name))
                return m;
        throw new IllegalArgumentException("No method " + name + " is defined in this scope");
    }

    /**
     * Gets a list of all defined methods
     * @return List of defined methods
     */
    public List<Method> getMethods() {
        return parentScope == null ? methods : parentScope.getMethods();
    }

    /**
     * Gets the next unused label address
     * @return New unique jumpable address
     */
    public int getNewLabelName() {
        if (parentScope != null)
            return parentScope.getNewLabelName();
        return 1 + getMethods().stream().mapToInt(value -> value.getLabels().size()).sum();
    }

    /**
     * @return Number of cells used on the stack by this symbol table
     */
    public int getSizeOnStack() {
        int size = 0;
        for (Variable v : localVariables)
            size += v.getType().getSizeOnStack();
        return size;
    }

    /**
     * @return Parent scope
     */
    public SymbolTable getParentScope() {
        return parentScope;
    }

    public int getMethodAddress(String method) {
        for (int i = 0, adr = 1; i < getMethods().size(); i++) {
            if (getMethods().get(i).getName().equals(method))
                return getMethods().get(i).getLabels().get(0).getLabelName();
        }
        throw new IllegalArgumentException("Method " + method + " not defined");
    }
}