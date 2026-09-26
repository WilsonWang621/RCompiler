#include"ast_builder.hpp"

#include <stdexcept>

namespace {

// 本次只允许穿过单孩子的表达式规则。
void requireSingleChild(antlr4::ParserRuleContext *ctx) {
    if (ctx == nullptr || ctx->children.size() != 1) {
        throw std::runtime_error(
            "only plain integer literal expressions are supported for now"
        );
    }
}

} // namespace

namespace rx::frontend{
    
std::unique_ptr<ast::Item> ASTBuilder::buildItem(rx::Parser::ItemContext *ctx){
    if (ctx->functionDefinition() != nullptr) {
        return buildFunction(ctx->functionDefinition());
    }

    throw std::runtime_error(
        "minimal AST currently supports only function items"
    );
}

std::unique_ptr<ast::FunctionItem> ASTBuilder::buildFunction(rx::Parser::FunctionDefinitionContext *ctx){
    std::string name = ctx->identifier()->getText();
    auto body = buildBlock(ctx->blockExpression());

    return std::make_unique<ast::FunctionItem>(std::move(name), std::move(body));
}

std::unique_ptr<ast::BlockExpr> ASTBuilder::buildBlock(rx::Parser::BlockExpressionContext *ctx){
    // 例如 { let x = 1; x } 中，最后的 x 是尾表达式。
    // 本次暂不支持。
    if (ctx->statementExpression() != nullptr) {
        throw std::runtime_error("block tail expressions are not supported yet");
    }

    auto block = std::make_unique<ast::BlockExpr>();

    for (auto statementCtx : ctx->statement()) {
        auto statement = buildStatement(statementCtx);
        block->addStatement(std::move(statement));
    }

    return block;
}

std::unique_ptr<ast::LetStmt> ASTBuilder::buildLet(rx::Parser::LetStatementContext *ctx){
    auto *binding = ctx->identifierBinding();

    if (binding->MUT() != nullptr) {
        throw std::runtime_error("let mut is not supported yet");
    }
    if (ctx->typeRef() != nullptr) {
        throw std::runtime_error("type annotations are not supported yet");
    }

    std::string name = binding->identifier()->getText();
    //Recursively construct the initialization expression
    auto initializer = buildExpression(ctx->expression());

    return std::make_unique<ast::LetStmt>(std::move(name), std::move(initializer));
}

ast::StmtPtr ASTBuilder::buildStatement(rx::Parser::StatementContext *ctx){
    if(ctx->letStatement() != nullptr){
        return buildLet(ctx->letStatement());
    }

    throw std::runtime_error("only let statements are supported now");
}

ast::ExprPtr ASTBuilder::buildExpression(rx::Parser::ExpressionContext *ctx){
    auto *assignment = ctx->assignmentExpression();
    requireSingleChild(assignment);

    auto *logicalOr = assignment->logicalOrExpression();
    requireSingleChild(logicalOr);

    auto *logicalAnd = logicalOr->logicalAndExpression(0);
    requireSingleChild(logicalAnd);

    auto *comparison = logicalAnd->comparisonExpression(0);
    requireSingleChild(comparison);

    auto *bitOr = comparison->bitOrExpression(0);
    requireSingleChild(bitOr);

    auto *bitXor = bitOr->bitXorExpression(0);
    requireSingleChild(bitXor);

    auto *bitAnd = bitXor->bitAndExpression(0);
    requireSingleChild(bitAnd);

    auto *shift = bitAnd->shiftExpression(0);
    requireSingleChild(shift);

    auto *additive = shift->additiveExpression(0);
    requireSingleChild(additive);

    auto *multiplicative = additive->multiplicativeExpression(0);
    requireSingleChild(multiplicative);

    auto *cast = multiplicative->castExpression(0);
    requireSingleChild(cast);

    auto *unary = cast->unaryExpression();
    requireSingleChild(unary);

    auto *postfix = unary->postfixExpression();
    requireSingleChild(postfix);

    auto *primary = postfix->primaryExpression();
    requireSingleChild(primary);

    auto *nonBlock = primary->nonBlockPrimary();
    if (nonBlock == nullptr || nonBlock->literalExpression() == nullptr) {
        throw std::runtime_error(
            "only literal expressions are supported for now"
        );
    }

    return buildLiteral(nonBlock->literalExpression());
}

ast::ExprPtr ASTBuilder::buildLiteral(rx::Parser::LiteralExpressionContext *ctx){
    auto integer = ctx->INTEGER_LITERAL();
    if(integer == nullptr){
        throw std::runtime_error{
             "only integer literals are supported for now"
        };
    }
    std::string text = integer->getText();

    // 本次仅支持由十进制数字组成的字面量。
    // 暂不处理进制前缀、下划线和类型后缀。
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos) {
        throw std::runtime_error(
            "only unsuffixed decimal integer literals are supported for now"
        );
    }

    return std::make_unique<ast::IntegerLiteralExpr>(
        std::move(text)
    );
}

std::unique_ptr<ast::Crate> ASTBuilder::build(rx::Parser::CrateContext *ctx){
    auto result = std::make_unique<ast::Crate>();
    for(auto itemCtx : ctx->item()){
        result->addItem(buildItem(itemCtx));
    }
    return result;
}

}