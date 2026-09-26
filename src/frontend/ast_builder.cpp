#include"ast_builder.hpp"

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
    // std::cerr
    // << "block text: " << ctx->getText() << '\n'
    // << "statement count: " << ctx->statement().size() << '\n'
    // << "has tail expression: "
    // << (ctx->statementExpression() != nullptr)
    // << '\n';

    //just empty block first
    if(!ctx->statement().empty() || ctx->statementExpression() != nullptr){
        throw std::runtime_error{ "minimal AST currently supports only empty blocks"};
    }

    return std::make_unique<ast::BlockExpr>();
}

std::unique_ptr<ast::Crate> ASTBuilder::build(rx::Parser::CrateContext *ctx){
    auto result = std::make_unique<ast::Crate>();
    for(auto itemCtx : ctx->item()){
        result->addItem(buildItem(itemCtx));
    }
    return result;
}
}