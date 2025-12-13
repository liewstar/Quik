#ifndef QuikContext_H
#define QuikContext_H

#include "Quik/QuikAPI.h"
#include "parser/ExpressionParser.h"
#include <QObject>
#include <QWidget>
#include <QVariantMap>
#include <QMap>
#include <QSet>
#include <functional>

namespace Quik {

/**
 * @brief 属性绑定信息
 */
struct PropertyBinding {
    QWidget* widget;            // 目标组件
    QString property;           // 绑定的属性名 (visible, enabled, text等)
    QString expression;         // 表达式字符串
    Condition condition;        // 解析后的条件
};

/**
 * @brief 响应式上下文管理器
 * 负责管理变量、依赖追踪和响应式更新
 */
class QUIK_API QuikContext : public QObject {
    Q_OBJECT
    
public:
    explicit QuikContext(QObject* parent = nullptr);
    ~QuikContext();
    
    // ========== 变量管理 ==========
    
    /**
     * @brief 注册变量（关联到组件）
     * @param name 变量名
     * @param widget 关联的组件
     */
    void registerVariable(const QString& name, QWidget* widget);
    
    /**
     * @brief 设置变量值
     * @param name 变量名
     * @param value 变量值
     */
    void setValue(const QString& name, const QVariant& value);
    
    /**
     * @brief 获取变量值
     * @param name 变量名
     * @return 变量值
     */
    QVariant getValue(const QString& name) const;
    
    /**
     * @brief 获取关联的组件
     * @param name 变量名
     * @return 关联的组件指针
     */
    QWidget* getWidget(const QString& name) const;
    
    // ========== 属性绑定 ==========
    
    /**
     * @brief 绑定visible属性到表达式
     * @param widget 目标组件
     * @param expression 表达式，如 "$chkStitch==0"
     */
    void bindVisible(QWidget* widget, const QString& expression);
    
    /**
     * @brief 绑定enabled属性到表达式
     * @param widget 目标组件
     * @param expression 表达式
     */
    void bindEnabled(QWidget* widget, const QString& expression);
    
    /**
     * @brief 通用属性绑定
     * @param widget 目标组件
     * @param property 属性名
     * @param expression 表达式
     */
    void bindProperty(QWidget* widget, const QString& property, const QString& expression);
    
    // ========== 响应式更新 ==========
    
    /**
     * @brief 触发所有绑定的初始化更新
     */
    void initializeBindings();
    
    /**
     * @brief 获取所有变量的当前上下文
     * @return 变量名到值的映射
     */
    QVariantMap getContext() const;
    
    // ========== 单变量监听 ==========
    
    /**
     * @brief 监听单个变量的变化
     * @param name 变量名
     * @param callback 回调函数，参数为新值
     * 
     * 使用示例：
     * @code
     * context->watch("count", [](const QVariant& value) {
     *     qDebug() << "count changed to:" << value.toInt();
     * });
     * @endcode
     */
    void watch(const QString& name, std::function<void(const QVariant&)> callback);
    
    /**
     * @brief 取消监听变量
     * @param name 变量名
     */
    void unwatch(const QString& name);
    
signals:
    /**
     * @brief 变量值改变信号
     * @param name 变量名
     * @param value 新值
     */
    void variableChanged(const QString& name, const QVariant& value);
    
public slots:
    /**
     * @brief 处理变量值改变
     * @param name 变量名
     * @param value 新值
     */
    void onVariableChanged(const QString& name, const QVariant& value);
    
private:
    /**
     * @brief 更新依赖于指定变量的所有绑定
     * @param varName 变量名
     */
    void updateDependentBindings(const QString& varName);
    
    /**
     * @brief 应用绑定到组件
     * @param binding 绑定信息
     */
    void applyBinding(const PropertyBinding& binding);
    
    /**
     * @brief 自动连接组件的值变化信号
     * @param name 变量名
     * @param widget 组件
     */
    void autoConnectWidget(const QString& name, QWidget* widget);
    
    /**
     * @brief 从变量值同步更新UI组件（双向绑定：C++ → UI）
     * @param name 变量名
     * @param value 变量值
     */
    void syncWidgetFromValue(const QString& name, const QVariant& value);
    
    /**
     * @brief 同步单个组件的值
     * @param widget 组件
     * @param value 变量值
     */
    void syncSingleWidget(QWidget* widget, const QVariant& value);
    
private:
    QVariantMap m_values;                                    // 变量值存储
    QMap<QString, QList<QWidget*>> m_widgets;                // 变量名 → 组件列表（支持多个组件绑定同一变量）
    QMap<QString, QList<PropertyBinding>> m_dependencies;    // 变量名 → 依赖它的绑定列表
    QList<PropertyBinding> m_allBindings;                    // 所有绑定
    
    // 单变量监听
    QMap<QString, std::function<void(const QVariant&)>> m_watchers;  // 变量名 → 监听回调
};

} // namespace Quik

#endif // QuikContext_H
