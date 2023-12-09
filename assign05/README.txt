MileStone 1

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

MileStone 2

For MileStone 2 I mostly only implemented the lowlevel_codegen class, with some minor
edits to the original highlevel_codegen class when there are specifics of how the
highlevel sequence is created which makes lowlevel codegen relatively harder. 

For the lowlevel codegen, most of my changes are in translate_hl_to_ll function, where I 
determined the storage allocation for memory storage and virtual register storage, and in 
translate_instruction, where I conditioned over each of the hl instructions and convert them
into corresponding low-level machine code. The hl-level opcode associated with longer translation 
procedures are moved into a private helper function so that the overall length of translate_instruction 
function is manageable.

