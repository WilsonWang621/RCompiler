#include "ast.hpp"

namespace{
    void printIndent(std::ostream &out, int indent){
        for(int i = 0; i < indent; i++){
            out << " ";
        }
    }
}
namespace rx::ast{
    void BlockExpr::dump(std::ostream &out, int indent) const {
        printIndent(out, indent);
        out << "Block\n";
        for (const auto &statement : stmts_) {
            statement->dump(out, indent + 1);
        }
    }

    void FunctionItem::dump(std::ostream &out, int indent) const {
        printIndent(out, indent);
        out << "Function: " << name_ << '\n';

        body_->dump(out, indent + 1);
    }

    void Crate::dump(std::ostream &out, int indent) const{
        printIndent(out, indent);
        out << "Crate\n";
        for(const auto &item : items){
            item->dump(out, indent + 1);
        }
    }

    void LetStmt::dump(std::ostream &out, int indent) const{
        printIndent(out, indent);

        out << "LetStmt: ";
        if (isMutable_) {
            out << "mut ";
        }
        out << name_ << '\n';

        initializer_->dump(out, indent + 1);
    }

    void IntegerLiteralExpr::dump(std::ostream &out, int indent) const{
        printIndent(out, indent);
        out << "IntegerLiteral: " << text_ << '\n';
    }

    void BinaryExpr::dump(std::ostream &out, int indent) const {
        printIndent(out, indent);
        out << "BinaryExpr: " << op_ << '\n';

        left_->dump(out, indent + 1);
        right_->dump(out, indent + 1);
    }

    void UnaryExpr::dump(std::ostream &out, int indent) const{
        printIndent(out, indent);
        out << "UnaryExpr: " << op_ << '\n';

        operand_->dump(out, indent + 1);
    }

    void PathExpr::dump(std::ostream &out, int indent) const{
        printIndent(out, indent);
        out << "PathExpr: ";

        for (std::size_t i = 0; i < segments_.size(); ++i) {
            if (i != 0) {
                out << "::";
            }

            out << segments_[i];
        }

        out << '\n'; 
    }

    void ExprStmt::dump(std::ostream &out, int indent) const{
        printIndent(out, indent);
        out << "ExprStmt:\n";
        expression_->dump(out, indent + 1);
    }

    void AssignExpr::dump(std::ostream &out, int indent) const {
        printIndent(out, indent);
        out << "AssignExpr\n";

        // 第一个孩子是赋值目标，第二个孩子是右侧的值。
        target_->dump(out, indent + 1);
        value_->dump(out, indent + 1);
    }
}