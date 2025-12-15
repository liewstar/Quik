#include "XMLUIBuilder.h"
#include "Quik/Quik.h"
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include <QRegularExpression>

namespace Quik {

XMLUIBuilder::XMLUIBuilder(QObject* parent)
    : QObject(parent)
    , m_context(new QuikContext(this))
{
}

XMLUIBuilder::~XMLUIBuilder() = default;

QWidget* XMLUIBuilder::buildFromFile(const QString& filePath, QWidget* parent) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString error = QString("Cannot open file: %1").arg(filePath);
        qWarning() << "[Quik]" << error;
        emit buildError(error);
        return nullptr;
    }
    
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    
    QWidget* result = buildFromString(content, parent);
    
    // 自动启用热更新（由 QUIK_HOT_RELOAD_ENABLED 控制）
#if QUIK_HOT_RELOAD_ENABLED
    if (result) {
        enableHotReload(filePath);
    }
#endif
    
    return result;
}

QWidget* XMLUIBuilder::buildFromString(const QString& xmlContent, QWidget* parent) {
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    
    if (!doc.setContent(xmlContent, &errorMsg, &errorLine, &errorColumn)) {
        QString error = QString("XML parse error at line %1, column %2: %3")
                        .arg(errorLine).arg(errorColumn).arg(errorMsg);
        qWarning() << "[Quik]" << error;
        emit buildError(error);
        return nullptr;
    }
    
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        emit buildError("Empty XML document");
        return nullptr;
    }
    
    qDebug() << "[Quik] Building UI from root element:" << root.tagName();
    
    // 创建根容器
    m_rootWidget = new QWidget(parent);
    auto* rootLayout = new QVBoxLayout(m_rootWidget);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);
    
    // 处理所有子元素
    processChildren(root, m_rootWidget);
    
    // 初始化所有绑定（触发初始状态）
    m_context->initializeBindings();
    
    qDebug() << "[Quik] UI build completed";
    emit buildCompleted(m_rootWidget);
    
    return m_rootWidget;
}

QWidget* XMLUIBuilder::getWidget(const QString& varName) const {
    return m_context->getWidget(varName);
}

QVariant XMLUIBuilder::getValue(const QString& varName) const {
    return m_context->getValue(varName);
}

void XMLUIBuilder::setValue(const QString& varName, const QVariant& value) {
    m_context->setValue(varName, value);
}

void XMLUIBuilder::connectButton(const QString& varName, std::function<void()> callback) {
    // 保存回调以便热更新后重新连接
    m_buttonCallbacks[varName] = callback;
    
    QWidget* widget = getWidget(varName);
    if (auto* button = qobject_cast<QPushButton*>(widget)) {
        connect(button, &QPushButton::clicked, this, callback);
    } else {
        qWarning() << "[Quik] Widget is not a button:" << varName;
    }
}

QVariantMap XMLUIBuilder::getAllValues() const {
    return m_context->getContext();
}

void XMLUIBuilder::watch(const QString& varName, std::function<void(const QVariant&)> callback) {
    // 保存回调以便热更新后重新连接
    m_watchCallbacks[varName] = callback;
    m_context->watch(varName, callback);
}

void XMLUIBuilder::unwatch(const QString& varName) {
    m_watchCallbacks.remove(varName);
    m_context->unwatch(varName);
}

// ========== 循环渲染数据源 ==========

void XMLUIBuilder::setListData(const QString& name, const QVariantList& items) {
    m_listData[name] = items;
    if (m_context) {
        m_context->setListData(name, items);
    }
}

QVariantList XMLUIBuilder::getListData(const QString& name) const {
    return m_listData.value(name);
}

// ========== 参数持久化实现 ==========

