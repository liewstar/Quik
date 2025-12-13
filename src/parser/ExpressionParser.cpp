#include "ExpressionParser.h"
#include <QRegularExpression>
#include <QDebug>

namespace Quik {

Condition ExpressionParser::parse(const QString& expr) {
    Condition cond;
    
    if (expr.isEmpty()) {
        return cond;
    }
    
    QString cleanExpr = expr.trimmed();
    
    // 移除左侧$前缀
    if (cleanExpr.startsWith("$")) {
        cleanExpr = cleanExpr.mid(1);
    }
    
    // 支持的运算符（按长度降序排列，确保先匹配长的）
    static const QStringList operators = {"==", "!=", ">=", "<=", ">", "<"};
    
    for (const QString& op : operators) {
        int pos = cleanExpr.indexOf(op);
        if (pos > 0) {
            cond.variable = cleanExpr.left(pos).trimmed();
            cond.op = op;
            QString rightSide = cleanExpr.mid(pos + op.length()).trimmed();
            
            // 检查右侧是否也是变量（以$开头）
            if (rightSide.startsWith("$")) {
                cond.isRightVariable = true;
                cond.compareVariable = rightSide.mid(1);  // 移除$前缀
            } else {
                cond.isRightVariable = false;
                // 尝试转换为数值
                bool ok;
                double numValue = rightSide.toDouble(&ok);
                if (ok) {
                    cond.compareValue = numValue;
                } else {
                    cond.compareValue = rightSide;
                }
            }
            
            cond.isValid = true;
            break;
        }
    }
    
    return cond;
}

bool ExpressionParser::evaluate(const Condition& condition, const QVariantMap& context) {
    if (!condition.isValid) {
        qWarning() << "[Quik] Invalid condition";
        return false;
    }
    
    // 从上下文获取左侧变量值
    QVariant leftValue = context.value(condition.variable);
    
    if (!leftValue.isValid()) {
        qWarning() << "[Quik] Variable not found:" << condition.variable;
        return false;
    }
    
    // 获取右侧值（可能是固定值，也可能是变量）
    QVariant rightValue;
    if (condition.isRightVariable) {
        // 右侧是变量，从上下文获取
        rightValue = context.value(condition.compareVariable);
        if (!rightValue.isValid()) {
            qWarning() << "[Quik] Variable not found:" << condition.compareVariable;
            return false;
        }
    } else {
        // 右侧是固定值
        rightValue = condition.compareValue;
    }
    
    return compareValues(leftValue, condition.op, rightValue);
}

bool ExpressionParser::evaluate(const QString& expr, const QVariantMap& context) {
    QString cleanExpr = expr.trimmed();
    
    // 支持括号表达式：递归处理括号
    if (cleanExpr.contains('(')) {
        return evaluateWithParentheses(cleanExpr, context);
    }
    
    // 检查是否包含 and/or 逻辑运算符
    if (cleanExpr.contains(" and ", Qt::CaseInsensitive) || 
        cleanExpr.contains(" or ", Qt::CaseInsensitive)) {
        CompoundCondition compound = parseCompound(expr);
        return evaluate(compound, context);
    }
    
    Condition cond = parse(expr);
    return evaluate(cond, context);
}

bool ExpressionParser::evaluateWithParentheses(const QString& expr, const QVariantMap& context) {
    QString result = expr;
    
    // 循环处理最内层括号
    while (result.contains('(')) {
        // 找到最内层的括号
        int closePos = result.indexOf(')');
        if (closePos < 0) {
            qWarning() << "[Quik] Mismatched parentheses in expression:" << expr;
            return false;
        }
        
        int openPos = result.lastIndexOf('(', closePos);
        if (openPos < 0) {
            qWarning() << "[Quik] Mismatched parentheses in expression:" << expr;
            return false;
        }
        
        // 提取括号内的子表达式
        QString subExpr = result.mid(openPos + 1, closePos - openPos - 1);
        
        // 递归求值子表达式
        bool subResult = evaluate(subExpr, context);
        
        // 用结果替换括号表达式
        result = result.left(openPos) + (subResult ? "1" : "0") + result.mid(closePos + 1);
    }
    
    // 处理剩余的表达式
    return evaluate(result, context);
}

CompoundCondition ExpressionParser::parseCompound(const QString& expr) {
    CompoundCondition compound;
    QString cleanExpr = expr.trimmed();
    
    // 使用正则表达式分割 and/or
    QRegularExpression re("\\s+(and|or)\\s+", QRegularExpression::CaseInsensitiveOption);
    QStringList parts = cleanExpr.split(re);
    
    // 提取逻辑运算符
    QRegularExpressionMatchIterator it = re.globalMatch(cleanExpr);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        compound.logicOps.append(match.captured(1).toLower());
    }
    
