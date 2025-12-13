#ifndef XMLUIBUILDER_H
#define XMLUIBUILDER_H

#include "Quik/QuikAPI.h"
#include "core/QuikContext.h"
#include "widget/WidgetFactory.h"
#include <QWidget>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QString>
#include <QMap>
#include <QFileSystemWatcher>
#include <functional>

namespace Quik {

/**
 * @brief XML UI构建器
 * 负责解析XML文件并构建Qt界面
 */
class QUIK_API XMLUIBuilder : public QObject {
    Q_OBJECT
    
public:
    explicit XMLUIBuilder(QObject* parent = nullptr);
    ~XMLUIBuilder();
    
    /**
     * @brief 从XML文件构建UI
     * @param filePath XML文件路径
     * @param parent 父组件
     * @return 构建的根组件
     */
    QWidget* buildFromFile(const QString& filePath, QWidget* parent = nullptr);
    
    /**
     * @brief 从XML字符串构建UI
     * @param xmlContent XML内容字符串
     * @param parent 父组件
     * @return 构建的根组件
     */
    QWidget* buildFromString(const QString& xmlContent, QWidget* parent = nullptr);
    
    /**
     * @brief 获取响应式上下文
     * @return 上下文指针
     */
    QuikContext* context() const { return m_context; }
    
    /**
     * @brief 通过变量名获取组件
     * @param varName 变量名
     * @return 组件指针
     */
    QWidget* getWidget(const QString& varName) const;
    
    /**
     * @brief 获取变量值
     * @param varName 变量名
     * @return 变量值
     */
    QVariant getValue(const QString& varName) const;
    
    /**
     * @brief 设置变量值
     * @param varName 变量名
     * @param value 变量值
     */
    void setValue(const QString& varName, const QVariant& value);
    
    /**
     * @brief 连接按钮点击信号
     * @param varName 按钮变量名
     * @param callback 回调函数
     */
    void connectButton(const QString& varName, std::function<void()> callback);
    
    /**
     * @brief 获取所有变量值
     * @return 变量名到值的映射
     */
    QVariantMap getAllValues() const;
    
    // ========== 单变量监听 ==========
    
    /**
     * @brief 监听单个变量的变化
     * @param varName 变量名
     * @param callback 回调函数
     * 
     * 使用示例：
     * @code
     * builder.watch("count", [](const QVariant& v) {
     *     qDebug() << "count =" << v.toInt();
     * });
     * @endcode
     */
    void watch(const QString& varName, std::function<void(const QVariant&)> callback);
    
    /**
     * @brief 取消监听变量
     */
    void unwatch(const QString& varName);
    
    // ========== 热更新 (Hot Reload) ==========
    
    /**
     * @brief 启用热更新（仅开发模式推荐使用）
     * 
     * 监听XML文件变化，自动重新加载UI并保持状态
     * 
     * @code
     * builder.buildFromFile("MyPanel.xml");
     * #ifdef QT_DEBUG
     * builder.enableHotReload("MyPanel.xml");
     * #endif
     * @endcode
     */
    void enableHotReload(const QString& filePath);
    
    /**
     * @brief 禁用热更新
     */
    void disableHotReload();
    
    /**
     * @brief 检查热更新是否启用
     */
    bool isHotReloadEnabled() const;
    
    /**
     * @brief 手动触发重新加载
     */
    void reload();
    
    // ========== 表单验证 ==========
    
    /**
     * @brief 检查所有表单是否有效
     * @return true表示所有字段验证通过
     */
    bool isValid() const;
    
    /**
     * @brief 获取所有验证错误
     * @return 变量名到错误信息的映射
     */
    QMap<QString, QString> getValidationErrors() const;
    
signals:
    /**
     * @brief UI构建完成信号
     * @param rootWidget 根组件
     */
    void buildCompleted(QWidget* rootWidget);
    
    /**
     * @brief 构建错误信号
     * @param errorMessage 错误信息
     */
    void buildError(const QString& errorMessage);
    
    /**
     * @brief 热更新完成信号
     */
    void reloaded();
    
private slots:
    void onFileChanged(const QString& path);
    
private:
    /**
     * @brief 递归构建组件树
     * @param element XML元素
     * @param parent 父组件
     * @return 构建的组件
     */
    QWidget* buildElement(const QDomElement& element, QWidget* parent);
    
    /**
     * @brief 处理布局容器
     * @param element XML元素
     * @param container 容器组件
     */
    void processChildren(const QDomElement& element, QWidget* container);
    
    /**
     * @brief 创建带标签的行布局
     * @param title 标签文本
     * @param widget 组件
     * @return 行容器
     */
    QWidget* createLabeledRow(const QString& title, QWidget* widget);
    
    /**
     * @brief 判断是否是容器标签
     * @param tagName 标签名
     * @return 是否是容器
     */
    bool isContainerTag(const QString& tagName) const;
    
    /**
     * @brief 判断是否是布局标签
     * @param tagName 标签名
     * @return 是否是布局标签
     */
    bool isLayoutTag(const QString& tagName) const;
    
private:
    QuikContext* m_context;
    QWidget* m_rootWidget = nullptr;
    
    // 热更新相关
    QFileSystemWatcher* m_watcher = nullptr;
    QString m_currentFilePath;
    QMap<QString, std::function<void()>> m_buttonCallbacks;
    QMap<QString, std::function<void(const QVariant&)>> m_watchCallbacks;
};

} // namespace Quik

#endif // XMLUIBUILDER_H
