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

    ast::ExprPtr buildUnary(rx::Parser::UnaryExpressionContext *ctx);

    ast::ExprPtr buildPostfix(rx::Parser::PostfixExpressionContext *ctx);

    ast::ExprPtr buildPrimary(rx::Parser::PrimaryExpressionContext *ctx);

    ast::ExprPtr buildPath(rx::Parser::PathInExpressionContext *ctx);

    ast::ExprPtr buildStatementExpression(rx::Parser::StatementExpressionContext *ctx);

    ast::ExprPtr buildStatementAdditive(rx::Parser::StatementAdditiveExpressionContext *ctx);

    ast::ExprPtr buildStatementMultiplicative(rx::Parser::StatementMultiplicativeExpressionContext *ctx);
    
    ast::ExprPtr buildStatementCast(rx::Parser::StatementCastExpressionContext *ctx);

    ast::ExprPtr buildStatementUnary(rx::Parser::StatementUnaryExpressionContext *ctx);

    ast::ExprPtr buildStatementPostfix(rx::Parser::StatementPostfixExpressionContext *ctx);

    // 两套表达式入口共用的底层转换。
    ast::ExprPtr buildNonBlockPrimary(rx::Parser::NonBlockPrimaryContext *ctx);
public:
    std::unique_ptr<ast::Crate> build(rx::Parser::CrateContext *ctx);
};
}
