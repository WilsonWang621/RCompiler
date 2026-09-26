#pragma once

#include<vector>
#include<memory>
#include<string>

namespace rx::ast{

// basic node
struct Info{ // store the pos and info of source code for debugging
    std::size_t begin;
    std::size_t end;
    std::size_t line;
};
struct ASTNode{
    Info info;
    virtual ~ASTNode() = default;
};

struct Item;
struct Type;
struct Expr;
struct Stmt;
struct ConstExpr;

using ItemPtr = std::unique_ptr<Item>;
using TypePtr = std::unique_ptr<Type>;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using ConstExprPtr = std::unique_ptr<ConstExpr>;

//the top floor
struct Crate {
    std::vector<std::unique_ptr<Item>> items;
};
//the first floor
struct Item{
    enum class Kind{
        Use,
        Func,
        Struct,
        Const,
        Imply,
    };
    Kind kind_;

    Item(Kind kind):kind_(kind){};
};

//some preparations
struct Lifetime{
    Info info_;
    std::string name;
};

enum class PathNameKind {
  Identifier,
  SelfValue, // self
  SelfType,  // Self
  Super,
  Crate,
};

struct PathSegment{
    Info info_;
    PathNameKind kind_;
    std::string name;
};

struct UseTree : ASTNode{ //a recursive structure

};

//the second floor
struct UseItem : Item{
    UseItem() : Item(Kind::Use){};
    std::unique_ptr<UseTree> tree;
};

//Type Node

//const expression

//function

//Struct

//Impl

//Expression base class
struct Expr : ASTNode{
    enum class Kind{
        IntegerLiteral,
        BooleanLiteral,
        Path,
        Block,
        Operator, 
        Unit, 
        Array,
        Struct, 
        Call
    };

    explicit Expr(Kind kind) : kind_(kind){};

    Kind kind_;
};
}