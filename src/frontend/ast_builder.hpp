#pragma once

#include"ParserBaseVisitor.h"
#include"ast.hpp"

namespace rx::frontend{

class ASTBuilder final : public ParserBaseVisitor{

    std::unique_ptr<ast::Item> buildItem(rx::Parser::ItemContext *ctx);

    std::unique_ptr<ast::FunctionItem> buildFunction(rx::Parser::FunctionDefinitionContext *ctx);

    std::unique_ptr<ast::BlockExpr> buildBlock(rx::Parser::BlockExpressionContext *ctx);

    //1.1
    ast::StmtPtr buildStatement(rx::Parser::StatementContext *ctx);

    std::unique_ptr<ast::LetStmt> buildLet(rx::Parser::LetStatementContext *ctx);

    ast::ExprPtr buildExpression(rx::Parser::ExpressionContext *ctx);

    ast::ExprPtr buildLiteral(rx::Parser::LiteralExpressionContext *ctx);

    //1.2
    ast::ExprPtr buildAdditive(rx::Parser::AdditiveExpressionContext *ctx);

    ast::ExprPtr buildMultiplicative(rx::Parser::MultiplicativeExpressionContext *ctx);

    ast::ExprPtr buildCast(rx::Parser::CastExpressionContext *ctx);
public:
    std::unique_ptr<ast::Crate> build(rx::Parser::CrateContext *ctx);
};
}
