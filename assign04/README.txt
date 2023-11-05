The classes that I modified the most for this milestone is the two ASTvisitor subclasses,
local storage allocation and highlevel-codegen. In both of them, I have overwritten 
necessary functions to ensure that the visitor is able to do its specific objective, such
as for local storage allocation assigning registers and offsets, or in high-level
code-gen actually appending high-level assembly instructions to the instruction
sequence.

To ensure the functionality of the two AST visitors, I also added new private / helper
functions in node-base, symbol and type classes. For type I added a helper function
to get the offset of each struct component, for symbol and node-base I added places to
store their respective virtual register or stack offset number.

