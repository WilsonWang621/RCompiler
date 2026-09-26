#pragma once

#include<vector>
#include<memory>
#include<string>
#include<iostream>

namespace rx::ast{

struct ASTNode{
    virtual ~ASTNode() = default;
    virtual void dump(std::ostream &out, int indent = 0) const = 0;
};

class Item : public ASTNode {};

class Stmt : public ASTNode {};

class Expr : public ASTNode {};

using ItemPtr = std::unique_ptr<Item>;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

//the top floor
class Crate : public ASTNode{
    std::vector<ItemPtr> items;

public:
    void addItem(ItemPtr item){
        items.push_back(std::move(item));
    }

    void dump(std::ostream &out, int indent = 0) const override;
};

class BlockExpr final : public Expr{
    std::vector<StmtPtr> stmts_;
public:
    void addStatement(StmtPtr statement){ 
        stmts_.push_back(std::move(statement));
    }
    void dump(std::ostream &out, int indent = 0) const override;
};

class FunctionItem final : public Item{
private:
    std::string name_;
    std::unique_ptr<BlockExpr> body_;
public:
    FunctionItem(std::string name, std::unique_ptr<BlockExpr> body):name_(std::move(name)), body_(std::move(body)){};
        
    void dump(std::ostream &out, int indent = 0) const override;
};

//version 1.1
class LetStmt final : public Stmt{
    std::string name_;
    ExprPtr initializer_;

public:
    LetStmt(std::string name, ExprPtr initializer):name_(std::move(name)), initializer_(std::move(initializer)){};

    void dump(std::ostream &out, int indent = 0) const override;
};

class IntegerLiteralExpr final : public Expr{
    std::string text_;

public:
    IntegerLiteralExpr(std::string text):text_(std::move(text)){};

    void dump(std::ostream &out, int indent = 0) const override;
};

//1.2
class BinaryExpr final : public Expr {
private:
    std::string op_;
    ExprPtr left_;
    ExprPtr right_;

public:
    BinaryExpr(std::string op, ExprPtr left, ExprPtr right): op_(std::move(op)), left_(std::move(left)), right_(std::move(right)) {}

    void dump(std::ostream &out, int indent = 0) const override;
};
}