namespace {

// 辅助函数：将点号分隔的 key 设置到嵌套 JSON 对象中
void setNestedValue(QJsonObject& root, const QString& key, const QJsonValue& value) {
    QStringList parts = key.split('.');
    if (parts.isEmpty()) return;
    
    if (parts.size() == 1) {
        root[key] = value;
        return;
    }
    
    // 递归创建嵌套结构
    QString firstPart = parts.takeFirst();
    QString remainingKey = parts.join('.');
    
    QJsonObject nested = root[firstPart].toObject();
    setNestedValue(nested, remainingKey, value);
    root[firstPart] = nested;
}

// 辅助函数：将 QVariant 转换为 QJsonValue
QJsonValue variantToJson(const QVariant& value) {
    if (value.type() == QVariant::List) {
        QJsonArray arr;
        for (const QVariant& item : value.toList()) {
            arr.append(variantToJson(item));
        }
        return arr;
    } else if (value.type() == QVariant::Map) {
        QJsonObject obj;
        QVariantMap map = value.toMap();
        for (auto it = map.begin(); it != map.end(); ++it) {
            obj[it.key()] = variantToJson(it.value());
        }
        return obj;
    } else {
        return QJsonValue::fromVariant(value);
    }
}

// 辅助函数：递归扁平化 JSON 对象，将嵌套结构转为点号分隔的 key
void flattenJson(const QJsonObject& obj, const QString& prefix, QVariantMap& result, QMap<QString, QVariantList>& lists) {
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QString key = prefix.isEmpty() ? it.key() : prefix + "." + it.key();
        
        if (it.value().isObject()) {
            // 递归处理嵌套对象
            flattenJson(it.value().toObject(), key, result, lists);
        } else if (it.value().isArray()) {
            // 数组作为列表数据
            QVariantList list;
            for (const QJsonValue& item : it.value().toArray()) {
                list.append(item.toVariant());
            }
            lists[it.key()] = list;  // 列表数据用原始 key（不带前缀）
        } else {
            // 普通值
            result[key] = it.value().toVariant();
        }
    }
}

} // anonymous namespace

QJsonObject XMLUIBuilder::toJsonObject(const QVariantMap& extraData) const {
    QJsonObject root;
    
    // 1. 收集所有 UI 变量值
    QVariantMap allValues = getAllValues();
    
    // 2. 合并额外数据
    for (auto it = extraData.begin(); it != extraData.end(); ++it) {
        allValues[it.key()] = it.value();
    }
    
    // 3. 按点号展开成嵌套结构
    for (auto it = allValues.begin(); it != allValues.end(); ++it) {
        setNestedValue(root, it.key(), variantToJson(it.value()));
    }
    
    // 4. 添加列表数据
    for (auto it = m_listData.begin(); it != m_listData.end(); ++it) {
        root[it.key()] = variantToJson(it.value()).toArray();
    }
    
    return root;
}

bool XMLUIBuilder::saveToJson(const QString& filePath, const QVariantMap& extraData) const {
    QJsonObject json = toJsonObject(extraData);
    QJsonDocument doc(json);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[Quik] Cannot open file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "[Quik] Saved parameters to:" << filePath;
    return true;
}

void XMLUIBuilder::fromJsonObject(const QJsonObject& json) {
    QVariantMap flatValues;
    QMap<QString, QVariantList> lists;
    
    // 1. 扁平化 JSON 对象
    flattenJson(json, QString(), flatValues, lists);
    
    // 2. 恢复 UI 变量值
    for (auto it = flatValues.begin(); it != flatValues.end(); ++it) {
        setValue(it.key(), it.value());
    }
    
    // 3. 恢复列表数据
    for (auto it = lists.begin(); it != lists.end(); ++it) {
        setListData(it.key(), it.value());
    }
    
    qDebug() << "[Quik] Loaded" << flatValues.size() << "values and" << lists.size() << "lists";
}

bool XMLUIBuilder::loadFromJson(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[Quik] Cannot open file for reading:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[Quik] JSON parse error:" << error.errorString();
        return false;
    }
    
    if (!doc.isObject()) {
        qWarning() << "[Quik] JSON root is not an object";
        return false;
    }
    
    fromJsonObject(doc.object());
    
    qDebug() << "[Quik] Loaded parameters from:" << filePath;
    return true;
}

// ========== 热更新实现 ==========

void XMLUIBuilder::enableHotReload(const QString& filePath) {
    if (m_watcher) {
        disableHotReload();
    }
    
    m_currentFilePath = filePath;
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(filePath);
    
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &XMLUIBuilder::onFileChanged);
    
    qDebug() << "[Quik] Hot reload enabled for:" << filePath;
}

void XMLUIBuilder::disableHotReload() {
    if (m_watcher) {
        disconnect(m_watcher, nullptr, this, nullptr);
        delete m_watcher;
        m_watcher = nullptr;
        m_currentFilePath.clear();
        qDebug() << "[Quik] Hot reload disabled";
    }
}

bool XMLUIBuilder::isHotReloadEnabled() const {
    return m_watcher != nullptr;
}