    // 解析每个条件
    for (const QString& part : parts) {
        Condition cond = parse(part.trimmed());
        if (cond.isValid) {
            compound.conditions.append(cond);
        }
    }
    
    compound.isCompound = compound.conditions.size() > 1;
    compound.isValid = !compound.conditions.isEmpty();
    
    return compound;
}

bool ExpressionParser::evaluate(const CompoundCondition& compound, const QVariantMap& context) {
    if (!compound.isValid || compound.conditions.isEmpty()) {
        return false;
    }
    
    // 如果只有一个条件，直接求值
    if (compound.conditions.size() == 1) {
        return evaluate(compound.conditions.first(), context);
    }
    
    // 求值第一个条件
    bool result = evaluate(compound.conditions.first(), context);
    
    // 依次应用逻辑运算符
    for (int i = 0; i < compound.logicOps.size() && i + 1 < compound.conditions.size(); ++i) {
        bool nextResult = evaluate(compound.conditions[i + 1], context);
        
        if (compound.logicOps[i] == "and") {
            result = result && nextResult;
        } else if (compound.logicOps[i] == "or") {
            result = result || nextResult;
        }
    }
    
    return result;
}

bool ExpressionParser::isExpression(const QString& str) {
    return str.trimmed().startsWith("$");
}

QStringList ExpressionParser::extractVariables(const QString& expr) {
    QStringList vars;
    
    // 使用正则表达式提取所有 $varName 形式的变量
    QRegularExpression re("\\$([a-zA-Z_][a-zA-Z0-9_]*)");
    QRegularExpressionMatchIterator it = re.globalMatch(expr);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = match.captured(1);
        if (!vars.contains(varName)) {
            vars.append(varName);
        }
    }
    
    return vars;
}

bool ExpressionParser::compareValues(const QVariant& left, const QString& op, const QVariant& right) {
    // 优先尝试数值比较（支持 >, <, >=, <= 等运算符）
    bool leftOk, rightOk;
    double leftNum = left.toDouble(&leftOk);
    double rightNum = right.toDouble(&rightOk);
    
    if (leftOk && rightOk) {
        // 数值比较
        if (op == "==") {
            // 处理浮点数精度问题
            if (leftNum == 0.0 && rightNum == 0.0) return true;
            return qFuzzyCompare(leftNum, rightNum);
        }
        if (op == "!=") {
            if (leftNum == 0.0 && rightNum == 0.0) return false;
            return !qFuzzyCompare(leftNum, rightNum);
        }
        if (op == ">")  return leftNum > rightNum;
        if (op == "<")  return leftNum < rightNum;
        if (op == ">=") return leftNum >= rightNum;
        if (op == "<=") return leftNum <= rightNum;
    }
    
    // 字符串比较
    QString leftStr = left.toString();
    QString rightStr = right.toString();
    
    if (op == "==") return leftStr == rightStr;
    if (op == "!=") return leftStr != rightStr;
    if (op == ">")  return leftStr > rightStr;
    if (op == "<")  return leftStr < rightStr;
    if (op == ">=") return leftStr >= rightStr;
    if (op == "<=") return leftStr <= rightStr;
    
    return false;
}

} // namespace Quik
