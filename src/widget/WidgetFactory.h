#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "Quik/QuikAPI.h"
#include <QWidget>
#include <QtXml/QDomElement>
#include <QMap>
#include <functional>

namespace Quik {

class QuikContext;

/**
 * @brief 组件创建函数类型
 * @param element XML元素
 * @param context 响应式上下文
 * @return 创建的组件
 */
using WidgetCreator = std::function<QWidget*(const QDomElement&, QuikContext*)>;

/**
 * @brief 组件工厂
 * 负责根据XML标签名创建对应的Qt组件
 */
class QUIK_API WidgetFactory {
public:
    /**
     * @brief 获取单例实例
     */
    static WidgetFactory& instance();
    
    /**
     * @brief 注册组件创建器
     * @param tagName XML标签名
     * @param creator 创建函数
     */
    void registerCreator(const QString& tagName, WidgetCreator creator);
    
    /**
     * @brief 创建组件
     * @param tagName XML标签名
     * @param element XML元素
     * @param context 响应式上下文
     * @return 创建的组件，如果标签未注册则返回nullptr
     */
    QWidget* create(const QString& tagName, const QDomElement& element, QuikContext* context);
    
    /**
     * @brief 检查标签是否已注册
     * @param tagName XML标签名
     * @return 是否已注册
     */
    bool hasCreator(const QString& tagName) const;
    
    /**
     * @brief 注册所有内置组件
     */
    void registerBuiltinWidgets();
    
private:
    WidgetFactory();
    ~WidgetFactory() = default;
    
    // 禁止拷贝
    WidgetFactory(const WidgetFactory&) = delete;
    WidgetFactory& operator=(const WidgetFactory&) = delete;
    
    // ========== 内置组件创建器 ==========
    static QWidget* createLabel(const QDomElement& element, QuikContext* context);
    static QWidget* createLineEdit(const QDomElement& element, QuikContext* context);
    static QWidget* createCheckBox(const QDomElement& element, QuikContext* context);
    static QWidget* createComboBox(const QDomElement& element, QuikContext* context);
    static QWidget* createSpinBox(const QDomElement& element, QuikContext* context);
    static QWidget* createDoubleSpinBox(const QDomElement& element, QuikContext* context);
    static QWidget* createPushButton(const QDomElement& element, QuikContext* context);
    static QWidget* createGroupBox(const QDomElement& element, QuikContext* context);
    static QWidget* createInnerGroupBox(const QDomElement& element, QuikContext* context);
    static QWidget* createRadioButton(const QDomElement& element, QuikContext* context);
    static QWidget* createHLine(const QDomElement& element, QuikContext* context);
    static QWidget* createVLine(const QDomElement& element, QuikContext* context);
    static QWidget* createLabelList(const QDomElement& element, QuikContext* context);
    static QWidget* createListBox(const QDomElement& element, QuikContext* context);
    static QWidget* createTabBar(const QDomElement& element, QuikContext* context);
    static QWidget* createHLayoutWidget(const QDomElement& element, QuikContext* context);
    static QWidget* createVLayoutWidget(const QDomElement& element, QuikContext* context);
    static QWidget* createStretch(const QDomElement& element, QuikContext* context);
    static QWidget* createPointLineEdit(const QDomElement& element, QuikContext* context);
    static QWidget* createTwoPointLineEdit(const QDomElement& element, QuikContext* context);
    static QWidget* createSlider(const QDomElement& element, QuikContext* context);
    static QWidget* createProgressBar(const QDomElement& element, QuikContext* context);
    
    // ========== 辅助方法 ==========
    static void applyCommonAttributes(QWidget* widget, const QDomElement& element, QuikContext* context);
    static QString getAttribute(const QDomElement& element, const QString& name, const QString& defaultValue = QString());
    static bool getBoolAttribute(const QDomElement& element, const QString& name, bool defaultValue = false);
    static int getIntAttribute(const QDomElement& element, const QString& name, int defaultValue = 0);
    static double getDoubleAttribute(const QDomElement& element, const QString& name, double defaultValue = 0.0);
    
private:
    QMap<QString, WidgetCreator> m_creators;
    bool m_initialized = false;
};

} // namespace Quik

#endif // WIDGETFACTORY_H
