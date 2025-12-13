#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "Quik/QuikAPI.h"
#include <QString>
#include <QVariant>
#include <QVariantMap>

namespace Quik {

/**
 * @brief 条件表达式结构体
 * 用于存储解析后的表达式，如 "$varName==value" 或 "$var1==$var2"
 */
struct QUIK_API Condition {
    QString variable;           // 左侧变量名（不含$前缀）
    QString op;                 // 运算符: ==, !=, >, <, >=, <=
    QVariant compareValue;      // 比较值（如果右侧是固定值）
    QString compareVariable;    // 右侧变量名（如果右侧也是变量，不含$前缀）
    bool isRightVariable = false;  // 右侧是否是变量
    bool isValid = false;       // 是否解析成功
};

/**
 * @brief 复合表达式结构体
 * 支持 and/or 逻辑组合，如 "$a==1 and $b==2" 或 "$x==on or $y==on"
 */
struct QUIK_API CompoundCondition {
    QList<Condition> conditions;  // 条件列表
    QStringList logicOps;         // 逻辑运算符列表 ("and" 或 "or")
    bool isCompound = false;      // 是否是复合表达式
    bool isValid = false;
};

/**
 * @brief 表达式解析器
 * 负责解析和求值条件表达式，如 visible="$chkStitch==0"
 */
class QUIK_API ExpressionParser {
public:
    /**
     * @brief 解析条件表达式
     * @param expr 表达式字符串，如 "$msEnergyState==On" 或 "$count>5"
     * @return 解析后的条件结构体
     */
    static Condition parse(const QString& expr);
    
    /**
     * @brief 解析复合条件表达式
     * @param expr 表达式字符串，支持 and/or，如 "$a==1 and $b==2"
     * @return 解析后的复合条件结构体
     */
    static CompoundCondition parseCompound(const QString& expr);
    
    /**
     * @brief 求值条件表达式
     * @param condition 已解析的条件
     * @param context 变量上下文，存储所有变量的当前值
     * @return 表达式求值结果（true/false）
     */
    static bool evaluate(const Condition& condition, const QVariantMap& context);
    
    /**
     * @brief 直接求值表达式字符串
     * @param expr 表达式字符串
     * @param context 变量上下文
     * @return 表达式求值结果
     */
    static bool evaluate(const QString& expr, const QVariantMap& context);
    
    /**
     * @brief 求值复合条件表达式
     * @param compound 已解析的复合条件
     * @param context 变量上下文
     * @return 表达式求值结果
     */
    static bool evaluate(const CompoundCondition& compound, const QVariantMap& context);
    
    /**
     * @brief 检查字符串是否是表达式（以$开头）
     * @param str 待检查的字符串
     * @return 是否是表达式
     */
    static bool isExpression(const QString& str);
    
    /**
     * @brief 从表达式中提取变量名
     * @param expr 表达式字符串
     * @return 变量名列表
     */
    static QStringList extractVariables(const QString& expr);

private:
    static bool compareValues(const QVariant& left, const QString& op, const QVariant& right);
    static bool evaluateWithParentheses(const QString& expr, const QVariantMap& context);
};

} // namespace Quik

#endif // EXPRESSIONPARSER_H
