[![CodeQL](https://github.com/ehb54/ultrascan3/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/ehb54/ultrascan3/actions/workflows/codeql-analysis.yml)

This is the home of the UltraScan GUI software source code.
More information is available [here](https://ultrascan.aucsolutions.com)

The UltraScan project is supported by NIH Funding GM120600

### Licensing

This project is licensed under the Lesser General Public License (LGPL). You can find the LGPL license in the LICENSE file in the root directory of this project.

This project uses the Eigen library, which is licensed under the Mozilla Public License 2.0 (MPL2). 
You can find the Eigen source code and license at [Eigen GitHub repository](https://gitlab.com/libeigen/eigen).

### Installation

The simplest way is to download and install a precompiled [binary](https://www.ultrascan3.aucsolutions.com/download.php)

### Building
 
If you wish to compile from source, here are some notes to get you started.

You will need [qt6](https://doc.qt.io/qt-6/gettingstarted.html) development and [qwt6.3.0](https://qwt.sourceforge.io/) installed.
For now [qt5](https://doc.qt.io/archives/qt-5.15/gettingstarted.html) is still supported but deprecated.

We typically compile from source as in [ansible roles build-qt, build-qwt & build-us3](https://github.com/KJSain/us3lims-roles/tree/master/roles)

One could also take hints from this [Dockerfile](https://github.com/ehb54/ultrascan3/blob/main/admin/codeql/docker/docker/Dockerfile) which uses an Ubuntu 20.04 container and qt packages.

Please create an issue or [contact us](https://www.ultrascan3.aucsolutions.com/contacts.php) if you have further questions.

### Toolchains in forks

GitHub application builds use the artifact locations and checksums in
`buildsys/toolchain.lock.json`. Forks inherit upstream toolchains when they sync;
automatic toolchain builds and scheduled nightlies run only in `ehb54/ultrascan3`.

On pushes, toolchain builds select targets whose tracked inputs differ from their
published `inputs_digest`. Platform scripts and Dockerfiles affect their targets;
shared dependency inputs and toolchain workflow changes affect all targets. Pins
without a fingerprint rebuild once to record it. Unchanged targets are skipped,
and a run with no selected targets does not publish or commit pins.

To test dependency changes in a fork, run **Toolchain build** manually on a feature
branch. Manual runs rebuild the selected targets even when their inputs match.
The workflow publishes to the fork and updates the pins for those targets on
that branch. Unselected targets keep their existing sources. Uncheck `open_pr` to
publish without committing pins; on `main`, enabling it creates a pin PR that is
automatically merged after validation. Published archives are retained so older
revisions and forks can still fetch their pinned dependencies.

### Container environment

Assuming you are experienced with containers...

A containerized development image is also available
```docker pull ehb1/usbuildtest```

The source is in `/ultrascan3`

Run and export the containers display in Linux 
```docker run -it --rm -e DISPLAY=${DISPLAY} -v /tmp/.X11-unix:/tmp/.X11-unix -v $HOME/.Xauthority:/home/xterm/.Xauthority --hostname $(hostname) ehb1/us3comp bash```

N.B. the `--rm` above will delete the container after you exit, so any changes will be lost after exiting.

Update the ultrascan3 code ```cd /ultrascan3 && git pull```

Compile both `us` and `us3_somo` ```cd /ultrascan3 && ./makeall.sh && ./makesomo.sh```

Run the compiled application ```LD_LIBRARY_PATH=/ultrascan3/lib:/qwt-6.1.5/lib /ultrascan3/bin/us```

Note - subprocesses do not currently run in the container, so you must run any compiled application individually, e.g. `us_fematch`


## Code style

The style of the UltraScan III coding standards ([wiki](https://github.com/ehb54/ultrascan3/wiki/UltraScan-III-Coding-Standards))
is encoded in `.clang-format` (clang-format 18) and enforced **on changed lines only**, by a pre-commit hook and the `Code Style` GitHub action.

    sudo apt install clang-format-18         # includes git-clang-format
    git config core.hooksPath .githooks      # or: pre-commit install
    scripts/check-format.sh --fix --staged   # format staged changes, then git add

In addition to formatting, `scripts/check-conventions.py` checks changed code for: `nullptr` (not `NULL`), `f()` (not `f( void )`),
`connect( a, &A::sig, b, &B::slot )` (not `SIGNAL`/`SLOT`), a trailing newline and an include guard; new files must start with
`//! \file <name>` and headers must use the guard `<NAME>_H` (`us_foo.h` -> `US_FOO_H`). Includes are sorted within each block.

`clang-tidy` (`.clang-tidy`) runs in the *Static Analysis* workflow on the changed lines of each pull request: security findings
(`clang-analyzer-*`, `bugprone-*`) are annotated as errors first, style findings (`modernize-use-nullptr`,
`modernize-redundant-void-arg`) as warnings with fix suggestions on the PR and a `clang-tidy-fixes` patch artifact.
Locally (needs a `compile_commands.json`): `scripts/run-clang-tidy-diff.sh --fix origin/main <build-dir>`.
