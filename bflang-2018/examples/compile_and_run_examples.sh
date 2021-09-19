#/bin/bash
EXAMPLE_DIR="."
BFC="../cmake-build-release/bfc"
BFI="../cmake-build-release/bfi"
DST_DIR="$EXAMPLE_DIR/dst"
SOURCE_FILES="characters.bl functions.bl io.bl member_functions.bl recursion.bl scopes.bl types.bl"
mkdir $DST_DIR
echo "Compiling examples from $EXAMPLE_DIR with compiler=$BFC and interpreter=$BFI"

for SRC_FILE in $SOURCE_FILES; do
    # Compile the source file
    echo "$BFC $SRC_FILE -I $EXAMPLE_DIR -o ${DST_DIR}/${SRC_FILE}.b"
    $BFC $SRC_FILE -I $EXAMPLE_DIR -o ${DST_DIR}/${SRC_FILE}.b

    # Run the source file
    echo "$BFI $DST_DIR/${SRC_FILE}.b"
    $BFI $DST_DIR/${SRC_FILE}.b

    echo;
done

# Compile multuple files example
echo "$BFC -i math.bl multiple_files_compiled.bl -I $EXAMPLE_DIR -o ${DST_DIR}/multiple_files_compiled.b"
$BFC -i math.bl multiple_files_compiled.bl -I $EXAMPLE_DIR -o ${DST_DIR}/multiple_files_compiled.b

# Run multiple files example
echo "$BFI $DST_DIR/multiple_files_compiled.b"
$BFI $DST_DIR/multiple_files_compiled.b