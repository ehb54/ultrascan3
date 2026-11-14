This overlay is copied from ports/qtbase at the vcpkg baseline in vcpkg.json
(a44707152fabbedcf1c17fe80d7bfb7d9370d8d5).

The only portfile change applies macos-accessibility-interface-ownership.patch
on macOS. The patch is the production-source change from upstream Qt commit
8148c48deb8e4345bacb797cb75c011df6f797fa (QTBUG-149612):
https://github.com/qt/qtbase/commit/8148c48deb8e4345bacb797cb75c011df6f797fa

Native Cocoa accessibility elements must not delete the accessible interfaces
owned by Qt views and the accessibility cache. Doing so can leave the table
with a stale interface and crash Assistant on an external accessibility query.
Accessibility remains enabled. Remove this overlay when the pinned Qt includes
the upstream fix. Rebuild and repin both macOS toolchain archives before
validating application packages; existing pinned archives do not gain this fix.

Validation so far: patch applies cleanly to the local Qt 6.10.1 source;
git diff --check passes. Rebuilt-package runtime validation is still pending.

The overlay also fixes cmake_automoc_parser's dynamic PCRE dependency on macOS.
The Windows-only vcpkg_copy_tool_dependencies helper does not supply it on Mac.
The port copies the exact linked PCRE library alongside the parser and changes
its reference to @loader_path, then refreshes the executable's ad-hoc signature.
This supports downstream builds from packages/ and relocated toolchains without
an absolute build-machine path. A local Mach-O fixture reproduced failure with
only the original relative lib rpath, then passed with this CMake fixup both in
place and after relocation. Full production-toolchain validation remains pending.
