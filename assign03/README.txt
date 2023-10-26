I mostly followed the starter code and implemented everything in the semantic analysis class. For each visit 
helper function, I iterate through all the kinds of operations and annotated any node that is a variable reference
with its symbol pointer, and any node that represent a value (both lvalue and rvalue) with its specific type. The only
exception is that for the variable declarator, since it is previously annotated with its type value when running through
visitBasicType helper function, it cannot be annotated again with symbol pointer. 

To ensure no data leak, on leaving the scope, the scope autimatically gets deleted. And in the SemanticAnalysis descructor, 
the global scope is deleted.