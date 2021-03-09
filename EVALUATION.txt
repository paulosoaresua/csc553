README

# Peephole Optimizations

1. **Removing jump to next instruction.**

   The last boolean expression term of a conditional in a loop, always generate instructions like

   ```assembly
   goto _L1
   _L1:
   ...
   ```

   This is because if any of the terms evaluate to false, the program must leave the loop.

   **Solution:** Unconditional jump instructions are removed whenever its destiny label is the next instruction to be executed.

   **Statistics:** 1 less branch per loop instruction.



2. **Removing jump after conditionals. **

   Consider the following set of instructions typically generated for evaluation of conditionals.

   ```assembly
   # Before optimization
   if x < 5 goto _L0
   goto _L1
   _L0:
   ```

   We can get rid of the jump in line 3 by changing the boolean operator in line 2.

   **Solution:** Whenever the pattern above is found, the boolean operator of the conditional jump is changed to its inverse, and its label is replaced by the one in the subsequent unconditional jump.

   ```assembly
   # After optimization
   if x >= 5 goto _L0
   _L0:
   ```

   **Statistics:** 1 less branch per conditional instruction.



3. **Copying constants to final variable.**

   Consider the following set of instructions typically generated for constant assignments.

   ```assembly
   # Before optimization
   _tmp0 = 1
   x = _tmp0
   ```

   We can get read of the two intermediary instructions by setting 1 directly into the address of x.

   **Solution:** Whenever a constant integer/char is assigned to a temporary variable and this variable is immediately assigned to x, the constant is saved directly in x instead. Note: cases not involving temporaries are handled by the intra-block copy propagation optimization method.

   ```assembly
   # After optimization
   x = 1
   ```

   **Statistics:** 1 less read and write per constant assignment instruction.