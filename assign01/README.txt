For the lexer implementation, since the majority of the rough structure is given, I
only made minor adjustments to the large switch statement in read_token. To avoid 
repetition of code, I grouped all the single-digit operations, all the possibly 
singe-or-double-digit operations (< -> <=; > -> >=; = -> ==), and all the 
either-double-or-erroneous ones into three distinct function calls and created respective
std::map to associate the operator with its corresponding token name. 

For the parser implementation, since the core recursive functions are given, I mostly implemented
the other parse non-terminal functions according to the existing ones. So even for the non-infix
operators, I tried to get both LHS and RHS parsed and then reset the LHS unique pointer to the 
operator and assigned the original LHS and RHS root nodes as its two kids. 

For the interpreter implementation, I choose a similar structure for both analyze and execute 
functions. Specifically, I split the recursive interpretation process into three levels: the 
unit-level (which will only be called once at least in the current implementation), the 
statement-level (which will be called for as many as the number of lines in the code), and the 
remaining low-level. The unit level will iterate over all its statement kids, whereas the statement
will similarly iterate over all its current kids. the low-level function would recursively interpret 
the abstract symbol tree elements.

The unit level is going to create an environment variable whereas its pointer
would be passed into each statement-level and low-level function to ensure that the whole script 
shares the same variable environment. The environment is composed of a map of string (variable name) 
and Value class (only the KIND_INT and a self-implemented KIND_INVALID are used because currently
there are no atomic / dynamic function). There are three helper functions in the environment class,
namely look_up, define, and assign_int. Each function has a retuning value that indicates an error 
has occured to be passed back to the interpreter class so that the error can be thrown with specific
location and other relevant information. 
