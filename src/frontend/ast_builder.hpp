#pragma once

#include"ParserBaseVisitor.h"
#include"ast.hpp"

namespace rx::frontend{

class ASTBuilder final : public ParserBaseVisitor{

    std::unique_ptr<ast::Item> buildItem(rx::Parser::ItemContext *ctx);

    std::unique_ptr<ast::FunctionItem> buildFunction(rx::Parser::FunctionDefinitionContext *ctx);

    std::unique_ptr<ast::BlockExpr> buildBlock(rx::Parser::BlockExpressionContext *ctx);

public:
    std::unique_ptr<ast::Crate> build(rx::Parser::CrateContext *ctx);
};
}
