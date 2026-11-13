# Project DB-save caller coverage

`test_us_project_db_writes.cpp` calls the real `US_Project::saveToDB` and
`saveToDB_auto` methods with the existing strict interaction mock. Each case runs
inside the normal settings/file sandbox; no live database is involved.

Covered for both entry points:

- Creation sends every project field in the stored-procedure argument order and
  uses the configured investigator or explicit investigator as appropriate.
- Update uses the looked-up ID, sends all fields, and never requests an insert ID.
- Lookup DBERROR/NOT_CONNECTED stops before cursor access or writes and returns
  that error. A successful disk save remains HD_ONLY.
- A missing inserted ID returns NO_PROJECT rather than claiming a DB save.
- Explicit-investigator calls do not change the configured investigator.

## Remaining production gaps

Source review found that both methods ignore `next()` failure and the return value
of `statusQuery()`. Consequently, a failed update can report OK/BOTH, and a failed
insert can use a stale last-insert ID. These are not asserted as acceptable behavior
by this suite. Required follow-up: check cursor/write outcomes, forbid value/insert-ID
access after failure, and test failed update/insert paths through the same mock.

`saveToDisk()` also returns void, so DB-save callers cannot reliably distinguish a
failed disk save before claiming BOTH. Disk/DB save-status policy needs an explicit
contract before adding broader rollback or changing that API.
