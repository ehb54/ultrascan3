/**
 * @name Unused function
 * @description Finds functions in the codebase that are defined but never called.
 * @kind problem
 * @problem.severity warning
 * @id cpp/unused-function
 * @tags performance, maintainability
 */
import cpp

from Function func
where exists(Location loc | func.getLocation() = loc) and
      not exists(Call call | call.getTarget() = func)
select func, "This function is never used."
