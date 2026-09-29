#include"ast_builder.hpp"

#include <stdexcept>

namespace {

// 本次只允许穿过单孩子的表达式规则。
void requireSingleChild(antlr4::ParserRuleContext *ctx) {
    if (ctx == nullptr || ctx->children.size() != 1) {
        throw std::runtime_error(
           "this expression form is not supported yet"
        );
    }
}

} // namespace

namespace rx::frontend{

std::unique_ptr<ast::Crate> ASTBuilder::build(rx::Parser::CrateContext *ctx){
    auto result = std::make_unique<ast::Crate>();
    for(auto itemCtx : ctx->item()){
        result->addItem(buildItem(itemCtx));
    }
    return result;
}
    
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
    auto block = std::make_unique<ast::BlockExpr>();

    for (auto statementCtx : ctx->statement()) {
        auto statement = buildStatement(statementCtx);
        block->addStatement(std::move(statement));
    }
    if( ctx->statementExpression() != nullptr){
        block->setTail(buildStatementExpression(ctx->statementExpression()));
    }
    

    return block;
}

std::unique_ptr<ast::LetStmt> ASTBuilder::buildLet(rx::Parser::LetStatementContext *ctx){
    auto *binding = ctx->identifierBinding();

    if (ctx->typeRef() != nullptr) {
        throw std::runtime_error("type annotations are not supported yet");
    }

    std::string name = binding->identifier()->getText();
    bool isMutable = binding->MUT() != nullptr;
    //Recursively construct the initialization expression
    auto initializer = buildExpression(ctx->expression());

    return std::make_unique<ast::LetStmt>(std::move(name), isMutable, std::move(initializer));
}

ast::StmtPtr ASTBuilder::buildStatement(rx::Parser::StatementContext *ctx){
    if(ctx->letStatement() != nullptr){
        return buildLet(ctx->letStatement());
    }

    if(ctx->statementExpression() != nullptr){
        auto expression = buildStatementExpression(ctx->statementExpression());
        return std::make_unique<ast::ExprStmt>(std::move(expression));
    }

    if(ctx->expressionWithBlock() != nullptr){
        auto withBlock = ctx->expressionWithBlock();
        // 本次只支持普通块。
        if (withBlock->ifExpression() != nullptr || withBlock->LOOP() != nullptr || withBlock->WHILE() != nullptr) {
            throw std::runtime_error{
                "if, loop and while are not supported yet"
            };
        }
        if(withBlock->blockExpression() != nullptr){
            auto block = buildBlock(withBlock->blockExpression());
            return std::make_unique<ast::ExprStmt>(
                std::move(block)
            );
        }
    }

    throw std::runtime_error("this statement form is not supported yet");
}

ast::ExprPtr ASTBuilder::buildExpression(rx::Parser::ExpressionContext *ctx){
    auto *assignment = ctx->assignmentExpression();

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

    auto left = buildAdditive(shift->additiveExpression(0));

    auto *op = assignment->assignmentOperator();

    // 没有赋值运算，直接返回原表达式。
    if (op == nullptr) {
        return left;
    }

    // 本次只支持 =，暂不支持 += 等复合赋值。
    if (op->equalsSign() == nullptr) {
        throw std::runtime_error(
            "compound assignment is not supported yet"
        );
    }

    // 右侧是完整 expression，递归构造。
    auto right = buildExpression(assignment->expression());

    return std::make_unique<ast::AssignExpr>(
        std::move(left),
        std::move(right)
    );
}

