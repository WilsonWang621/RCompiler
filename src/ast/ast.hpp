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
    ExprPtr tail_;
public:
    void addStatement(StmtPtr statement){ 
        stmts_.push_back(std::move(statement));
    }
    
    void setTail(ExprPtr tail){
        tail_ = std::move(tail);
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
    bool isMutable_;
    ExprPtr initializer_;

public:
    LetStmt(std::string name, bool isMutable, ExprPtr initializer):name_(std::move(name)), isMutable_(isMutable), initializer_(std::move(initializer)){};

    void dump(std::ostream &out, int indent = 0) const override;
};

class ExprStmt final : public Stmt {
    ExprPtr expression_;

public:
    explicit ExprStmt(ExprPtr expression): expression_(std::move(expression)){};

    void dump(std::ostream &out, int indent = 0) const override;
};


class IntegerLiteralExpr final : public Expr{
    std::string text_;

public:
    IntegerLiteralExpr(std::string text):text_(std::move(text)){};

    void dump(std::ostream &out, int indent = 0) const override;
};

class BooleanLiteralExpr final : public Expr{
    bool flag_;

public:
    BooleanLiteralExpr(bool flag): flag_(flag){};

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

class UnaryExpr final : public Expr{
    std::string op_;
    ExprPtr operand_;

public:
    UnaryExpr(std::string op, ExprPtr operand): op_(std::move(op)), operand_(std::move(operand)){};

    void dump(std::ostream &out, int indent = 0) const override;
};

class PathExpr final : public Expr{
    std::vector<std::string> segments_;

public:
    explicit PathExpr(std::vector<std::string> segments) : segments_(std::move(segments)) {}

    void dump(std::ostream &out, int indent = 0) const override;
};

class AssignExpr final : public Expr {
private:
    ExprPtr target_;
    ExprPtr value_;

public:
    AssignExpr(ExprPtr target, ExprPtr value): target_(std::move(target)), value_(std::move(value)) {}

    void dump(std::ostream &out, int indent = 0) const override;
};
}
