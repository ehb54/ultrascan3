/**
 * @name Unused function
 * @description Finds unused functions in C/C++ code.
 * @kind problem
 * @problem.severity warning
 */
import cpp

from Function func
where not func.isExtern() and
      not func.isEntryPoint() and
      func.getNumberOfCallers() = 0
select func, "This function is never used."
