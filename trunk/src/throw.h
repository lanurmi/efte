/*
 meant to be use as named throw/catch condition eventually. for now, failing macros can
 use these to signal reason of Failure in a global, to allow OnFail handler to deal with condition

*/

#define STACKUNDERFLOW -4          // param stack underflow
#define RETURNSTACKUNDERFLOW -6    // return stack underflow
#define DICTOVERFLOW         -8    // dictionary overflow
#define INVALIDADDR          -9    // invalid memory address
#define DIVZERO             -10    // division by zero
#define OUTOFRANGE          -11    // result out of range
#define NOTFOUND            -13    // Undefined word
#define COMPILEONLY         -14    // Interpreting a compile-only word
#define ZERONAME            -16    // Attempt to use zero-length string as a name
#define PICNUMOVERFLOW      -17    // Pictured numeric ouput string overflow
#define PARSEOVERFLOW       -18    // Parsed string overflow
#define NAMETOOLONG         -19    // Word name too long
#define READONLY            -20    // Write to a read-only location
#define UNSUPPORTED         -21    // Unsupported operation
#define BADCONTROL          -22    // Control structure mismatch
#define BADNUM              -24    // Invalid numeric argument
#define CONTROLUNBAL        -25    // Control stack imbalance
#define USERINTERRUPT       -28    // User interrupt
#define BADBODY             -31    // >BODY used on non-CREATEd definition
#define INVALIDNAME         -32    // Invalid name argument
#define INVALIDBLOCK        -35    // Invalid block number
#define INVALIDFILEPOS      -36    // Invalid file position
#define FILENONEXISTING     -38    // Non-existent file
#define FILEUNEXPECTEDEND   -39    // Unexpected end of file











