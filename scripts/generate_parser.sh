#!/bin/sh

set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
antlr_jar="$project_dir/.antlr/antlr-4.13.2-complete.jar"

if [ ! -f "$antlr_jar" ]; then
    echo "error: ANTLR tool not found: $antlr_jar" >&2
    exit 2
fi

mkdir -p "$project_dir/generated"
cd "$project_dir/grammar"
java -jar "$antlr_jar" \
    -Dlanguage=Cpp -package rx -visitor -no-listener \
    -o "$project_dir/generated" Lexer.g4 Parser.g4

# The grammar's generated class is named Lexer, which clashes with
# antlr4::Lexer after the C++ template imports the antlr4 namespace.
# Fully qualify its out-of-class definitions to remove the ambiguity.
sed -i \
    -e 's/^Lexer::/rx::Lexer::/' \
    -e 's/ Lexer::/ rx::Lexer::/g' \
    -e 's/: Lexer(input)/: antlr4::Lexer(input)/' \
    "$project_dir/generated/Lexer.cpp"

sed -i \
    -e 's/Parser::/rx::Parser::/g' \
    -e 's/options) : Parser(input)/options) : antlr4::Parser(input)/' \
    "$project_dir/generated/Parser.cpp"