void XMLUIBuilder::onFileChanged(const QString& path) {
    Q_UNUSED(path)
    
    // 延迟100ms重载，避免文件写入未完成
    QTimer::singleShot(100, this, &XMLUIBuilder::reload);
    
    // 文件变化后需要重新添加监听（某些系统会移除）
    if (m_watcher && !m_currentFilePath.isEmpty()) {
        m_watcher->addPath(m_currentFilePath);
    }
}

void XMLUIBuilder::reload() {
    if (m_currentFilePath.isEmpty()) {
        qWarning() << "[Quik] No file path set for reload";
        return;
    }
    
    // 1. 先验证XML是否有效（不销毁旧UI）
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[Quik] Hot reload: Cannot open file";
        return;
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(content, &errorMsg, &errorLine, &errorColumn)) {
        qWarning() << "[Quik] Hot reload: XML parse error at line" << errorLine 
                   << "- waiting for valid XML...";
        showErrorOverlay(errorMsg, errorLine, errorColumn);
        return;  // XML无效，保持旧UI不变
    }
    
    // XML有效，隐藏错误覆盖层
    hideErrorOverlay();
    
    qDebug() << "[Quik] Hot reloading:" << m_currentFilePath;
    
    // 2. XML有效，保存当前状态
    QVariantMap state = getAllValues();
    QWidget* oldRoot = m_rootWidget;
    QWidget* parent = oldRoot ? oldRoot->parentWidget() : nullptr;
    QLayout* parentLayout = parent ? parent->layout() : nullptr;
    int layoutIndex = -1;
    
    // 找到旧widget在layout中的位置
    if (parentLayout && oldRoot) {
        for (int i = 0; i < parentLayout->count(); ++i) {
            if (parentLayout->itemAt(i)->widget() == oldRoot) {
                layoutIndex = i;
                break;
            }
        }
    }
    
    // 3. 重建Context
    delete m_context;
    m_context = new QuikContext(this);
    m_rootWidget = nullptr;
    
    // 恢复数据源到新Context
    for (auto it = m_listData.begin(); it != m_listData.end(); ++it) {
        m_context->setListData(it.key(), it.value());
    }
    
    // 4. 重建UI（使用已验证的内容）
    QWidget* newRoot = buildFromString(content, parent);
    if (!newRoot) {
        qWarning() << "[Quik] Hot reload failed to build UI";
        // 恢复旧Context
        m_context = new QuikContext(this);
        m_rootWidget = oldRoot;
        return;
    }
    
    // 5. 替换旧UI
    if (parentLayout && layoutIndex >= 0) {
        parentLayout->removeWidget(oldRoot);
        if (auto* boxLayout = qobject_cast<QBoxLayout*>(parentLayout)) {
            boxLayout->insertWidget(layoutIndex, newRoot);
        } else {
            parentLayout->addWidget(newRoot);
        }
    }
    
    // 销毁旧UI
    if (oldRoot) {
        oldRoot->hide();
        oldRoot->deleteLater();
    }
    
    // 6. 恢复状态值
    for (auto it = state.begin(); it != state.end(); ++it) {
        setValue(it.key(), it.value());
    }
    
    // 7. 重新连接按钮回调
    for (auto it = m_buttonCallbacks.begin(); it != m_buttonCallbacks.end(); ++it) {
        QWidget* widget = getWidget(it.key());
        if (auto* button = qobject_cast<QPushButton*>(widget)) {
            connect(button, &QPushButton::clicked, this, it.value());
        }
    }
    
    // 8. 重新连接watch回调
    for (auto it = m_watchCallbacks.begin(); it != m_watchCallbacks.end(); ++it) {
        m_context->watch(it.key(), it.value());
    }
    
    qDebug() << "[Quik] Hot reload completed";
    emit reloaded();
}

bool XMLUIBuilder::isValid() const {
    return getValidationErrors().isEmpty();
}

QMap<QString, QString> XMLUIBuilder::getValidationErrors() const {
    QMap<QString, QString> errors;
    
    if (!m_rootWidget) return errors;
    
    // 遍历所有LineEdit检查验证状态
    QList<QLineEdit*> lineEdits = m_rootWidget->findChildren<QLineEdit*>();
    for (QLineEdit* lineEdit : lineEdits) {
        if (lineEdit->property("_Quik_hasError").toBool()) {
            // 获取变量名
            QString varName = lineEdit->property("_Quik_varName").toString();
            if (varName.isEmpty()) {
                varName = lineEdit->objectName();
            }
            errors[varName] = lineEdit->toolTip();
        }
    }
    
    return errors;
}

