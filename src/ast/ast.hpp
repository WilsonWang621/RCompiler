#pragma once

#include<vector>
#include<memory>
#include<string>

namespace rx::ast{

struct ASTNode{
    virtual ~ASTNode() = default;
};

struct Item : ASTNode{};

struct UseItem : Item {};
struct FunctionItem : Item {
    std::string name;
};
struct StructItem : Item {};
struct ConstItem : Item {
    std::string name;
};
struct ImplItem : Item {};


struct Stmt : ASTNode{};
struct LetStmt : Stmt{};
struct ExprStmt : Stmt{};

//the top floor
struct Crate {
    std::vector<std::unique_ptr<Item>> items;
};

}