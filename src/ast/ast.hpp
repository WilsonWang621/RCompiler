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
public:
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

}