QWidget* XMLUIBuilder::buildElement(const QDomElement& element, QWidget* parent) {
    QString tagName = element.tagName();
    
    // 跳过Choice元素（由ComboBox内部处理）
    if (tagName == "Choice") {
        return nullptr;
    }
    
    // 使用工厂创建组件
    QWidget* widget = WidgetFactory::instance().create(tagName, element, m_context);
    
    if (!widget) {
        // 创建错误占位符，显示未知标签
        QString error = QString("Unknown tag: <%1>").arg(tagName);
        qWarning() << "[Quik]" << error;
        emit buildError(error);
        
        // 返回一个红色错误标签作为占位符
        auto* errorLabel = new QLabel(QString("[Error: %1]").arg(error));
        errorLabel->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 5px; border: 1px dashed red; }");
        return errorLabel;
    }
    
    // 如果是容器，递归处理子元素
    if (isContainerTag(tagName)) {
        processChildren(element, widget);
    }
    
    return widget;
}

void XMLUIBuilder::processChildren(const QDomElement& element, QWidget* container) {
    QLayout* layout = container->layout();
    if (!layout) {
        layout = new QVBoxLayout(container);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(5);
    }
    
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        QString tagName = child.tagName();
        
        // 跳过Choice和Item（由父组件处理）
        if (tagName == "Choice" || tagName == "Item") {
            child = child.nextSiblingElement();
            continue;
        }
        
        // 处理addStretch
        if (tagName == "addStretch") {
            int stretch = child.attribute("stretch", "1").toInt();
            if (auto* boxLayout = qobject_cast<QBoxLayout*>(layout)) {
                boxLayout->addStretch(stretch);
            }
            child = child.nextSiblingElement();
            continue;
        }
        
        // ========== 处理通用 q-for ==========
        QString qFor = child.attribute("q-for");
        if (!qFor.isEmpty()) {
            processGeneralQFor(child, container, qFor);
            child = child.nextSiblingElement();
            continue;
        }
        
        QWidget* childWidget = buildElement(child, container);
        
        if (childWidget) {
            // 检查是否是stretch占位符
            if (childWidget->objectName() == "__stretch__") {
                int stretch = childWidget->property("stretchFactor").toInt();
                if (auto* boxLayout = qobject_cast<QBoxLayout*>(layout)) {
                    boxLayout->addStretch(stretch);
                }
                delete childWidget;
                child = child.nextSiblingElement();
                continue;
            }
            
            // 检查是否需要添加标签
            QString title = child.attribute("title");
            
            // 某些组件需要带标签的行布局
            bool needsLabel = !title.isEmpty() && 
                             (tagName == "LineEdit" || tagName == "ComboBox" || 
                              tagName == "SpinBox" || tagName == "DoubleSpinBox");
            
            // 获取暂存的 visible/enabled 属性
            QString visible = childWidget->property("_Quik_visible").toString();
            QString enabled = childWidget->property("_Quik_enabled").toString();
            
            // 确定绑定目标：带标签的行容器 或 组件本身
            QWidget* bindTarget = childWidget;
            
            if (needsLabel) {
                QWidget* row = createLabeledRow(title, childWidget);
                bindTarget = row;  // 绑定到整个行容器
                layout->addWidget(row);
            } else {
                layout->addWidget(childWidget);
            }
            
            // 应用 visible 绑定到目标
            if (!visible.isEmpty()) {
                if (ExpressionParser::isExpression(visible)) {
                    m_context->bindVisible(bindTarget, visible);
                } else {
                    bindTarget->setVisible(visible == "true" || visible == "1");
                }
            }
            
            // 应用 enabled 绑定到目标
            if (!enabled.isEmpty()) {
                if (ExpressionParser::isExpression(enabled)) {
                    m_context->bindEnabled(bindTarget, enabled);
                } else {
                    bindTarget->setEnabled(enabled == "true" || enabled == "1");
                }
            }
        }
        
        child = child.nextSiblingElement();
    }
}

QWidget* XMLUIBuilder::createLabeledRow(const QString& title, QWidget* widget) {
    auto* row = new QWidget();
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    
    auto* label = new QLabel(title);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setMinimumWidth(120);
    
    layout->addWidget(label);
    layout->addWidget(widget, 1);  // 组件占据剩余空间
    
    return row;
}

