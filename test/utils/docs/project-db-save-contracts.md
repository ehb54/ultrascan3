# Project DB-save caller coverage

`test_us_project_db_writes.cpp` exercises the real `US_Project::saveToDB` and
`saveToDB_auto` methods with the existing strict interaction mock and normal
settings/file sandbox. No live database or new seam is required.

Both entry points share the implementation and have these contracts:

- Creation/update sends every field in stored-procedure order. Creation uses the
  configured or explicit investigator; neither path changes the configured ID.
- Query errors stop before cursor/write calls. A failed cursor maps OK/NOROWS to
  NO_PROJECT and propagates other errors, without reading values or writing.
- Missing, malformed or nonpositive looked-up IDs stop before update.
- Failed update/insert returns the write status. Failed insert never fetches a
  potentially stale last-insert ID. A missing inserted ID returns NO_PROJECT.
- A failed DB attempt preserves the previous project ID and does not promote
  saveStatus to BOTH. After a successful disk save it remains HD_ONLY, including
  when the object previously had DB_ONLY/BOTH status.
- Successful DB operations publish the confirmed ID and set BOTH.

These regressions address #1237. The separate `saveToDisk()` void-return limitation
remains: the caller cannot reliably establish disk-save success when that operation
fails. Broader disk/DB save-status policy, disk error reporting and rollback are
outside this change.
