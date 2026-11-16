This overlay is copied from ports/qtbase at the vcpkg baseline in vcpkg.json
(a44707152fabbedcf1c17fe80d7bfb7d9370d8d5). Every copied file is byte-identical
to that port, including CRLF line endings, which buildsys/vcpkg/.gitattributes
keeps Git from converting. Two files are added: this README and
macos-accessibility-interface-ownership.patch.

portfile.cmake differs from the port only by two VCPKG_TARGET_IS_OSX blocks.
Neither changes the build on Windows or Linux, but vcpkg's package ABI covers
the port's files, so every target's toolchain is built from this overlay.

1. It applies macos-accessibility-interface-ownership.patch. The patch is the
   production-source change from upstream Qt commit
   8148c48deb8e4345bacb797cb75c011df6f797fa (QTBUG-149612), in both
   qcocoaaccessibilityelement.mm and qcocoaaccessibilityelement.h:
   https://github.com/qt/qtbase/commit/8148c48deb8e4345bacb797cb75c011df6f797fa

   Native Cocoa accessibility elements must not delete the accessible
   interfaces owned by Qt views and the accessibility cache. Doing so can leave
   the table with a stale interface and crash Assistant on an external
   accessibility query. Accessibility remains enabled. Remove this block, and
   the patch, when the pinned Qt includes the upstream fix.

2. It fixes cmake_automoc_parser's dynamic PCRE dependency on macOS. The
   Windows-only vcpkg_copy_tool_dependencies helper does not supply it on Mac.
   The port copies the exact linked PCRE library alongside the parser and
   changes its reference to @loader_path. It keeps the linker's ad-hoc
   signature, which install_name_tool keeps valid through vcpkg's later rpath
   fixup; a codesign signature is invalidated by that fixup with Xcode 15 and
   the parser is killed. This supports downstream builds from packages/ and
   relocated toolchains without an absolute build-machine path.

Validation: the patch applies to the Qt 6.10.1 sources. A local arm64 build
with the .mm change, packaged as a PKG and installed into an empty
/Applications/UltraScan3, loaded nothing outside the package or Apple system
paths, and Help displayed the manual without the accessibility crash.
