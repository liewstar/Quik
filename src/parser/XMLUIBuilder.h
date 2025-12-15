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
#include <QJsonObject>
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
    
    // ========== 循环渲染数据源 (q-for) ==========
    
    /**
     * @brief 设置循环渲染的数据源
     * @param name 数据源名称（对应 q-for="item in name" 中的 name）
     * @param items 数据列表，每项是一个 QVariantMap，如 {{"text", "选项1"}, {"val", "opt1"}}
     * 
     * 使用示例：
     * @code
     * // XML:
     * // <ComboBox var="cbo">
     * //     <Choice q-for="item in modes" text="$item.text" val="$item.val"/>
     * // </ComboBox>
     * 
     * // C++:
     * QVariantList modes;
     * modes << QVariantMap{{"text", "模式一"}, {"val", "mode1"}};
     * modes << QVariantMap{{"text", "模式二"}, {"val", "mode2"}};
     * builder.setListData("modes", modes);
     * @endcode
     */
    void setListData(const QString& name, const QVariantList& items);
    
    /**
     * @brief 获取数据源
     */
    QVariantList getListData(const QString& name) const;
    
    // ========== 参数持久化 ==========
    
    /**
     * @brief 保存所有参数到 JSON 文件
     * @param filePath 文件路径
     * @param extraData 额外数据（可选），会合并到输出中
     * @return 是否保存成功
     * 
     * 变量名中的点号会自动展开为嵌套结构：
     * - var="mesh.maxSize" → {"mesh": {"maxSize": 1.0}}
     * - 列表数据保持数组格式
     * 
     * 使用示例：
     * @code
     * builder.saveToJson("config.json");
     * 
     * // 带额外数据
     * builder.saveToJson("config.json", {
     *     {"meta.version", "1.0"},
     *     {"meta.timestamp", QDateTime::currentDateTime().toString()}
     * });
     * @endcode
     */
    bool saveToJson(const QString& filePath, const QVariantMap& extraData = {}) const;
    
    /**
     * @brief 从 JSON 文件加载参数
     * @param filePath 文件路径
     * @return 是否加载成功
     * 
     * 自动处理嵌套结构，恢复到对应的 UI 变量和列表数据
     * 
     * 使用示例：
     * @code
     * builder.loadFromJson("config.json");
     * @endcode
     */
    bool loadFromJson(const QString& filePath);
    
    /**
     * @brief 导出为 JSON 对象（不写文件）
     * @param extraData 额外数据
     * @return JSON 对象
     */
    QJsonObject toJsonObject(const QVariantMap& extraData = {}) const;
    
    /**
     * @brief 从 JSON 对象加载（不读文件）
     * @param json JSON 对象
     */
    void fromJsonObject(const QJsonObject& json);
    
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
    
    /**
     * @brief 处理通用 q-for 指令
     * @param element 带 q-for 属性的元素
     * @param container 父容器
     * @param qForExpr q-for 表达式，如 "item in items" 或 "(item, idx) in items"
     */
    void processGeneralQFor(const QDomElement& element, QWidget* container, const QString& qForExpr);
    
    /**
     * @brief 根据模板和数据渲染单个组件
     * @param templateXml 模板 XML 字符串
     * @param index 当前索引
     * @param itemData 当前项数据
     * @param itemVar 循环变量名
     * @param indexVar 索引变量名
     * @return 渲染的组件
     */
    QWidget* renderQForItem(const QString& templateXml, int index, const QVariantMap& itemData,
                            const QString& itemVar, const QString& indexVar);
    
    /**
     * @brief 替换字符串中的模板变量
     * @param str 原始字符串
     * @param index 当前索引
     * @param itemData 当前项数据
     * @param itemVar 循环变量名
     * @param indexVar 索引变量名
     * @return 替换后的字符串
     */
    QString replaceTemplateVars(const QString& str, int index, const QVariantMap& itemData,
                                const QString& itemVar, const QString& indexVar) const;
    
private:
    QuikContext* m_context;
    QWidget* m_rootWidget = nullptr;
    
    // 热更新相关
    QFileSystemWatcher* m_watcher = nullptr;
    QString m_currentFilePath;
    QMap<QString, std::function<void()>> m_buttonCallbacks;
    QMap<QString, std::function<void(const QVariant&)>> m_watchCallbacks;
    
    // 错误覆盖层
    QWidget* m_errorOverlay = nullptr;
    void showErrorOverlay(const QString& errorMsg, int line, int column);
    void hideErrorOverlay();
    
    // 循环渲染数据源
    QMap<QString, QVariantList> m_listData;
};

} // namespace Quik

#endif // XMLUIBUILDER_H