bool XMLUIBuilder::isContainerTag(const QString& tagName) const {
    static const QStringList containers = {
        "GroupBox", "InnerGroupBox", "Frame", "Widget", "ScrollArea",
        "HLayoutWidget", "VLayoutWidget"
    };
    return containers.contains(tagName);
}

bool XMLUIBuilder::isLayoutTag(const QString& tagName) const {
    static const QStringList layouts = {
        "HBoxLayout", "VBoxLayout", "FormLayout", "GridLayout"
    };
    return layouts.contains(tagName);
}

// ========== 通用 q-for 实现 ==========

void XMLUIBuilder::processGeneralQFor(const QDomElement& element, QWidget* container, const QString& qForExpr) {
    // 解析 q-for 表达式
    QString itemVar, indexVar, listName;
    
    // 尝试匹配带索引的格式: (item, index) in listName
    QRegularExpression reWithIndex("\\(\\s*(\\w+)\\s*,\\s*(\\w+)\\s*\\)\\s+in\\s+(\\w+)");
    QRegularExpressionMatch matchWithIndex = reWithIndex.match(qForExpr);
    
    if (matchWithIndex.hasMatch()) {
        itemVar = matchWithIndex.captured(1);
        indexVar = matchWithIndex.captured(2);
        listName = matchWithIndex.captured(3);
    } else {
        // 尝试匹配简单格式: item in listName
        QRegularExpression reSimple("(\\w+)\\s+in\\s+(\\w+)");
        QRegularExpressionMatch matchSimple = reSimple.match(qForExpr);
        if (matchSimple.hasMatch()) {
            itemVar = matchSimple.captured(1);
            listName = matchSimple.captured(2);
        }
    }
    
    if (itemVar.isEmpty() || listName.isEmpty()) {
        qWarning() << "[Quik] Invalid q-for expression:" << qForExpr;
        return;
    }
    
    // 将元素转换为 XML 字符串作为模板
    QString templateXml;
    QTextStream stream(&templateXml);
    element.save(stream, 0);
    
    // 移除 q-for 属性，避免递归
    templateXml.replace(QRegularExpression("\\s*q-for\\s*=\\s*\"[^\"]*\""), "");
    
    qDebug() << "[Quik] Processing general q-for:" << qForExpr;
    qDebug() << "[Quik] Template:" << templateXml.left(100) << "...";
    
    // 创建一个占位容器用于放置动态生成的组件
    auto* placeholder = new QWidget(container);
    placeholder->setObjectName(QString("_qfor_%1").arg(listName));
    auto* placeholderLayout = new QVBoxLayout(placeholder);
    placeholderLayout->setContentsMargins(0, 0, 0, 0);
    placeholderLayout->setSpacing(5);
    
    // 添加到父容器
    if (QLayout* parentLayout = container->layout()) {
        parentLayout->addWidget(placeholder);
    }
    
    // 捕获 this 和必要变量用于回调
    QString capturedItemVar = itemVar;
    QString capturedIndexVar = indexVar;
    
    // 注册通用 q-for 绑定
    m_context->registerGeneralQFor(
        listName, itemVar, indexVar, placeholder, templateXml,
        [this, capturedItemVar, capturedIndexVar](const QString& tpl, int idx, const QVariantMap& data) -> QWidget* {
            return renderQForItem(tpl, idx, data, capturedItemVar, capturedIndexVar);
        }
    );
}

QWidget* XMLUIBuilder::renderQForItem(const QString& templateXml, int index, const QVariantMap& itemData,
                                       const QString& itemVar, const QString& indexVar) {
    // 替换模板中的变量
    QString processedXml = replaceTemplateVars(templateXml, index, itemData, itemVar, indexVar);
    
    qDebug() << "[Quik] Rendering q-for item" << index << ":" << processedXml.left(200);
    
    // 解析替换后的 XML
    QDomDocument doc;
    QString errorMsg;
    if (!doc.setContent(processedXml, &errorMsg)) {
        qWarning() << "[Quik] Failed to parse q-for template:" << errorMsg;
        qWarning() << "[Quik] Processed XML:" << processedXml;
        return nullptr;
    }
    
    QDomElement element = doc.documentElement();
    if (element.isNull()) {
        return nullptr;
    }
    
    // 使用现有的 buildElement 创建组件
    QWidget* widget = buildElement(element, nullptr);
    
    if (widget) {
        // 处理 visible 属性绑定
        QString visible = element.attribute("visible");
        if (!visible.isEmpty()) {
            if (ExpressionParser::isExpression(visible)) {
                m_context->bindVisible(widget, visible);
            } else {
                widget->setVisible(visible == "true" || visible == "1");
            }
        }
        
        // 处理 enabled 属性绑定
        QString enabled = element.attribute("enabled");
        if (!enabled.isEmpty()) {
            if (ExpressionParser::isExpression(enabled)) {
                m_context->bindEnabled(widget, enabled);
            } else {
                widget->setEnabled(enabled == "true" || enabled == "1");
            }
        }
    }
    
    return widget;
}

