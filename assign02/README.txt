Milestone One:
I first added the required new TOKEN and AST nodes in lexer and parse2 part. 

To ensure that both if/else statements and intrinsic functions would work in the current system,
I completed the implementation for the environment, valrep, and value classes.

I am able to retain a big part of my code structure as the last assignment. Particularly, 
the majority of the hierachical flow for both the analyze and execute part is retained the 
same: 

unit-level --> statement-level --> low-level

The only two change I have made are for the if/else statement and intrinsic functions.
For the first change, I split the breakdown of children in statement-level so that
it can either go to an if/else or while-level, where the new environment necessary will be
created, and from there it gets then passe again back to statement-level. For the second change, I 
add one more execution at low-level breakdown : funcall which deals with whenever an intrinsic
functional call happens. I also need to add a helper function called "preinstall_intrinsic_functions"
which helps install all intrinsic functions at global environment so that it can access everywhere.

THe overall flow now looks like:

unit-level (preinstall intrinsic funcs) --> statement-level  <--> (optional: if / while-level)
                                                    --> low-level

Milestone Two:

I first added the required new TOKEN and AST nodes in lexer and parse2 part for the new String datatype, 
lambda and normal function.

For this milestone I mostly used the same implementation as before. The biggest changes compared 
to last milestone would be I converted all environment to VarRep data type which then get wrapped
in a Value wrapper, and the addition of reference counting to keep track of all the dynamic valreps
inside Value datatype.

I implemented Str and Arr as respective new classes with their separate header and cpp files, then
added them to the compilation in Makefile. 

For Str and Arr, I just added their respective intrinsic functions and use the same pre-install
intrinsics funcs helper function to install them in global environment. For function, however, 
I need to split the unit level passing down to children to function-level and statement-level,
so that the function can be processed and the its statementlist again passed to statement-level.
Becuase of the existence of lambda functions, I also need to check the situation of function-level
appearing in low-level execution. I also added funcall at low-level execution so that it deals with
situation whenever an already-declared function is called later.

The full flow now looks like:

unit-level (instrinsic) --> function-level
               |-->             |--> statement-level  <--> (optional: if / while-level)
                                            |--> low-level