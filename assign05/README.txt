Most of the optimizations are completed in cfg_transform.cpp and peephole_ll.cpp. 

In the cfg_transform.cpp, I implemented two optimization helper classes:

   - LocalOptimizationHighLevel: 
            first round of high level optimization, iterative approach,
            uses local copy propagation, local value numbering. 
            Use a liveness analysis helper to check which values are
            necessary
   -  LocalMregAssignmentHighLevel:
            second round of high level optimization, assign all the virtual registers
            with a machine register

In peephole_ll.cpp, implemented one peephole optimization with a number of matches, each
corresponding to an instance of optimizable block.

I also modified context.cpp, highlevel_codegen.cpp, lowlevel_codegen.cpp and 
local_storage_allocation.cpp to support the optimization process.