QString XMLUIBuilder::replaceTemplateVars(const QString& str, int index, const QVariantMap& itemData,
                                          const QString& itemVar, const QString& indexVar) const {
    QString result = str;
    
    // 替换索引变量 $idx 或 $index
    if (!indexVar.isEmpty()) {
        result.replace(QString("$%1").arg(indexVar), QString::number(index));
    }
    
    // 替换 $item.xxx 格式的变量
    for (auto it = itemData.begin(); it != itemData.end(); ++it) {
        QString placeholder = QString("$%1.%2").arg(itemVar).arg(it.key());
        result.replace(placeholder, it.value().toString());
    }
    
    // 替换 var 属性中的动态部分，如 var="data.$idx.name" → var="data.0.name"
    if (!indexVar.isEmpty()) {
        QRegularExpression varPattern(QString("var\\s*=\\s*\"([^\"]*\\$%1[^\"]*)\"").arg(indexVar));
        QRegularExpressionMatchIterator matches = varPattern.globalMatch(result);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString original = match.captured(0);
            QString varValue = match.captured(1);
            QString newVarValue = varValue.replace(QString("$%1").arg(indexVar), QString::number(index));
            result.replace(original, QString("var=\"%1\"").arg(newVarValue));
        }
    }
    
    return result;
}

void XMLUIBuilder::showErrorOverlay(const QString& errorMsg, int line, int column) {
    if (!m_rootWidget) return;
    
    QWidget* parent = m_rootWidget->parentWidget();
    if (!parent) parent = m_rootWidget;
    
    // 创建或更新错误覆盖层
    if (!m_errorOverlay) {
        m_errorOverlay = new QWidget(parent);
        m_errorOverlay->setObjectName("QuikErrorOverlay");
        m_errorOverlay->setStyleSheet(
            "QWidget#QuikErrorOverlay {"
            "  background-color: rgba(220, 38, 38, 0.95);"
            "  border-radius: 8px;"
            "  padding: 16px;"
            "}"
            "QLabel#errorTitle {"
            "  color: white;"
            "  font-size: 16px;"
            "  font-weight: bold;"
            "}"
            "QLabel#errorMsg {"
            "  color: rgba(255, 255, 255, 0.9);"
            "  font-size: 13px;"
            "  font-family: Consolas, Monaco, monospace;"
            "}"
            "QLabel#errorHint {"
            "  color: rgba(255, 255, 255, 0.7);"
            "  font-size: 12px;"
            "}"
        );
        
        auto* layout = new QVBoxLayout(m_errorOverlay);
        layout->setSpacing(8);
        
        auto* titleLabel = new QLabel("XML Parse Error");
        titleLabel->setObjectName("errorTitle");
        layout->addWidget(titleLabel);
        
        auto* msgLabel = new QLabel();
        msgLabel->setObjectName("errorMsg");
        msgLabel->setWordWrap(true);
        layout->addWidget(msgLabel);
        
        auto* hintLabel = new QLabel("Fix the error and save to reload");
        hintLabel->setObjectName("errorHint");
        layout->addWidget(hintLabel);
    }
    
    // 更新错误信息
    auto* msgLabel = m_errorOverlay->findChild<QLabel*>("errorMsg");
    if (msgLabel) {
        msgLabel->setText(QString("Line %1, Column %2:\n%3")
                         .arg(line).arg(column).arg(errorMsg));
    }
    
    // 定位到父容器底部
    m_errorOverlay->setParent(parent);
    m_errorOverlay->adjustSize();
    int x = (parent->width() - m_errorOverlay->width()) / 2;
    int y = parent->height() - m_errorOverlay->height() - 16;
    m_errorOverlay->move(x, y);
    m_errorOverlay->raise();
    m_errorOverlay->show();
}

void XMLUIBuilder::hideErrorOverlay() {
    if (m_errorOverlay) {
        m_errorOverlay->hide();
    }
}

} // namespace Quik
