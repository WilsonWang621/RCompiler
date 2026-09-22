# C++ compiler integration for the test runner.
#
# The executable is expected to support:
#   ./target/compiler --stage semantic <source>
#   ./target/compiler --stage codegen <source> -o <output>

# Download the pinned ANTLR runtime when needed, then configure and build the
# C++ compiler. CMake places the executable at target/compiler.
BUILD = ./scripts/build.sh

# Accept a valid program with exit code 0 and reject an invalid program with
# exit code 1. Diagnostics may be written to stderr.
SEMANTIC = ./target/compiler --stage semantic {source}

# Generate RV32IM assembly at the exact path supplied through {output}.
CODEGEN = ./target/compiler --stage codegen {source} -o {output}

# Run the generated assembly in REIMU. Simulator messages go to stderr while
# program output and cycle profiles are written to runner-provided files.
RUN = xmake run -P vendor/REIMU reimu --memory=256M --stack=1M \
    -f {output} -o {stdout} -p {profile} 1>&2