ast::ExprPtr ASTBuilder::buildLiteral(rx::Parser::LiteralExpressionContext *ctx){
    if(ctx->TRUE() != nullptr || ctx->FALSE() != nullptr){
        bool flag = ctx->TRUE() != nullptr;

        return std::make_unique<ast::BooleanLiteralExpr>(flag);
    }

    auto integer = ctx->INTEGER_LITERAL();
    if(integer != nullptr){
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

    throw std::runtime_error(
        "only integer and boolean literals are supported for now"
    );
}

ast::ExprPtr ASTBuilder::buildAdditive(rx::Parser::AdditiveExpressionContext *ctx){
    auto operands = ctx->multiplicativeExpression();
    auto operators = ctx->additiveOperator();

    // 先构造第一个操作数。
    auto result = buildMultiplicative(operands[0]);

    // 从左到右，逐次把已有结果作为新的左孩子。
    for (std::size_t i = 0; i < operators.size(); ++i) {
        std::string op = operators[i]->getText();
        auto right = buildMultiplicative(operands[i + 1]);

        result = std::make_unique<ast::BinaryExpr>(
            std::move(op),
            std::move(result),
            std::move(right)
        );
    }

    return result;
}

ast::ExprPtr ASTBuilder::buildMultiplicative(rx::Parser::MultiplicativeExpressionContext *ctx) {
    auto operands = ctx->castExpression();
    auto operators = ctx->multiplicativeOperator();

    auto result = buildCast(operands[0]);

    for (std::size_t i = 0; i < operators.size(); ++i) {
        std::string op = operators[i]->getText();
        auto right = buildCast(operands[i + 1]);

        result = std::make_unique<ast::BinaryExpr>(
            std::move(op),
            std::move(result),
            std::move(right)
        );
    }

    return result;
}

ast::ExprPtr ASTBuilder::buildCast(rx::Parser::CastExpressionContext *ctx) {
    if (!ctx->typeRef().empty()) {
        throw std::runtime_error("as casts are not supported yet");
    }

    return buildUnary(ctx->unaryExpression());
}

ast::ExprPtr ASTBuilder::buildUnary(rx::Parser::UnaryExpressionContext *ctx){
    //two branches 1)has prefix operator, build recursively  2) otherwise give it to next floor postfixExpression directly
    if(ctx->unaryOperator() != nullptr){
        std::string op = ctx->unaryOperator()->getText();

        // std::cerr << op << '\n';

        // 本次支持取负和取反。
        // 解引用、借用等操作留到后面。
        if(op != "-" && op != "!"){
            throw std::runtime_error{"this unary is not supported yet"};
        }

        auto operand = buildUnary(ctx->unaryExpression());

        return std::make_unique<ast::UnaryExpr>(
            std::move(op),
            std::move(operand)
        );
    }
    return buildPostfix(ctx->postfixExpression());
}

ast::ExprPtr ASTBuilder::buildPostfix(rx::Parser::PostfixExpressionContext *ctx){
    if(!ctx->postfixSuffix().empty()){
        throw std::runtime_error{"postfix operations are not supported yet"};
    }

    return buildPrimary(ctx->primaryExpression());
}

ast::ExprPtr ASTBuilder::buildPrimary(rx::Parser::PrimaryExpressionContext *ctx) {
    auto nonBlock = ctx->nonBlockPrimary();

    if (nonBlock != nullptr) {
        return buildNonBlockPrimary(nonBlock);
    }

    auto withBlock = ctx->expressionWithBlock();

    if (withBlock != nullptr) {
        if (withBlock->ifExpression() != nullptr || withBlock->LOOP() != nullptr || withBlock->WHILE() != nullptr) {
            throw std::runtime_error{
                "if, loop and while are not supported yet"
            };
        }

        auto block = withBlock->blockExpression();

        if (block != nullptr) {
            return buildBlock(block);
        }
    }

    throw std::runtime_error{
        "unsupported primary expression"
    };
}

ast::ExprPtr ASTBuilder::buildPath(rx::Parser::PathInExpressionContext *ctx){
    auto segmentContexts = ctx->pathExprSegment();

    //just support single path for the time being
    if(segmentContexts.size() != 1){
        throw std::runtime_error{
            "only single-segment paths are supported for now"
        };
    }
    auto segmentCtx = segmentContexts[0];
    // 暂不支持带泛型参数的路径，例如 foo::<i32>。
    if (segmentCtx->genericArgs() != nullptr) {
        throw std::runtime_error(
            "generic arguments in paths are not supported yet"
        );
    }

    auto identCtx = segmentCtx->pathIdentSegment();

    // pathIdentSegment 也允许 self 和 Self。
    // 本关只接受普通 identifier。
    if (identCtx->identifier() == nullptr) {
        throw std::runtime_error(
            "self and Self paths are not supported yet"
        );
    }

    std::string name = identCtx->identifier()->getText();

    std::vector<std::string> segments;
    segments.push_back(std::move(name));

    return std::make_unique<ast::PathExpr>(
        std::move(segments)
    );
}

ast::ExprPtr ASTBuilder::buildStatementExpression(rx::Parser::StatementExpressionContext *ctx){
    auto *assignment = ctx->statementAssignmentExpression();

    auto *logicalOr = assignment->statementLogicalOrExpression();
    requireSingleChild(logicalOr);

    auto *logicalAnd = logicalOr->statementLogicalAndExpression();
    requireSingleChild(logicalAnd);

    auto *comparison = logicalAnd->statementComparisonExpression();
    requireSingleChild(comparison);

    auto *bitOr = comparison->statementBitOrExpression();
    requireSingleChild(bitOr);

    auto *bitXor = bitOr->statementBitXorExpression();
    requireSingleChild(bitXor);

    auto *bitAnd = bitXor->statementBitAndExpression();
    requireSingleChild(bitAnd);

    auto *shift = bitAnd->statementShiftExpression();
    requireSingleChild(shift);

    auto left = buildStatementAdditive(shift->statementAdditiveExpression());

    auto *op = assignment->assignmentOperator();

    if (op == nullptr) {
        return left;
    }

    if (op->equalsSign() == nullptr) {
        throw std::runtime_error(
            "compound assignment is not supported yet"
        );
    }

    // 注意：右侧回到普通 expression 入口
    auto right = buildExpression(assignment->expression());

    return std::make_unique<ast::AssignExpr>(
        std::move(left),
        std::move(right)
    );
}

ast::ExprPtr ASTBuilder::buildStatementAdditive(rx::Parser::StatementAdditiveExpressionContext *ctx) {
    auto result = buildStatementMultiplicative(ctx->statementMultiplicativeExpression());

    auto operators = ctx->additiveOperator();
    auto operands = ctx->multiplicativeExpression();

    for (std::size_t i = 0; i < operators.size(); ++i) {
        std::string op = operators[i]->getText();
        auto right = buildMultiplicative(operands[i]);

        result = std::make_unique<ast::BinaryExpr>(
            std::move(op),
            std::move(result),
            std::move(right)
        );
    }

    return result;
}

ast::ExprPtr ASTBuilder::buildStatementMultiplicative(rx::Parser::StatementMultiplicativeExpressionContext *ctx) {
    auto result = buildStatementCast(ctx->statementCastExpression());

    auto operators = ctx->multiplicativeOperator();
    auto operands = ctx->castExpression();

    for (std::size_t i = 0; i < operators.size(); ++i) {
        std::string op = operators[i]->getText();
        auto right = buildCast(operands[i]);

        result = std::make_unique<ast::BinaryExpr>(
            std::move(op),
            std::move(result),
            std::move(right)
        );
    }

    return result;
}

ast::ExprPtr ASTBuilder::buildStatementCast(rx::Parser::StatementCastExpressionContext *ctx) {
    if (!ctx->typeRef().empty()) {
        throw std::runtime_error(
            "as casts are not supported yet"
        );
    }

    return buildStatementUnary(ctx->statementUnaryExpression());
}

ast::ExprPtr ASTBuilder::buildStatementUnary(rx::Parser::StatementUnaryExpressionContext *ctx) {
    if (ctx->unaryOperator() != nullptr) {
        std::string op = ctx->unaryOperator()->getText();

        if (op != "-" && op != "!") {
            throw std::runtime_error(
                "this unary operator is not supported yet"
            );
        }

        // 语法规定：前缀运算符后面使用普通 unaryExpression。
        auto operand = buildUnary(ctx->unaryExpression());

        return std::make_unique<ast::UnaryExpr>(
            std::move(op),
            std::move(operand)
        );
    }

    return buildStatementPostfix(
        ctx->statementPostfixExpression()
    );
}

ast::ExprPtr ASTBuilder::buildStatementPostfix(rx::Parser::StatementPostfixExpressionContext *ctx) {
    if (ctx->expressionWithBlock() != nullptr ||
        ctx->dotSuffix() != nullptr ||
        !ctx->postfixSuffix().empty()) {
        throw std::runtime_error(
            "block-leading or postfix expressions are not supported yet"
        );
    }

    return buildNonBlockPrimary(ctx->nonBlockPrimary());
}

ast::ExprPtr ASTBuilder::buildNonBlockPrimary(rx::Parser::NonBlockPrimaryContext *ctx){
    if (ctx == nullptr) {
        throw std::runtime_error(
            "expected a non-block primary expression"
        );
    }

    if (ctx->literalExpression() != nullptr) {
        return buildLiteral(ctx->literalExpression());
    }

    if (ctx->pathInExpression() != nullptr) {
        if (ctx->LBRACE() != nullptr) {
            throw std::runtime_error(
                "struct construction is not supported yet"
            );
        }

        return buildPath(ctx->pathInExpression());
    }

    if (ctx->LPAREN() != nullptr) {
        auto *inner = ctx->expression();

        if (inner == nullptr) {
            throw std::runtime_error(
                "unit expression () is not supported yet"
            );
        }

        return buildExpression(inner);
    }

    throw std::runtime_error(
        "this primary expression is not supported yet"
    );
}
    // Boolean operators, comparisons, closed operands and if expressions.
ast::ExprPtr ASTBuilder::buildConditionExpression(rx::Parser::ConditionExpressionContext *ctx){
    auto assignment = ctx->conditionAssignmentExpression();
    
    auto left = buildConditionLogicalOr(assignment->conditionLogicalOrExpression());
    auto op = assignment->assignmentOperator();

    if(op == nullptr){
        return left;
    }
    if(op->equalsSign() == nullptr){
        throw std::runtime_error{"compound assignment is not supported yet"};
    }

    auto right = buildConditionExpression(assignment->conditionExpression());

    return std::make_unique<ast::AssignExpr>(
        std::move(left),
        std::move(right)
    );
}

ast::ExprPtr ASTBuilder::buildBitOr(rx::Parser::BitOrExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildBitXor(ctx->bitXorExpression(0));
}

ast::ExprPtr ASTBuilder::buildBitXor(rx::Parser::BitXorExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildBitAnd(ctx->bitAndExpression(0));
}

ast::ExprPtr ASTBuilder::buildBitAnd(rx::Parser::BitAndExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildShift(ctx->shiftExpression(0));
}

ast::ExprPtr ASTBuilder::buildShift(rx::Parser::ShiftExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildAdditive(ctx->additiveExpression(0));
}

ast::ExprPtr ASTBuilder::buildClosedBitOr(rx::Parser::ClosedBitOrExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildClosedBitXor(ctx->closedBitXorExpression());
}

ast::ExprPtr ASTBuilder::buildClosedBitXor(rx::Parser::ClosedBitXorExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildClosedBitAnd(ctx->closedBitAndExpression());
}

ast::ExprPtr ASTBuilder::buildClosedBitAnd(rx::Parser::ClosedBitAndExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildClosedShift(ctx->closedShiftExpression());
}

ast::ExprPtr ASTBuilder::buildClosedShift(rx::Parser::ClosedShiftExpressionContext *ctx){
    requireSingleChild(ctx);
    return buildClosedAdditive(ctx->closedAdditiveExpression(0));
}

ast::ExprPtr ASTBuilder::buildLogicalOr(rx::Parser::LogicalOrExpressionContext *ctx){
    auto operands = ctx->logicalAndExpression();
    auto result = buildLogicalAnd(operands[0]);

    for(std::size_t i = 1; i < operands.size(); ++i){
        auto right = buildLogicalAnd(operands[i]);
        result = std::make_unique<ast::BinaryExpr>(
            "||",
            std::move(result),
            std::move(right)
        );
    }
    return result;
}

ast::ExprPtr ASTBuilder::buildLogicalAnd(rx::Parser::LogicalAndExpressionContext *ctx){
    auto operands = ctx->comparisonExpression();
    auto result = buildComparison(operands[0]);

    for(std::size_t i = 1; i < operands.size(); ++i){
        auto right = buildComparison(operands[i]);
        result = std::make_unique<ast::BinaryExpr>(
            "&&",
            std::move(result),
            std::move(right)
        );
    }
    return result;
}

ast::ExprPtr ASTBuilder::buildComparison(rx::Parser::ComparisonExpressionContext *ctx){
    //closedBitOrExpression LT bitOrExpression
    if(ctx->LT() != nullptr){
        auto left = buildClosedBitOr(ctx->closedBitOrExpression());
        auto right = buildBitOr(ctx->bitOrExpression(0));
        
        return std::make_unique<ast::BinaryExpr>(
            "<",
            std::move(left),
            std::move(right)
        );
    }

    //bitOrExpression (comparisonExceptLt bitOrExpression)?
    auto left = buildBitOr(ctx->bitOrExpression(0));
    if(ctx->comparisonExceptLt() == nullptr){
        return left;
    }
    std::string op = ctx->comparisonExceptLt()->getText();
    auto right = buildBitOr(ctx->bitOrExpression(1));

    return std::make_unique<ast::BinaryExpr>(
        op,
        left,
        right
    );
}

ast::ExprPtr ASTBuilder::buildClosedAdditive(rx::Parser::ClosedAdditiveExpressionContext *ctx){
    auto operands = ctx->multiplicativeExpression();
    auto operators = ctx->additiveOperator();

    if(operands.empty()){
        return buildClosedMultiplicative(ctx->closedMultiplicativeExpression());
    }
    
    auto result = buildMultiplicative(operands[0]);
    for(size_t i = 0; i < operators.size(); i++){
        std::string op = operators[i]->getText();
        ast::ExprPtr right;
        if(i + 1 < operands.size()){
            right = buildMultiplicative(operands[i + 1]);
        }else{
            right = buildClosedMultiplicative(ctx->closedMultiplicativeExpression());
        }
        result = std::make_unique<ast::BinaryExpr>(
            std::move(op),
            std::move(result),
            std::move(right)
        );
    }
    return result;
}

ast::ExprPtr ASTBuilder::buildClosedMultiplicative(rx::Parser::ClosedMultiplicativeExpressionContext *ctx){
    auto operands = ctx->castExpression();
    auto operators = ctx->multiplicativeOperator();

    if(operands.empty()){
        return buildClosedCast(ctx->closedCastExpression());
    }

    auto result = buildCast(operands[0]);
    for(size_t i = 0; i < operators.size(); i++){
        std::string op = operators[i]->getText();
        ast::ExprPtr right;
        if(i + 1 < operands.size()){
            right = buildCast(operands[i + 1]);
        }
        else{
            right = buildClosedCast(ctx->closedCastExpression());
        }

        result = std::make_unique<ast::BinaryExpr>(
            op,
            result,
            right
        );
    }
    return result;
}

ast::ExprPtr ASTBuilder::buildClosedCast(rx::Parser::ClosedCastExpressionContext *ctx){
    if(ctx->unaryExpression() == nullptr){
        throw std::runtime_error{"as casts are not supported yet"};
    }
    return buildUnary(ctx->unaryExpression());
}

//statement prefix: The expression that enters from the beginning of the statement.

// ast::ExprPtr ASTBuilder::buildConditionLogicalOr(rx::Parser::ConditionLogicalOrExpressionContext *ctx){
//     auto operands = ctx->logicalAndExpression();
//     auto result = buildStatementLogicalAnd(ctx->statementLogicalAndExpression());

//     for(int i = 0; i < operands.size(); i++){
//         auto right = buildConditionComparison()
//     }

// }
}
