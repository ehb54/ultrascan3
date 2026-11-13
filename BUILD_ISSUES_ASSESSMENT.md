# UltraScan core build issues assessment

Fresh verification shows that these six reviewed tickets overstate the remaining implementation work. Several are answered or substantially implemented. Treat the dispositions below as the current backlog assessment; the broader architecture findings later in this document are separate improvement candidates, not reasons to keep an otherwise completed ticket open.

The review covers #546, #646, #679, #768, #869, and #1024; it does not assume all six need code changes. #937 remains with samo38 and is retained below only as a coordination reference; it is not a dependency or deliverable of this initiative. #942 is excluded. SOMO-specific implementation is excluded; #679 contributes only its documentation and Qt help requirements. Assessment date: November 13, 2026. Source baseline: main commit `1f838727ffaa9549db62884e0504acf2f94b3eac`.

# Ticket revalidation — November 13, 2026

At the initial revalidation, GitHub reported all six tickets as **OPEN**. The user subsequently confirmed that **#768 is closed with a comment**; it has no remaining work in this initiative. Issue bodies, comments, current source, published assets, and build logs were checked. Open status alone does not establish outstanding implementation work.

| Ticket | Verified finding | Recommended disposition |
|---|---|---|
| [#546](https://github.com/ehb54/ultrascan-tickets/issues/546) | Qt deployment and macOS PKG packaging exist. Published build logs show successful arm64 and x86_64 PKG creation and help-file staging. The original checklist asks for DMG. | Largely implemented; reconcile PKG versus DMG and confirm clean-machine install/help behavior, then close or retain only specific failures. |
| [#646](https://github.com/ehb54/ultrascan-tickets/issues/646) | The reported time-dependent revision generation was changed to commit time in December 2025. Current qmake generator preserves an unchanged header. An isolated two-invocation check confirmed identical content and unchanged modification time. | Likely addressed; the user is testing #646 in another session. Await that session's two-successive-qmake-build evidence before closure. The generator check here is not a full incremental-build test. |
| [#679](https://github.com/ehb54/ultrascan-tickets/issues/679) | Modern packages build and stage Qt help. June 2026 makeall.sh changes improved Python/Sphinx detection; its manual target generates both help files. However, make_all.pro still lacks a documentation target, and makeall.sh only prints the detected Assistant invocation rather than providing the originally requested setup. | Partially addressed. Retain a narrow legacy-entrypoint requirement if that workflow remains supported; explicitly retire it otherwise. SOMO remains outside this analysis and must be separated before closing the original mixed ticket. |
| ~~[#768](https://github.com/ehb54/ultrascan-tickets/issues/768)~~ **Closed** | The original question asks where the application version is stored. A March 4 comment answers utils/us_defines.h; current CMake also checks its agreement with VERSION. | **Closed with a comment**, as confirmed by the user. Broader metadata consistency ideas are separate work, not an expansion of this question. |
| [#869](https://github.com/ehb54/ultrascan-tickets/issues/869) | **Fixed locally.** README and archive naming share the calculated filename; x64/arm64 archive fixtures passed. | Implementation complete; delivery and ticket closure remain. |
| [#1024](https://github.com/ehb54/ultrascan-tickets/issues/1024) | On-demand source now constructs version/run/hash/platform names and uploads staged files without the preset-directory nesting. The shared Windows packaging path explicitly validates Assistant, manual.qch, manual.qhc and qsqlite.dll; the published nightly log confirms that check passed. Linux APP logs also show both manuals staged and real Git metadata generated. | Original fixes appear implemented. Verify one on-demand artifact and installed help before closure; do not add unrelated architecture work to this ticket. |

Build evidence: [November 11 published nightly run 43027179129](https://github.com/ehb54/ultrascan3/actions/runs/43027179129), source `0eee7f31f9aa28056ec6241b4fe70b0eca506420`, and [nightly release assets](https://github.com/ehb54/ultrascan3/releases/tag/nightly). This is the latest published package set inspected, not a package build of current main. Successful November 12/13 workflow status alone is not evidence of a newer published package. The on-demand workflow API returned zero retained runs, so its naming and nesting fixes are source-verified, not verified from a recent on-demand artifact. No downloaded installer was executed during this review.

The #646 fixture used a temporary Git repository containing the current revision.sh. After first generation, the header timestamp was set to a known old value; a second invocation preserved both that timestamp and all bytes. This checks the specific generator behavior without changing the working checkout or claiming that all qmake dependency rules are correct.

No tickets were changed by this review. The #869 fix is now implemented locally and awaits delivery; #679 still needs a decision on supported legacy entry points. #768 has since been closed with a comment; #546, #646 and #1024 primarily need bounded acceptance checks. The APP-only static-library optimization documented later is a separate improvement and does not itself close any of these tickets.

# #869 local implementation

Implemented after the ticket revalidation above; implemented on `build-issues`, not yet merged or published. Linux APP staging now receives the complete archive filename from the same `LinuxPackageName.cmake` calculation used by `LinuxPkg.cmake`. Standalone deployment uses that helper as a fallback. Existing architecture naming and override behavior are preserved. The README remains useful installation guidance, with its literal `<arch>` replaced by the calculated filename. No SOMO behavior was changed.

Validation: temporary CMake fixture projects ran the actual deployment and archive modules for explicit `x64` and `arm64` labels. Both produced tar.xz archives whose embedded README named the actual archive, contained no `<arch>`, and documented a launcher path present in the archive. Fixtures used dummy application/help files and a stub dependency scanner on the macOS host; this validates README/archive integration, not Linux binary compatibility or a complete application build. `git diff --check` passed for the changed build files. Fixture evidence: `/private/tmp/us3-869-phx5nlxi`.

This fixes the local CMake archive name covered by #869. CI may subsequently rename published downloads; those downstream release aliases are outside this change. The GitHub issue remains open pending delivery.

# #1024 package selection and runtime verification — November 13, 2026

The shared action's Stage package step did select the first profile-matching output with `head -n 1`, despite claiming to require exactly one package. It now rejects zero or multiple candidates, prints ambiguous candidates, and copies only a unique match. Existing APP/HPC filtering and supported extensions are preserved. This change is implemented on `build-issues` and awaits merge/publication.

`python3 test/packaging/test_stage_package.py` passes four test methods exercising the actual inline action script under macOS Bash 3.2: zero candidates, one candidate for each of tar.xz/tar.gz/pkg/exe (including a directory containing spaces), multiple APP candidates, and APP/HPC filtering including zero and multiple HPC matches. Failed selection leaves no staged files. `git diff --check` passes. These fixtures do not run a full CI build or upload.

No retained on-demand runs were available. The November 13 nightly run 43435477779 has no artifacts; the newest published packages inspected remain from run 43027179129, source `0eee7f31f9aa28056ec6241b4fe70b0eca506420`, November 11. Workflow-artifact download stalled and was stopped; release assets were downloaded successfully. Consequently, this check does not establish the on-demand ZIP layout or validate a fresh build of the local changes.

Downloaded release assets:

- `us3-redhat-8.10-x86_64-nightly-20261111-0eee7f3-43027179129-1.tar.xz`; SHA-256 `315dbeca19e6c1026b01b493fa2a730730c0bd2aa488f5d6cdb6e8eb0285fbe5`.
- `us3-Windows-x86_64-nightly-20261111-0eee7f3-43027179129-1-setup.exe`; SHA-256 `5a3733ca6964c6bef7930748d127e13ffa1da0efc323c951077cad4235f69ae3`.

Both payloads contain the application, help daemon, Assistant, manual.qch, manual.qhc, and the SQLite Qt plugin. Both help databases pass SQLite integrity checks. The RHEL archive was extracted and its Assistant and help daemon launched with the bundled library/plugin paths in an isolated Ubuntu 22.04 x86_64 container using Qt's offscreen platform. Both remained running until the 10-second timeout; this is a launch smoke test, not native RHEL 8 acceptance or proof of displayed help content.

The Windows installer completed with exit code 0 in the Windows 11 VM, installing into `C:\Temp\us3-1024\UltraScan3`. Launching the installed help daemon in the interactive desktop produced an Assistant error: **Cannot load sqlite database driver**. The installer contains `plugins/sqldrivers/qsqlite.dll`, whose PE import table requires `sqlite3.dll`; that dependency is absent from the extracted installer payload. Required-file presence checks therefore do not establish functioning help. This is a reproduced installed-help failure, though the test launched the daemon directly rather than navigating the main application's Help menu.

A second defect is visible in both packages: their registered help namespace is `com.aucsolutions.manual.v4.3.0-dev-9950-0eee7f3`, but the help daemon requests `qthelp://ultrascaniii/manual/index.html`. The Linux daemon log confirms that request. Current source likewise constructs the versioned namespace in `doc/manual/source/conf.py` while hard-coding `ultrascaniii` in `programs/us_helpdaemon/us_helpdaemon.cpp`. Resolve this mismatch and verify actual page navigation after repairing Windows plugin dependencies.

Evidence is retained under `/private/tmp/us3-1024-release`, including extracted payloads, `windows-extract.log`, and `windows-help-visible.png`. The Windows test installation is retained for follow-up. No ticket was changed or closed. Next work: repair and validate the Windows SQLite runtime dependency, reconcile the help namespace, then produce fresh on-demand packages and verify ZIP layout and application-driven help on the intended platforms.

# Help defect fixes — November 13, 2026

Both defects reproduced above now have local fixes, superseding the repair work listed as next steps in the preceding section. They are implemented on `build-issues`, not yet merged or published.

- Windows: `deploy_windows` now passes the active target triplet's vcpkg `bin` directory to `WinDeploy.cmake`. The deployment script already copies non-Qt runtime DLLs from that directory; the missing argument prevented that path from supplying `sqlite3.dll`. Final stage validation now requires a nonempty `sqlite3.dll` alongside Assistant and the other help components. It does not search the host triplet for a substitute runtime.
- Help URLs: `doc/manual/source/conf.py` now generates the stable `ultrascaniii` namespace expected by the existing help daemon. Documentation version/build metadata remains visible in the manual. No application binary changes or new Qt module dependencies are needed.

Validation:

- All seven packaging test methods pass (`python3 -m unittest discover -s test/packaging -p 'test_*.py'`). The three new Windows tests run the real deployment script with fixture executables, a no-op Qt deployer, and suppressed unrelated downloads. They verify copying the supplied SQLite runtime and rejection of missing or empty runtimes. These are deployment logic tests, not Windows binary-load tests.
- Full Sphinx qthelp generation succeeded with the changed configuration, followed by Qt 5 qhelpgenerator compilation of manual.qch/manual.qhc. A compiled QHelpEngineCore probe resolves the daemon's URL for `index.html`, `manual/index.html`, and an anchored index URL to nonempty HTML.
- The retained Windows test installation was supplied with those regenerated manuals and the x64 SQLite runtime from `C:\Users\me3\vcpkg\installed\x64-windows\bin\sqlite3.dll`. Its existing help daemon launched its existing Qt 6 Assistant and visibly displayed the documentation homepage at `qthelp://ultrascaniii/manual/index.html`; the previous SQLite error disappeared. Screenshot: `/private/tmp/us3-help-fix/windows-help-fixed.png`.
- `git diff --check` passes. Build logs, generated help, and the Qt probe are retained under `/private/tmp/us3-help-fix` (Sphinx and qhelpgenerator logs use the adjacent `us3-help-fix-*.log` filenames).

Limits: the Windows runtime test repaired the dedicated installation of the November 11 package; it did not build or install a fresh installer from these edits. It launched the help daemon directly, not the main application's Help menu. The fixture does not validate native windeployqt execution. Fresh CI packaging, final installer inspection, and application-driven navigation on the intended platforms remain the delivery gate for #1024.

# APP package dependency completeness — November 13, 2026

Implemented locally after the help fixes above; implemented on `build-issues`, not yet merged or published. This supersedes the earlier baseline finding that APP deployment depends mainly on `us` and discovers companion executables by globbing the build directory.

The existing per-platform program selection loop now registers its actual executable targets. A shared package-input target depends on that complete inventory plus documentation. A configuration-specific manifest records target output files and macOS bundle directories using CMake generator expressions, preserving renamed executables and multi-configuration output paths. Windows, Linux, and macOS deployment consume the same inventory and reject missing or empty executable outputs. Core executable staging no longer scans the build output directory.

Developer builds may still omit documentation, but invoking an APP package target without Sphinx/qhelpgenerator-backed documentation fails explicitly. Final help validation requires nonempty manuals, Assistant, and the platform SQLite plugin (plus Windows sqlite3.dll). Linux/macOS stages now also reset themselves on standalone deployment. Optional SOMO source-bin imports on Windows/macOS are restricted to their executable naming patterns so stale core programs cannot reenter through that separate path; no SOMO targets or dependencies were added. HPC packaging is unchanged.

Validation:

- All 14 packaging test methods pass. Clean fixture builds invoke only a package target, compile their executable dependencies, generate help through the documentation dependency, and inspect the resulting archive. They exercise the real Linux/Windows/macOS deployment scripts with stub Qt deployment tools. Linux uses the real `package_linux_tarball`; Windows/macOS use fixture tar archives instead of native installer tools.
- Archive checks cover the main program, help daemon, renamed companion output, manuals, Assistant, and exclusion of stale core files placed in both build and legacy SOMO input directories. macOS tests cover both native bundles and wrapping bare companions. A Ninja Multi-Config Release build verifies configuration-specific target paths.
- Negative cases cover unavailable documentation, missing help output, missing Assistant, missing declared executable, and missing/empty Windows SQLite runtime. The original artifact-selection tests remain passing.
- The actual program-selection loop, exercised with minimal target declarations, selects 70 non-MPI APP executables on Linux/Windows, 62 on macOS, and 71 when MPI is enabled on Linux. It preserves target names that differ from their directory names.
- An isolated archive of the real source configured successfully as Linux x86_64 APP/Qt 5.15.3. Its generated Makefile graph confirms `package_linux_tarball -> deploy_linux -> us3_app_package_inputs -> 70 program targets + documentation`. Sphinx is absent in that container, so the dependency-enabled configuration used `/usr/bin/true` as a Sphinx stand-in; this verifies the generated graph only. The ordinary configuration exposes the intended package-time failure for missing documentation.
- `git diff --check` passes. Evidence: `/private/tmp/us3-package-graph`, `/private/tmp/us3-package-fixtures.log`, and `/private/tmp/us3-package-all-tests.log`.

Full Linux acceptance is in progress: a fresh Ninja Release build is invoking only `package_linux_tarball`, using real Qt 5 tools in a disposable x86_64 Ubuntu 22.04 container. The initial GNU Make attempt exposed missing Qt tools and an LTO jobserver failure; the fresh Ninja attempt installs the prerequisites and uses a validation-only runtime-directory layout for the system Qt libraries/plugins. Logs are under `/private/tmp/us3-package-graph/native-*.log`.

Remaining acceptance: completion and archive inspection of that full clean UltraScan APP package build, followed by native installer/runtime verification. The fixture archives and configured graph do not establish full native NSIS/PKG behavior or runtime library completeness. No ticket was changed or closed.

# Ownership and current scope

The architecture and reuse analysis below excludes all SOMO code, targets, dependencies, and packaging requirements. References to the original mixed scope of #679 are historical context only. #937 remains with samo38.

| Ticket | Owner and labels | Assessment |
|---|---|---|
| [#546 macOS CMake packaging](https://github.com/ehb54/ultrascan-tickets/issues/546) | ehb54; enhancement, triage | General UltraScan packaging. Existing implementation needs reconciliation against the older checklist. |
| [#646 repeated qmake compilation](https://github.com/ehb54/ultrascan-tickets/issues/646) | Unassigned; bug | General incremental-build behavior. Reproduce on current code before changing dependencies. |
| [#679 incomplete standard build](https://github.com/ehb54/ultrascan-tickets/issues/679) | ehb54; bug, triage | Mixed scope. Include documentation and help; track SOMO completion separately. |
| [#768 revision header contents](https://github.com/ehb54/ultrascan-tickets/issues/768) | ehb54; enhancement, triage | Closed with a comment. The original question is answered; broader metadata consistency is separate work. |
| [#869 Linux README architecture](https://github.com/ehb54/ultrascan-tickets/issues/869) | ehb54; general, triage | Fixed locally and fixture-tested; awaiting delivery and closure. |
| [#937 vcpkg organization](https://github.com/ehb54/ultrascan-tickets/issues/937) | samo38; general | Outside our implementation scope; left with samo38 and existing PR #465. |
| [#1024 on-demand packaging](https://github.com/ehb54/ultrascan-tickets/issues/1024) | aaron-auc; bug | Several requested fixes exist in source. Verify produced artifacts, including the RHEL 8 follow-up report. |

All four tickets assigned to ehb54 carry the triage label; none carries the somo label. This supports treating them as items needing evaluation, but does not prove why they were assigned or that implementation is active. The only directly matching open implementation PR found for this set is [#465, new vcpkg layout](https://github.com/ehb54/ultrascan3/pull/465), authored by samo38. Its latest visible update and commit are from July 9, 2026. Leave that implementation with samo38; do not duplicate it in this initiative. No directly matching open PR was found for the other six; unpublished work cannot be ruled out.

# What remains for each ticket

## macOS packaging

[#546](https://github.com/ehb54/ultrascan-tickets/issues/546) reads like an initial implementation checklist, but main already has substantial deployment code: Qt deployment, library-path rewriting, Assistant staging, and a PKG target using pkgbuild and productbuild. The root build explicitly describes PKG packaging rather than CPack DMG packaging. Treat the old DMG and directory-layout requests as decisions to reconcile with the established installer, not automatic requirements for a second packaging system. [Mac deployment](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/macos/MacDeploy.cmake), [PKG implementation](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/macos/MacOsPkg.cmake).

The manual-copy section copies help files only when present, whereas the Linux and Windows deployment scripts contain explicit checks for required help files. Add or verify a consistent final package check on macOS. Acceptance should cover arm64 and x86_64 installation on a machine without the development toolchain, application startup, resources, help navigation, and runtime library resolution. Close the ticket only after its checklist reflects the chosen package format and that evidence exists.

## Incremental builds

Coordination update: the user is testing #646 in another session. This session will not duplicate that work or infer closure from package-build results.

The discussion on [#646](https://github.com/ehb54/ultrascan-tickets/issues/646) attributes recompilation to time-dependent version metadata. Current qmake revision generation already uses the commit date and preserves the header when its values are unchanged. Its forced revision target therefore does not, by itself, establish unnecessary recompilation. Reproduce two successive builds and inspect which files and timestamps cause any second-build work. [qmake revision generator](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/programs/us/revision.sh), [qmake target](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/programs/us/us.pro).

There is a related CMake concern: GenerateVersion.cmake writes the build-directory header unconditionally, although it avoids rewriting the source-directory copy when content is unchanged. This is a candidate cause of unnecessary CMake recompilation, not proof of the original qmake defect. Both generators also write into the source tree, which merits checking when qmake and CMake use the same checkout. [CMake revision generator](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/GenerateVersion.cmake).

Acceptance: an unchanged second build performs no C++ compilation or relinking; a relevant source or commit change updates identity correctly; switching build systems does not cause repeated header churn. A metadata-check command may still run without invalidating compiled targets.

## Documentation and help

[#679](https://github.com/ehb54/ultrascan-tickets/issues/679) names two entry points, makeall.sh and qmake make_all.pro. They do not currently promise the same result. makeall.sh attempts the manual build but disables it with a warning when a usable Python/Sphinx environment cannot be found. make_all.pro lists application and library subprojects without an explicit documentation subproject. [makeall.sh](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/makeall.sh), [make_all.pro](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/make_all.pro).

Define a clear distinction between a developer application build, where documentation may be optional, and an APP distribution package, where working help is required. Document which command produces each. For the supported complete-build path, validate manual.qch, manual.qhc, Assistant, its required plugins, and the application's actual help launch. Packaging must fail if a required component is missing. This overlaps directly with #1024 and the macOS verification in #546.

Keep the SOMO requirement visible as separate remaining scope. Completing documentation alone would not justify closing #679 in its present form.

## Build identity

[#768](https://github.com/ehb54/ultrascan-tickets/issues/768) is closed with a comment. It originally asked where the application version is stored; the answer is utils/us_defines.h. Current CMake extracts the version from that header and checks the root VERSION copy for agreement. Preserve this existing check rather than introduce a competing version source. [Root CMake configuration](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/CMakeLists.txt).

The useful shared work is to define application version, Git commit, commit-count build number, dirty state, and CI run number separately. The on-demand artifact currently calls the GitHub run number “build”; this is not the commit-count BUILDNUM compiled into the program. Its naming may be acceptable, but the distinction should be explicit. The qmake and CMake generators also use different hash lengths and dirty-file rules. Agree on their intended meaning before making output identical. [On-demand workflow](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/.github/workflows/on-demand-build.yml), [qmake generator](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/programs/us/revision.sh), [CMake generator](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/GenerateVersion.cmake).

A follow-up comment on #1024 reports RHEL 8 packages with build 0 and unknown Git identity. Current CMake still allows a no-Git fallback. For distributed CI packages, require trustworthy identity or pass explicit identity into the build; retain a clearly identified fallback if source-archive development builds need it.

Acceptance: package name, README, application display, and CI record identify the same source revision and application version; shallow history or missing Git cannot silently produce misleading release metadata. Track this broader metadata work separately from closed ticket #768.

## Linux README

[#869](https://github.com/ehb54/ultrascan-tickets/issues/869) is fixed locally. At the reviewed baseline, LinuxDeploy.cmake wrote a literal <arch> into its README extraction command while LinuxPkg.cmake derived the actual archive name separately. The implemented fix now generates both from the same package-name value. [README generation](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/linux/LinuxDeploy.cmake), [archive naming](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/linux/LinuxPkg.cmake).

Retain the README if it supplies useful installation guidance. Acceptance: the extraction command names the actual archive, no unresolved placeholders remain, and the documented extracted directory and launcher paths match the package. This can ship independently of the larger refactors.

## External coordination reference for dependency organization

[#937](https://github.com/ehb54/ultrascan-tickets/issues/937) combines two different changes: grouping version-controlled overlay configuration, and moving generated installed dependencies into one shared admin/vcpkg/installed tree. The first can improve navigation. The second conflicts with the current toolchain's deliberate use of per-build-directory vcpkg installations. Its comments explain that APP and HPC manifest features can cause a shared tree to remove dependencies needed by the other profile. Binary caching already provides reuse without sharing the installed tree. [Current toolchain](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/toolchain.cmake).

Review notes for samo38’s separate work: consolidating overlays and triplets under admin/vcpkg may improve navigation, while build-local installed dependencies preserve isolation. Documentation or a diagnostic command could expose resolved tool paths. Avoid folding the ticket's proposed march=native experiment into a layout change; distributable binaries need a separate CPU-compatibility decision. Review these changes with samo38 against [open PR #465](https://github.com/ehb54/ultrascan3/pull/465).

Suggested acceptance for the separate #937 work: APP and HPC builds coexist without altering each other's dependencies; supported Qt variants and architectures resolve the intended overlays and tools; cache restoration still works; no consumer retains obsolete overlay paths. This work should not block package-content fixes.

## On-demand packages

[#1024](https://github.com/ehb54/ultrascan-tickets/issues/1024) already has substantial source-level coverage. The on-demand workflow names artifacts with version, run number, short commit, and platform. The packaging action copies the selected installer/archive into artifact-upload before upload, which is intended to remove preset-directory nesting. Windows deployment rejects missing or empty Assistant, manual.qch, manual.qhc, and qsqlite.dll. Linux deployment rejects missing or empty manual files. [Workflow](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/.github/workflows/on-demand-build.yml), [upload action](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/.github/actions/build-package/action.yml), [Windows checks](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/windows/WinDeploy.cmake), [Linux checks](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/linux/LinuxDeploy.cmake).

One remaining inspection finding: the upload action says it requires exactly one package, but selects the first matching file and only rejects an empty result. Multiple stale candidates could therefore cause the wrong package to be chosen. Select the expected output explicitly or reject ambiguous results.

Acceptance: download and inspect fresh Windows and RHEL 8 artifacts, verify the ZIP root and names, install or extract them, check identity, and launch help from the application. Verify package contents after final packaging, not only in the staging directory. Record the tested commit and workflow run before closing the ticket.

# Recommended delivery sequence

| Order | Work | Tickets | Completion evidence |
|---|---|---|---|
| 1 | Reconcile existing implementations and ownership | All six in scope | Remaining criteria documented; coordinate #1024 with aaron-auc. Leave #937 with samo38. |
| 2 | Verify package completeness and fix remaining omissions | #1024, #679 documentation, #546 | Installed help works; required-file omissions fail packaging; artifact selection is unambiguous. |
| 3 | Deliver the completed Linux README naming fix | #869 | Local x64/arm64 archive fixtures passed; delivery and closure remain. |
| 4 | Align identity and preserve incremental builds | #646, #1024 identity; separate metadata follow-up | Stable no-change rebuilds; accurate identity on every packaged platform. |

Use one tracking initiative with separate pull requests for these outcomes. Package completeness is the main user-facing priority. Identity work ties the incremental-build and artifact-naming issues together; dependency organization remains separate work owned by samo38.

The shared completion gate should cover Windows, Linux Ubuntu, Linux RHEL/Oracle 8, and macOS arm64/x86_64 APP packages. Check HPC only for applicable build identity, archive naming, and dependency isolation; GUI help requirements should not be imposed on HPC packages. For every closure, retain the source commit, build command or workflow run, artifact name, and the relevant verification result. This prevents open tickets from being treated as entirely unimplemented while also avoiding closure based solely on code inspection.

## Proposed split of ticket 679

Keep #679 focused on documentation generation and Qt help setup, and move its SOMO requirement to a separate linked issue outside this analysis and project scope. Preserve the original report and cross-link the two scopes. The documentation ticket can then close when its own acceptance criteria pass without implying that SOMO integration is complete. This is a proposed ticket change; no GitHub tickets have been edited or created.

# Architecture assessment and project scale

The six tickets point to two related structural weaknesses: the build does not consistently express the complete product as a dependency graph, and reusable application functionality is often owned by executable directories rather than library targets. The result is reliance on wrapper execution order, repeated compilation of large source groups, and metadata and packaging rules that can drift between paths. This is a build-system and component-boundary project, with small defect fixes that can proceed independently.

This section records the analysis of the main baseline stated above. All counts exclude SOMO; source-line counts are descriptive, not performance measurements. The subsequent bounded implementation and its validation are recorded under “First implementation result” below.

## Current flow and its weak boundaries

The supported CMake wrapper path is approximately:

```text
CI ref and profile
  -> resolve dependency toolchain
  -> configure CMake
  -> wrapper builds the default target
  -> platform deployment target gathers files into staging
  -> platform installer or archive target
  -> CI searches for a package and uploads it
```

The default build before packaging is significant. Root deployment targets for Windows, Linux, and macOS start with a dependency on us and conditionally add documentation. They do not explicitly depend on the full companion-program set. scripts/build.ps1 documents that its full build is necessary before the package target; otherwise companion executables would not be built. WinDeploy.cmake then globs us_*.exe from the output directory. A clean staging directory does not solve an incomplete or stale input directory. The documented standalone package target therefore has weaker guarantees than the wrapper sequence. This is confirmed target structure; the precise incomplete-package outcome still needs a clean-build reproduction. See [root targets](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/CMakeLists.txt), [Windows wrapper](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/scripts/build.ps1), and [Windows staging](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/admin/cmake/packaging/windows/WinDeploy.cmake).

Documentation has a similar split. Its CMake target can disappear when Sphinx is missing, or produce intermediate help sources when qhelpgenerator is unavailable. Wrappers and some platform staging scripts impose stronger requirements for a package. These checks are useful but distributed. The required result should depend on build intent: developer binaries, complete APP distribution, or HPC distribution. Those intents need not require identical components.

Release-process context supplied by the user: formal builds use tags while contributors pause GitHub changes. No change to that source-selection process is planned.

| Structural issue | Ticket connection | Consequence |
|---|---|---|
| Required package components are not fully represented in package dependencies | #546, #679, #1024 | Success depends on entry point and previously available outputs. |
| Identity and filenames are constructed in multiple places | #869, #1024; separate metadata follow-up | Labels, compiled version, README instructions, and actual outputs can disagree. |
| Generated metadata has inconsistent ownership and write behavior | #646; separate metadata follow-up | Header churn can trigger recompilation; qmake and CMake can affect the same source-tree header. |
| Publication discovers outputs through broad searches | #1024 | Existing files can influence which artifact is selected. |
| Executable directories act as reusable implementation modules | Broader than the six tickets | A small shared change may compile repeatedly and pull in unrelated program code. |

These are related weaknesses, not proof that one root cause explains every reported defect. In particular, repeated source compilation does not cause the literal architecture placeholder in #869. It makes the wider build expensive and harder to reason about while the packaging and identity gaps cause correctness problems at different boundaries.

## Quantified source reuse

A static inventory examined all 71 programs/*/CMakeLists.txt files. It removed line comments, normalized explicit existing .cpp paths, expanded the source-directory variables used in those paths, and counted each source once per program build file. It did not execute CMake, expand globbed source sets, count headers or generated moc files, or measure compiler executions. Platform totals below apply the program exclusions visible in programs/CMakeLists.txt and exclude MPI for APP.

| Inventory | Program definitions | Unique explicit .cpp files | Source-to-program memberships | Files listed in multiple programs | Memberships beyond one per unique file |
|---|---:|---:|---:|---:|---:|
| All main program definitions, including MPI | 71 | 193 | 484 | 48 | 291 |
| Non-MPI APP candidate set | 70 | 178 | 469 | 48 | 291 |
| macOS APP candidate set after platform exclusions | 62 | 167 | 277 | 41 | 110 |

For the non-MPI APP set, 291 of 469 source memberships, approximately 62%, are repetitions beyond one membership per unique file. This is a compilation-reuse opportunity indicator, not a claim that 62% of elapsed build time can be eliminated. Compatible compilation settings and dependency boundaries must be established first.

The same APP inventory contains 217,723 unique physical source lines and 913,375 lines when weighted by source membership, a factor of approximately 4.2. Comments and blank lines are included; preprocessing, templates, generated Qt code, optimization, caching, parallelism, and linking costs are not. On macOS the corresponding counts are 206,882 and 447,168, approximately 2.2 times. These figures describe repeated source exposure, not emitted machine code or reachable functionality.

Twelve program definitions import .cpp files from outside their own directory. The largest aggregators are concentrated in seven programs:

| Program definition | Explicit .cpp files | From outside its own directory | Physical source lines represented |
|---|---:|---:|---:|
| us_com_project | 48 | 46 | 120,430 |
| us_com_project_academic | 48 | 47 | 120,423 |
| us_protocol_dev | 48 | 46 | 119,490 |
| us_query_rmsd | 41 | 40 | 91,040 |
| us_autoflow_analysis | 40 | 38 | 84,613 |
| us_fematch | 40 | 32 | 84,607 |
| us_reporter_gmp | 40 | 38 | 84,616 |

The corresponding qmake files contain the same counts of explicit .cpp tokens for these seven programs. The pattern predates or spans CMake; changing build-system syntax alone would preserve it.

The analysis-profile pair appears in ten program definitions. Six editor implementation files appear in eight programs each, and nine 2DSA implementation files also appear in eight each. The reporter implementation itself appears in seven. The largest repeated source-line groups are editing, reporting, 2DSA, analysis profiles, experiment setup, autoflow, and FEMatch. See representative [FEMatch sources](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/programs/us_fematch/CMakeLists.txt) and [project sources](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/programs/us_com_project/CMakeLists.txt).

## Where reuse is sound and where it is overextended

The core libraries already provide compiled reuse: programs link us_utils and us_gui through us_program_common. An INTERFACE target such as us_program_common shares configuration and link requirements; it does not consolidate the programs' own .cpp compilation.

The application layer uses source reuse heavily enough to be an architectural concern. It avoids maintaining copies of source text, but still compiles that implementation into each consuming executable. That is legal C++, and separate executables do not by themselves create a one-definition-rule violation. The cost is repeated compilation, repeated Qt metadata generation, potentially larger distributions, and long source lists that must be maintained together.

The strongest impediment to straightforward library extraction is reciprocal coupling. The CMake source groups for autoflow, FEMatch, and the reporter import one another. This is not merely a naming problem: us_resplot_fem.cpp includes autoflow and reporter headers, while their headers include FEMatch. Turning every existing directory into a separate library would preserve these cycles. Identify lower-level calculation, data, and presentation services first, then place workflow orchestration above them. An internal combined library can be a transition if necessary, but it should not be mistaken for completed separation.

Two additional findings concern library variants:

- gui/CMakeLists.txt creates both shared and static GUI targets unconditionally. With no EXCLUDE_FROM_ALL, a default APP build builds the static GUI target even though APP selects shared libraries. The inspected GUI source glob contains 61 .cpp files before generated resources and moc work.
- utils/CMakeLists.txt always creates its ordinary library and additionally creates a static library for testing or static preference. With the root BUILD_SHARED_LIBS setting, TEST and HPC can build both variants in the default target even when their principal consumers select static. The inspected utils glob contains 61 .cpp files before NO_DB exclusions.
- configure_us_gui_target adds Qt::OpenGLWidgets to the literal us_gui target rather than its target_name parameter. This is a confirmed inconsistency in the configuration helper; whether another dependency masks it for the static target needs a configured-build check.

See [GUI targets](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/gui/CMakeLists.txt) and [utility targets](https://github.com/ehb54/ultrascan3/blob/1f838727ffaa9549db62884e0504acf2f94b3eac/utils/CMakeLists.txt).

Separate compilation is justified when the output contract differs: Qt 5 versus Qt 6, target architecture, compiler ABI, debug/runtime settings, NO_DB, instrumentation, and Windows export/import definitions. US_UTIL_STATIC changes both utility and GUI export macros in us_extern.h. Therefore, an object file built for the shared variant must not simply be reused for the static variant without checking its compilation contract.

Static libraries are often a useful first extraction: compile a component once per compatible configuration and link it into multiple programs. They reduce compilation duplication while keeping a copy of required machine code in each executable. Shared libraries can additionally centralize deployed code but introduce export, ABI, loader-path, and packaging requirements. Object libraries can reuse compiled objects when settings genuinely match, but do not automatically solve source ownership, transitive dependencies, or Qt moc ownership. Each extracted component should own its generated Qt metadata exactly once per variant. See [CMake library types](https://cmake.org/cmake/help/v3.21/command/add_library.html), [target usage requirements](https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html), and [AUTOMOC](https://cmake.org/cmake/help/v3.21/prop_tgt/AUTOMOC.html).

“Cross-compilation” means producing code for another target environment. The observed source-list duplication is compilation across programs, not cross-compilation. The CI matrix largely selects native platform runners or Linux containers. The toolchain's host-based defaults should not be taken as a general cross-compilation design, but cross-compilation is not established as the cause of these six issues. See [CMake toolchains](https://cmake.org/cmake/help/v3.21/manual/cmake-toolchains.7.html).

## Existing progress and project boundaries

[Open PR #618](https://github.com/ehb54/ultrascan3/pull/618), by aaron-auc, already extracts reporting calculations into us_calc_* code under us_utils. That addresses shared calculation ownership and should be accounted for before designing overlapping changes. Its title calls this a library, while its description places the implementation in the existing utility library. It does not by itself remove the broad application source lists. Its proposed platform-policy changes also mean the counts and supported matrix should be refreshed after it merges.

A practical project has three workstreams: complete-product packaging, consistent build identity and incremental behavior, and application component reuse. They can share evidence and acceptance criteria without requiring one large rewrite. The first two cover the six selected tickets; the third is a larger follow-on exposed by this analysis. #937 belongs to samo38 and is not part of any of these workstreams.

# Established practices applicable to UltraScan

The recommended architecture is an explicit build, install, validate, package, and publish sequence:

```text
Immutable source revision + build configuration + pinned toolchain
  -> required component targets
  -> clean installation tree
  -> runtime dependency deployment
  -> validate required contents and identity
  -> platform package
  -> inspect or install the package and smoke-test
  -> publish the exact verified artifact
```

CMake already supplies target dependencies, install rules, components, and packaging integration. Add missing program dependencies and install rules incrementally; preserve the established PKG, NSIS, and tar formats while improving the common input tree. CPack normally packages files described by install rules, but simply enabling CPack is not enough when target dependencies or required components are incomplete. Also, cmake --install alone does not build missing outputs: the orchestrated target must build them first. See [CMake install rules](https://cmake.org/cmake/help/v3.21/command/install.html) and [CPack](https://cmake.org/cmake/help/v3.21/module/CPack.html).

Keep Qt deployment as a platform-aware step. Linking successfully does not establish that runtime plugins, Assistant, help collections, and loader paths will work on a clean machine. Native deployment tools and explicit application requirements remain necessary. See [Qt deployment guidance](https://doc.qt.io/qt-6/deployment.html).

Use one resolved identity record for the application version, full commit, optional commit count, target, profile, and toolchain identity. Treat the CI run number separately. Generate headers and README filenames from those agreed values, and return the exact package path to publication rather than searching all build directories.

Generated files should have declared outputs and dependencies, and unchanged content should not update a header timestamp. A metadata check can run every build if needed to observe Git state, while writing its outputs only when values change. CMake configure_file preserves timestamps when content is unchanged; custom commands support OUTPUT, DEPENDS, and BYPRODUCTS. See [configure_file](https://cmake.org/cmake/help/v3.21/command/configure_file.html) and [custom commands](https://cmake.org/cmake/help/v3.21/command/add_custom_command.html).

Pinned dependencies and immutable source identity improve repeatability, but do not establish byte-for-byte reproducibility. SOURCE_DATE_EPOCH is an established way to stabilize source-derived timestamps where tools support it. Signing and other platform behavior need separate treatment. Full reproducibility can be a later objective; it need not block package correctness. See [SOURCE_DATE_EPOCH](https://reproducible-builds.org/docs/source-date-epoch/).

# Determining how much of a source file is needed

There is no single reliable “percent needed” for a C++ source file. Measure compilation, link retention, and observed execution separately for a specified program and configuration.

| Question | Evidence | What it cannot establish |
|---|---|---|
| How often is a file compiled, and are the variants compatible? | compile_commands.json grouped by normalized source path and meaningful compiler settings | Whether all emitted functions are used |
| Which objects and functions contribute to one binary? | Linker map, symbols and section sizes; debug information for source attribution | Whether retained code is exercised or semantically required |
| Which functions execute in supported workflows? | Function/branch coverage for those workflows | That zero-covered code can safely be deleted |
| Can a component be extracted without changing behavior? | Dependency analysis plus builds and behavioral tests of all consumers | Universal safety from one successful application run |

CMAKE_EXPORT_COMPILE_COMMANDS provides exact compiler invocations with Makefile and Ninja generators. Compare definitions, include paths, language standard, optimization, PIC, target ABI, and instrumentation before classifying repeats as equivalent. Source-membership counts above are a preliminary inventory; this generated database is the next step toward an actual configured graph. See [compilation database support](https://cmake.org/cmake/help/v3.21/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html).

For an initial experiment, choose Linux and one heavily aggregated program such as us_query_rmsd alongside us_fematch. Capture the existing link map first. In a separate diagnostic configuration, function/data sections plus linker garbage collection can expose which sections are discarded. For GNU tooling these are -ffunction-sections, -fdata-sections, --gc-sections, --print-gc-sections, and a -Map output. These are diagnostic proposals, not changes to production flags. A static archive normally extracts object members, so one referenced function can pull in the object containing many others; section-level analysis helps distinguish that from true dependency. See [GCC section options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html) and [GNU linker options](https://sourceware.org/binutils/docs/ld/Options.html).

Link retention is conservative. Qt metaobjects, slots, virtual dispatch, callbacks, static initialization, exported symbols, and whole-archive resource registration can retain functions without obvious direct calls. Optimization and inlining complicate attribution to individual source functions. A small retained fraction can indicate an overlarge module, but it is not an automatic deletion list.

Use the project's existing coverage instrumentation, or a separate LLVM source-coverage configuration, to record representative editing, fitting, reporting, and failure workflows. Report function and branch coverage with the workflow names; unexecuted code remains unknown. LLVM coverage supplies source-level execution evidence, not proof of semantic deadness. See [LLVM source coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html).

The useful per-file result is: consumers; compatible compile variants; compile time; retained code bytes/functions per consumer; exercised functions/branches by workflow; candidate component; and confidence. A high-cost file compiled into eight programs but retaining a small, different subset in each is a strong extraction candidate. A file retained almost entirely by all eight is a strong compiled-library reuse candidate. Neither result requires deleting its source.

## Evidence needed before sizing implementation effort

Measure one representative Linux APP build, one macOS APP build, and the relevant Windows library/linkage path. Keep clean-build time, no-change rebuild time, and rebuild-after-one-shared-source-change separate; record whether compiler caches are warm. Use compiler timing and linker maps to identify the dominant files, rather than deriving effort or expected savings from line counts.

Test a direct package-target invocation from a clean configured tree as well as the wrapper path. Compare expected component inventories and installed behavior. Probe missing help files, ambiguous package outputs, and absent Git metadata deliberately in isolated test environments.

Refresh the source inventory after #618 lands, then select one narrowly scoped extraction and compare all affected consumers. The likely architectural boundary is reusable calculation/data services below presentation and workflow orchestration, but the reciprocal application dependencies need a symbol-level review before committing to module names or a timetable.

The current evidence supports a substantial but concentrated problem: dozens of repeated implementation files, seven large aggregating programs, redundant default library variants, and several weak build-to-package boundaries. It does not yet support a calendar estimate, a measured speedup, or a claim that most of the source is unused.

# First implementation result

The static GUI library and its UltraScan3::Gui_Static alias are now created only when BUILD_TESTING or US3_PREFER_STATIC is enabled, matching the existing condition for the static utility library. Normal APP builds select shared libraries and no longer create the unused static GUI target. TEST and explicitly requested static configurations retain it. This also applies to configurations without a named profile when neither tests nor static libraries are requested.

Only gui/CMakeLists.txt was changed for this implementation. The Qt OpenGLWidgets helper inconsistency, utility-library variants, application source sharing, and packaging architecture are separate findings and remain unchanged.

Validation used an isolated archive of the current checkout's HEAD plus this CMake edit, avoiding the workspace's unrelated source and test changes. Baseline and changed APP and TEST configurations succeeded in the existing Linux x86_64 build container with Qt 5.15.3. Generated compile_commands.json entries and link.txt rules were compared after normalizing build-directory paths and CMake's trailing RPATH padding.

| Check | Result |
|---|---|
| APP compilation graph | 758 commands before, 695 after; the 63 removed commands belong exclusively to us_gui_static: 61 GUI sources, generated moc compilation, and generated resources. |
| APP link graph | Only the static GUI archive rule was removed. All remaining link commands were equivalent. |
| TEST compilation graph | 359 commands before and after, with identical normalized commands. |
| TEST link graph | Identical normalized link commands; GUI and program tests retain their previous library linkage. |
| Explicit static configuration with testing disabled | Configure and generation succeeded; us_gui_static remains available. |
| Patch formatting | git diff --check passed for gui/CMakeLists.txt. |

This verifies target selection and generated build rules. It is not an elapsed-time benchmark, a fresh executable build, or an installer/runtime test. Windows, macOS, and Qt 6 were not configured in this verification. No GitHub ticket was changed or closed.

## Effect on the planned tickets

| Ticket | Effect of this change |
|---|---|
| #646 incremental recompilation | Supporting efficiency improvement only. Avoids an unnecessary library compilation in APP builds, but does not fix or establish the reported qmake no-change rebuild problem. |
| #546 macOS packaging | No package-content change is intended; shared GUI installation rules are unchanged. Clean-machine package verification remains required. |
| #679 documentation and help | No direct fix. Documentation production and help deployment are unchanged. |
| #768 version-location question | Closed with a comment; no implementation required. Broader identity work is separate. |
| #869 README architecture | No direct fix from the GUI optimization. The separate local implementation documented above replaces the placeholder with the calculated archive filename and passed fixture validation; delivery remains pending. |
| #1024 on-demand packaging | Reduces the generated APP compilation workload used by the workflow; does not resolve artifact selection, metadata, or help validation. |

The other architectural findings give these tickets shared priorities: package dependency completeness informs #546/#679/#1024; consistent identity and output naming inform #869/#1024 and a separate metadata follow-up; generated-header stability informs #646. Closed ticket #768 is not part of that follow-up. Application-library extraction is a separate, larger follow-on and is not a prerequisite for finishing those tickets.


