#include "WidgetFactory.h"
#include "core/QuikContext.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QRegularExpression>
#include <QListWidget>
#include <QTabBar>
#include <QSlider>
#include <QProgressBar>
#include <QDebug>

namespace Quik {

// 验证错误提示文本（集中管理，便于国际化）
// 注意：如需中文，请确保源文件保存为UTF-8 with BOM编码
namespace ValidationMessages {
    static QString required()         { return QStringLiteral("Required"); }
    static QString minValue(double v) { return QString("Min: %1").arg(v); }
    static QString maxValue(double v) { return QString("Max: %1").arg(v); }
    static QString invalidFormat()    { return QStringLiteral("Invalid"); }
    static QString invalidNumber()    { return QStringLiteral("Invalid number"); }
}

WidgetFactory& WidgetFactory::instance() {
    static WidgetFactory instance;
    return instance;
}

WidgetFactory::WidgetFactory() {
    registerBuiltinWidgets();
}

void WidgetFactory::registerCreator(const QString& tagName, WidgetCreator creator) {
    m_creators[tagName] = creator;
}

QWidget* WidgetFactory::create(const QString& tagName, const QDomElement& element, QuikContext* context) {
    if (!m_creators.contains(tagName)) {
        qWarning() << "[Quik] Unknown widget tag:" << tagName;
        return nullptr;
    }
    
    return m_creators[tagName](element, context);
}

bool WidgetFactory::hasCreator(const QString& tagName) const {
    return m_creators.contains(tagName);
}

void WidgetFactory::registerBuiltinWidgets() {
    if (m_initialized) return;
    
    registerCreator("Label", createLabel);
    registerCreator("LineEdit", createLineEdit);
    registerCreator("CheckBox", createCheckBox);
    registerCreator("ComboBox", createComboBox);
    registerCreator("SpinBox", createSpinBox);
    registerCreator("DoubleSpinBox", createDoubleSpinBox);
    registerCreator("PushButton", createPushButton);
    registerCreator("ApplyButton", createPushButton);  // ApplyButton同PushButton
    registerCreator("GroupBox", createGroupBox);
    registerCreator("InnerGroupBox", createInnerGroupBox);
    registerCreator("RadioButton", createRadioButton);
    registerCreator("HLine", createHLine);
    registerCreator("VLine", createVLine);
    registerCreator("Separator", createHLine);  // Separator同HLine
    registerCreator("LabelList", createLabelList);
    registerCreator("ListBox", createListBox);
    registerCreator("NewTabBar", createTabBar);
    registerCreator("TabBar", createTabBar);
    registerCreator("HLayoutWidget", createHLayoutWidget);
    registerCreator("VLayoutWidget", createVLayoutWidget);
    registerCreator("addStretch", createStretch);
    registerCreator("PointLineEdit", createPointLineEdit);
    registerCreator("TwoPointLineEdit", createTwoPointLineEdit);
    registerCreator("Slider", createSlider);
    registerCreator("ProgressBar", createProgressBar);
    
    m_initialized = true;
    qDebug() << "[Quik] Registered" << m_creators.size() << "builtin widgets";
}

// ========== 内置组件创建器 ==========

QWidget* WidgetFactory::createLabel(const QDomElement& element, QuikContext* context) {
    auto* label = new QLabel();
    
    QString title = getAttribute(element, "title");
    QString text = getAttribute(element, "text");
    label->setText(title.isEmpty() ? text : title);
    
    // 对齐方式
    QString align = getAttribute(element, "align", "left");
    if (align == "left") {
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    } else if (align == "right") {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    } else if (align == "center") {
        label->setAlignment(Qt::AlignCenter);
    }
    
    applyCommonAttributes(label, element, context);
    return label;
}

QWidget* WidgetFactory::createLineEdit(const QDomElement& element, QuikContext* context) {
    auto* lineEdit = new QLineEdit();
    
    // 默认值
    QString defaultVal = getAttribute(element, "default");
    if (!defaultVal.isEmpty()) {
        lineEdit->setText(defaultVal);
    }
    
    // 占位符
    QString placeholder = getAttribute(element, "placeholder");
    if (!placeholder.isEmpty()) {
        lineEdit->setPlaceholderText(placeholder);
    }
    
    // 只读
    bool readOnly = getBoolAttribute(element, "readOnly", false);
    lineEdit->setReadOnly(readOnly);
    
    // 验证器和范围
    QString valid = getAttribute(element, "valid");
    double minVal = getDoubleAttribute(element, "min", -1e308);
    double maxVal = getDoubleAttribute(element, "max", 1e308);
    bool required = getBoolAttribute(element, "required", false);
    QString pattern = getAttribute(element, "pattern");
    QString errorMsg = getAttribute(element, "errorMsg");
    
    // 不使用QValidator，允许用户自由输入，通过红框提示验证错误
    
    // 存储验证参数和变量名到属性中
    QString varName = getAttribute(element, "var");
    lineEdit->setProperty("_Quik_varName", varName);
    lineEdit->setProperty("_Quik_valid", valid);
    lineEdit->setProperty("_Quik_min", minVal);
    lineEdit->setProperty("_Quik_max", maxVal);
    lineEdit->setProperty("_Quik_required", required);
    lineEdit->setProperty("_Quik_pattern", pattern);
    lineEdit->setProperty("_Quik_errorMsg", errorMsg);
    
    // 正常样式
    QString normalStyle = "QLineEdit { border: 1px solid #ccc; padding: 2px; }";
    QString errorStyle = "QLineEdit { border: 2px solid red; padding: 1px; background-color: #fff0f0; }";
    lineEdit->setProperty("_Quik_normalStyle", normalStyle);
    lineEdit->setProperty("_Quik_errorStyle", errorStyle);
    
    // 连接文本变化信号进行验证
    QObject::connect(lineEdit, &QLineEdit::textChanged, [lineEdit, valid, minVal, maxVal, required, pattern, errorMsg, normalStyle, errorStyle]() {
        QString text = lineEdit->text();
        QString error;
        
        // 必填验证
        if (required && text.isEmpty()) {
            error = errorMsg.isEmpty() ? ValidationMessages::required() : errorMsg;
        }
        // 数值范围验证
        else if (!text.isEmpty() && (valid == "double" || valid == "int")) {
            bool ok;
            double val = text.toDouble(&ok);
            if (!ok) {
                // 转换失败（数值过大或格式错误）- 始终使用内置提示
                error = ValidationMessages::invalidNumber();
            } else if (val < minVal) {
                error = errorMsg.isEmpty() ? ValidationMessages::minValue(minVal) : errorMsg;
            } else if (val > maxVal) {
                error = errorMsg.isEmpty() ? ValidationMessages::maxValue(maxVal) : errorMsg;
            }
        }
        // 正则验证
        else if (!pattern.isEmpty() && !text.isEmpty()) {
            QRegularExpression re(pattern);
            if (!re.match(text).hasMatch()) {
                error = errorMsg.isEmpty() ? ValidationMessages::invalidFormat() : errorMsg;
            }
        }
        
        // 应用样式和提示
        if (!error.isEmpty()) {
            lineEdit->setStyleSheet(errorStyle);
            lineEdit->setToolTip(error);
            lineEdit->setProperty("_Quik_hasError", true);
        } else {
            lineEdit->setStyleSheet(normalStyle);
            lineEdit->setToolTip("");
            lineEdit->setProperty("_Quik_hasError", false);
        }
    });
    
    // 对齐方式
    QString align = getAttribute(element, "align");
    if (align == "left") {
        lineEdit->setAlignment(Qt::AlignLeft);
    } else if (align == "right") {
        lineEdit->setAlignment(Qt::AlignRight);
    } else if (align == "center") {
        lineEdit->setAlignment(Qt::AlignCenter);
    }
    
    applyCommonAttributes(lineEdit, element, context);
    return lineEdit;
}

QWidget* WidgetFactory::createCheckBox(const QDomElement& element, QuikContext* context) {
    auto* checkBox = new QCheckBox();
    
    QString title = getAttribute(element, "title");
    QString text = getAttribute(element, "text");
    checkBox->setText(title.isEmpty() ? text : title);
    
    // 默认值
    bool defaultVal = getBoolAttribute(element, "default", false);
    checkBox->setChecked(defaultVal);
    
    applyCommonAttributes(checkBox, element, context);
    return checkBox;
}

QWidget* WidgetFactory::createComboBox(const QDomElement& element, QuikContext* context) {
    auto* comboBox = new QComboBox();
    
    QString defaultVal = getAttribute(element, "default");
    int defaultIndex = 0;
    
    // 解析Choice子元素
    QDomElement choice = element.firstChildElement("Choice");
    int index = 0;
    while (!choice.isNull()) {
        // 检查是否有 q-for 属性
        QString qFor = getAttribute(choice, "q-for");
        if (!qFor.isEmpty() && context) {
            // 解析 q-for 格式：
            // 1. "item in listName"
            // 2. "(item, index) in listName"
            QString itemVar, indexVar, listName;
            
            // 尝试匹配带索引的格式: (item, index) in listName
            QRegularExpression reWithIndex("\\(\\s*(\\w+)\\s*,\\s*(\\w+)\\s*\\)\\s+in\\s+(\\w+)");
            QRegularExpressionMatch matchWithIndex = reWithIndex.match(qFor);
            
            if (matchWithIndex.hasMatch()) {
                itemVar = matchWithIndex.captured(1);   // "item"
                indexVar = matchWithIndex.captured(2);  // "index"
                listName = matchWithIndex.captured(3);  // "listName"
            } else {
                // 尝试匹配简单格式: item in listName
                QRegularExpression reSimple("(\\w+)\\s+in\\s+(\\w+)");
                QRegularExpressionMatch matchSimple = reSimple.match(qFor);
                if (matchSimple.hasMatch()) {
                    itemVar = matchSimple.captured(1);
                    listName = matchSimple.captured(2);
                }
            }
            
            if (!itemVar.isEmpty() && !listName.isEmpty()) {
                // 获取模板属性
                QString textTemplate = getAttribute(choice, "text");
                QString valTemplate = getAttribute(choice, "val");
                
                // 注册 q-for 绑定（支持响应式更新）
                context->registerQForBinding(comboBox, listName, itemVar, indexVar, textTemplate, valTemplate);
            }
        } else {
            // 普通 Choice 元素
            QString text = getAttribute(choice, "text");
            QString val = getAttribute(choice, "val");
            
            // 将val存储为itemData
            if (!val.isEmpty()) {
                comboBox->addItem(text, val);
                // 检查是否是默认选项
                if (val == defaultVal) {
                    defaultIndex = index;
                }
            } else {
                comboBox->addItem(text);
            }
            ++index;
        }
        
        choice = choice.nextSiblingElement("Choice");
    }
    
    // 设置默认选项
    if (comboBox->count() > 0) {
        // 如果default是数字索引
        bool ok;
        int numDefault = defaultVal.toInt(&ok);
        if (ok && numDefault >= 0 && numDefault < comboBox->count()) {
            comboBox->setCurrentIndex(numDefault);
        } else {
            comboBox->setCurrentIndex(defaultIndex);
        }
    }
    
    applyCommonAttributes(comboBox, element, context);
    return comboBox;
}

QWidget* WidgetFactory::createSpinBox(const QDomElement& element, QuikContext* context) {
    auto* spinBox = new QSpinBox();
    
    int min = getIntAttribute(element, "min", 0);
    
    // 支持 max="+" 表示最大值
    QString maxStr = getAttribute(element, "max", "100");
    int max = (maxStr == "+" || maxStr == "max") ? INT_MAX : maxStr.toInt();
    
    int defaultVal = getIntAttribute(element, "default", min);
    
    spinBox->setRange(min, max);
    spinBox->setValue(defaultVal);
    
    applyCommonAttributes(spinBox, element, context);
    return spinBox;
}

QWidget* WidgetFactory::createDoubleSpinBox(const QDomElement& element, QuikContext* context) {
    auto* spinBox = new QDoubleSpinBox();
    
    double min = getDoubleAttribute(element, "min", 0.0);
    double max = getDoubleAttribute(element, "max", 100.0);
    double defaultVal = getDoubleAttribute(element, "default", min);
    int decimals = getIntAttribute(element, "decimals", 2);
    
    spinBox->setRange(min, max);
    spinBox->setDecimals(decimals);
    spinBox->setValue(defaultVal);
    
    applyCommonAttributes(spinBox, element, context);
    return spinBox;
}

QWidget* WidgetFactory::createPushButton(const QDomElement& element, QuikContext* context) {
    auto* button = new QPushButton();
    
    QString text = getAttribute(element, "text");
    QString title = getAttribute(element, "title");
    button->setText(text.isEmpty() ? title : text);
    
    applyCommonAttributes(button, element, context);
    return button;
}

QWidget* WidgetFactory::createGroupBox(const QDomElement& element, QuikContext* context) {
    auto* groupBox = new QGroupBox();
    
    QString title = getAttribute(element, "title");
    QString titleMode = getAttribute(element, "titleMode");
    
    // titleMode处理
    // onlyTitle: 只显示标题，无边框，标题左对齐
    // titlePlusExpand: 标题+可折叠
    // secondaryTitle: 次级标题样式（灰色、较小）
    if (titleMode == "onlyTitle") {
        groupBox->setTitle(title);
        groupBox->setFlat(true);
        groupBox->setStyleSheet(R"(
            QGroupBox {
                border: none;
                font-weight: bold;
                margin-top: 0px;
                padding-top: 16px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 0px;
                padding: 0px 0px;
            }
        )");
    } else if (titleMode == "secondaryTitle") {
        groupBox->setTitle(title);
        groupBox->setFlat(true);
        groupBox->setStyleSheet(R"(
            QGroupBox {
                border: none;
                color: #666;
                font-size: 11px;
                margin-top: 0px;
                padding-top: 14px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 0px;
                padding: 0px 0px;
            }
        )");
    } else if (!title.isEmpty()) {
        // 有标题的普通GroupBox
        groupBox->setTitle(title);
        groupBox->setStyleSheet(R"(
            QGroupBox {
                border: 1px solid #ccc;
                border-radius: 3px;
                margin-top: 8px;
                padding-top: 8px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 8px;
                padding: 0px 4px;
            }
        )");
    }
    // 无标题的GroupBox不设置样式
    
    // 是否显示边框
    bool showbox = getBoolAttribute(element, "showbox", true);
    if (!showbox) {
        groupBox->setFlat(true);
        groupBox->setStyleSheet("QGroupBox { border: none; margin-top: 0px; padding-top: 0px; }");
    }
    
    // 创建布局
    auto* layout = new QVBoxLayout(groupBox);
    
    // 边距处理
    QString margins = getAttribute(element, "margins");
    if (!margins.isEmpty()) {
        QStringList parts = margins.split(",");
        if (parts.size() == 4) {
            layout->setContentsMargins(parts[0].toInt(), parts[1].toInt(), 
                                       parts[2].toInt(), parts[3].toInt());
        }
    } else {
        bool nomargin = getBoolAttribute(element, "nomargin", false);
        if (nomargin) {
            layout->setContentsMargins(0, 0, 0, 0);
        } else {
            // 有标题时顶部边距需要更大
            if (!title.isEmpty() && titleMode.isEmpty()) {
                layout->setContentsMargins(8, 16, 8, 8);
            } else {
                layout->setContentsMargins(5, 5, 5, 5);
            }
        }
    }
    layout->setSpacing(5);
    
    applyCommonAttributes(groupBox, element, context);
    return groupBox;
}

QWidget* WidgetFactory::createRadioButton(const QDomElement& element, QuikContext* context) {
    auto* radioButton = new QRadioButton();
    
    QString title = getAttribute(element, "title");
    QString text = getAttribute(element, "text");
    radioButton->setText(title.isEmpty() ? text : title);
    
    bool defaultVal = getBoolAttribute(element, "default", false);
    radioButton->setChecked(defaultVal);
    
    applyCommonAttributes(radioButton, element, context);
    return radioButton;
}

QWidget* WidgetFactory::createHLine(const QDomElement& element, QuikContext* context) {
    auto* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    
    applyCommonAttributes(line, element, context);
    return line;
}

QWidget* WidgetFactory::createVLine(const QDomElement& element, QuikContext* context) {
    auto* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    
    applyCommonAttributes(line, element, context);
    return line;
}

QWidget* WidgetFactory::createInnerGroupBox(const QDomElement& element, QuikContext* context) {
    // InnerGroupBox是无边框的容器，用于布局
    auto* widget = new QWidget();
    
    // 布局方向
    QString layout = getAttribute(element, "layout", "ver");
    QBoxLayout* boxLayout = nullptr;
    if (layout == "hor" || layout == "horizontal") {
        boxLayout = new QHBoxLayout(widget);
    } else {
        boxLayout = new QVBoxLayout(widget);
    }
    
    // 边距控制
    bool hasLeftMargin = getBoolAttribute(element, "hasLeftMargin", true);
    bool hasTopMargin = getBoolAttribute(element, "hasTopMargin", true);
    int left = hasLeftMargin ? 5 : 0;
    int top = hasTopMargin ? 5 : 0;
    boxLayout->setContentsMargins(left, top, 5, 5);
    boxLayout->setSpacing(5);
    
    applyCommonAttributes(widget, element, context);
    return widget;
}

QWidget* WidgetFactory::createLabelList(const QDomElement& element, QuikContext* context) {
    // LabelList是一个水平排列的标签列表，用于显示选中信息
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    
    // 对齐方式
    QString align = getAttribute(element, "align", "left");
    
    // 解析Item子元素
    QDomElement item = element.firstChildElement("Item");
    while (!item.isNull()) {
        QString title = getAttribute(item, "title");
        QString var = getAttribute(item, "var");
        bool highlight = getBoolAttribute(item, "highlight", false);
        
        auto* label = new QLabel(title);
        if (highlight) {
            label->setStyleSheet("font-weight: bold; color: #0066cc;");
        }
        
        // 注册变量
        if (!var.isEmpty() && context) {
            label->setObjectName(var);
            context->registerVariable(var, label);
        }
        
        layout->addWidget(label);
        item = item.nextSiblingElement("Item");
    }
    
    // 添加弹性空间
    if (align == "left") {
        layout->addStretch();
    } else if (align == "right") {
        layout->insertStretch(0);
    }
    
    applyCommonAttributes(widget, element, context);
    return widget;
}

QWidget* WidgetFactory::createListBox(const QDomElement& element, QuikContext* context) {
    auto* listWidget = new QListWidget();
    
    // 选择模式
    QString selection = getAttribute(element, "selection", "single");
    if (selection == "multi" || selection == "multiple") {
        listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    } else if (selection == "extended") {
        listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    } else {
        listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    
    // 排序
    bool sorting = getBoolAttribute(element, "sorting", true);
    listWidget->setSortingEnabled(sorting);
    
    applyCommonAttributes(listWidget, element, context);
    return listWidget;
}

QWidget* WidgetFactory::createTabBar(const QDomElement& element, QuikContext* context) {
    auto* tabBar = new QTabBar();
    
    QString var = getAttribute(element, "var");
    QString defaultVal = getAttribute(element, "default");
    int defaultIndex = 0;
    
    // 解析Choice子元素
    QDomElement choice = element.firstChildElement("Choice");
    int index = 0;
    while (!choice.isNull()) {
        QString text = getAttribute(choice, "text");
        QString val = getAttribute(choice, "val");
        
        tabBar->addTab(text);
        tabBar->setTabData(index, val);
        
        // 检查是否是默认选项
        if (!defaultVal.isEmpty() && val == defaultVal) {
            defaultIndex = index;
        }
        
        ++index;
        choice = choice.nextSiblingElement("Choice");
    }
    
    // 设置默认选项
    if (tabBar->count() > 0) {
        tabBar->setCurrentIndex(defaultIndex);
    }
    
    // 注册变量并连接信号
    if (!var.isEmpty() && context) {
        tabBar->setObjectName(var);
        context->registerVariable(var, tabBar);
        
        // 初始化变量值
        if (tabBar->count() > 0) {
            QString initialVal = tabBar->tabData(defaultIndex).toString();
            context->setValue(var, initialVal);
        }
        
        // 连接信号
        QObject::connect(tabBar, &QTabBar::currentChanged, [context, var, tabBar](int index) {
            QString val = tabBar->tabData(index).toString();
            context->setValue(var, val);
        });
    }
    
    applyCommonAttributes(tabBar, element, context);
    return tabBar;
}

QWidget* WidgetFactory::createHLayoutWidget(const QDomElement& element, QuikContext* context) {
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout(widget);
    
    // 边距处理
    QString margins = getAttribute(element, "margins");
    if (!margins.isEmpty()) {
        QStringList parts = margins.split(",");
        if (parts.size() == 4) {
            layout->setContentsMargins(parts[0].toInt(), parts[1].toInt(), 
                                       parts[2].toInt(), parts[3].toInt());
        }
    } else {
        layout->setContentsMargins(0, 0, 0, 0);
    }
    layout->setSpacing(5);
    
    applyCommonAttributes(widget, element, context);
    return widget;
}

QWidget* WidgetFactory::createVLayoutWidget(const QDomElement& element, QuikContext* context) {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    
    // 边距处理
    QString margins = getAttribute(element, "margins");
    if (!margins.isEmpty()) {
        QStringList parts = margins.split(",");
        if (parts.size() == 4) {
            layout->setContentsMargins(parts[0].toInt(), parts[1].toInt(), 
                                       parts[2].toInt(), parts[3].toInt());
        }
    } else {
        layout->setContentsMargins(0, 0, 0, 0);
    }
    layout->setSpacing(5);
    
    applyCommonAttributes(widget, element, context);
    return widget;
}

QWidget* WidgetFactory::createStretch(const QDomElement& element, QuikContext* context) {
    Q_UNUSED(context);
    // 创建一个特殊的占位widget，XMLUIBuilder会识别并添加stretch
    auto* widget = new QWidget();
    widget->setObjectName("__stretch__");
    
    int stretch = getIntAttribute(element, "stretch", 1);
    widget->setProperty("stretchFactor", stretch);
    
    return widget;
}

QWidget* WidgetFactory::createPointLineEdit(const QDomElement& element, QuikContext* context) {
    // 三维坐标输入组件：X, Y, Z三个输入框
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    
    QString var = getAttribute(element, "var");
    QString valid = getAttribute(element, "valid", "double");
    
    // 创建X, Y, Z三个输入框
    QStringList labels = {"X:", "Y:", "Z:"};
    for (int i = 0; i < 3; ++i) {
        auto* label = new QLabel(labels[i]);
        auto* edit = new QLineEdit("0");
        edit->setObjectName(var + "_" + QString::number(i));
        
        if (valid == "double") {
            edit->setValidator(new QDoubleValidator(edit));
        } else if (valid == "int") {
            edit->setValidator(new QIntValidator(edit));
        }
        
        layout->addWidget(label);
        layout->addWidget(edit, 1);
        
        // 注册子变量
        if (!var.isEmpty() && context) {
            context->registerVariable(var + "_" + QString::number(i), edit);
        }
    }
    
    // 是否有选择按钮
    bool hasButton = getBoolAttribute(element, "hasButton", false);
    if (hasButton) {
        auto* btn = new QPushButton("...");
        btn->setFixedWidth(30);
        btn->setObjectName(var + "_btn");
        layout->addWidget(btn);
        if (!var.isEmpty() && context) {
            context->registerVariable(var + "_btn", btn);
        }
    }
    
    applyCommonAttributes(widget, element, context);
    return widget;
}

QWidget* WidgetFactory::createTwoPointLineEdit(const QDomElement& element, QuikContext* context) {
    // 两点坐标输入组件
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    
    QString var = getAttribute(element, "var");
    QString leftLabel = getAttribute(element, "left", "Point 1");
    QString rightLabel = getAttribute(element, "right", "Point 2");
    QString valid = getAttribute(element, "valid", "double");
    
    // 第一行：左侧标签 + 三个输入框
    auto* row1 = new QWidget();
    auto* layout1 = new QHBoxLayout(row1);
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->setSpacing(5);
    
    layout1->addWidget(new QLabel(leftLabel));
    for (int i = 0; i < 3; ++i) {
        auto* edit = new QLineEdit("0");
        edit->setObjectName(var + "_p1_" + QString::number(i));
        if (valid == "double") {
            edit->setValidator(new QDoubleValidator(edit));
        }
        layout1->addWidget(edit, 1);
        if (!var.isEmpty() && context) {
            context->registerVariable(var + "_p1_" + QString::number(i), edit);
        }
    }
    layout->addWidget(row1);
    
    // 第二行：右侧标签 + 三个输入框
    auto* row2 = new QWidget();
    auto* layout2 = new QHBoxLayout(row2);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->setSpacing(5);
    
    layout2->addWidget(new QLabel(rightLabel));
    for (int i = 0; i < 3; ++i) {
        auto* edit = new QLineEdit("0");
        edit->setObjectName(var + "_p2_" + QString::number(i));
        if (valid == "double") {
            edit->setValidator(new QDoubleValidator(edit));
        }
        layout2->addWidget(edit, 1);
        if (!var.isEmpty() && context) {
            context->registerVariable(var + "_p2_" + QString::number(i), edit);
        }
    }
    layout->addWidget(row2);
    
    applyCommonAttributes(widget, element, context);
    return widget;
}

// ========== 辅助方法 ==========

void WidgetFactory::applyCommonAttributes(QWidget* widget, const QDomElement& element, QuikContext* context) {
    if (!widget) return;
    
    // 变量名（用于注册和绑定）
    QString var = getAttribute(element, "var");
    if (!var.isEmpty() && context) {
        widget->setObjectName(var);
        context->registerVariable(var, widget);
    }
    
    // tooltip
    QString tooltip = getAttribute(element, "tooltip");
    if (!tooltip.isEmpty()) {
        widget->setToolTip(tooltip);
    }
    
    // 固定宽度
    int fixedWidth = getIntAttribute(element, "fixedWidth", -1);
    if (fixedWidth > 0) {
        widget->setFixedWidth(fixedWidth);
    }
    
    // 固定高度
    int fixedHeight = getIntAttribute(element, "fixedHeight", -1);
    if (fixedHeight > 0) {
        widget->setFixedHeight(fixedHeight);
    }
    
    // visible/enabled 属性暂存到 property 中，由 XMLUIBuilder 统一处理
    // 这样可以支持带 title 的组件将绑定应用到整个行容器
    QString visible = getAttribute(element, "visible");
    if (!visible.isEmpty()) {
        widget->setProperty("_Quik_visible", visible);
    }
    
    QString enabled = getAttribute(element, "enabled");
    if (!enabled.isEmpty()) {
        widget->setProperty("_Quik_enabled", enabled);
    }
}

QString WidgetFactory::getAttribute(const QDomElement& element, const QString& name, const QString& defaultValue) {
    if (element.hasAttribute(name)) {
        return element.attribute(name);
    }
    return defaultValue;
}

bool WidgetFactory::getBoolAttribute(const QDomElement& element, const QString& name, bool defaultValue) {
    QString value = getAttribute(element, name);
    if (value.isEmpty()) {
        return defaultValue;
    }
    return value == "true" || value == "1" || value == "yes" || value == "on";
}

int WidgetFactory::getIntAttribute(const QDomElement& element, const QString& name, int defaultValue) {
    QString value = getAttribute(element, name);
    if (value.isEmpty()) {
        return defaultValue;
    }
    bool ok;
    int result = value.toInt(&ok);
    return ok ? result : defaultValue;
}

double WidgetFactory::getDoubleAttribute(const QDomElement& element, const QString& name, double defaultValue) {
    QString value = getAttribute(element, name);
    if (value.isEmpty()) {
        return defaultValue;
    }
    bool ok;
    double result = value.toDouble(&ok);
    return ok ? result : defaultValue;
}

QWidget* WidgetFactory::createSlider(const QDomElement& element, QuikContext* context) {
    auto* slider = new QSlider(Qt::Horizontal);
    
    int min = getIntAttribute(element, "min", 0);
    int max = getIntAttribute(element, "max", 100);
    int defaultVal = getIntAttribute(element, "default", min);
    int step = getIntAttribute(element, "step", 1);
    
    slider->setRange(min, max);
    slider->setValue(defaultVal);
    slider->setSingleStep(step);
    slider->setPageStep(step * 10);
    
    // 方向
    QString orientation = getAttribute(element, "orientation", "horizontal");
    if (orientation == "vertical" || orientation == "ver") {
        slider->setOrientation(Qt::Vertical);
    }
    
    // 刻度
    QString tickPosition = getAttribute(element, "ticks");
    if (tickPosition == "above") {
        slider->setTickPosition(QSlider::TicksAbove);
    } else if (tickPosition == "below") {
        slider->setTickPosition(QSlider::TicksBelow);
    } else if (tickPosition == "both") {
        slider->setTickPosition(QSlider::TicksBothSides);
    }
    
    int tickInterval = getIntAttribute(element, "tickInterval", 0);
    if (tickInterval > 0) {
        slider->setTickInterval(tickInterval);
    }
    
    applyCommonAttributes(slider, element, context);
    return slider;
}

QWidget* WidgetFactory::createProgressBar(const QDomElement& element, QuikContext* context) {
    auto* progressBar = new QProgressBar();
    
    int min = getIntAttribute(element, "min", 0);
    int max = getIntAttribute(element, "max", 100);
    int defaultVal = getIntAttribute(element, "default", min);
    
    progressBar->setRange(min, max);
    progressBar->setValue(defaultVal);
    
    // 方向
    QString orientation = getAttribute(element, "orientation", "horizontal");
    if (orientation == "vertical" || orientation == "ver") {
        progressBar->setOrientation(Qt::Vertical);
    }
    
    // 文本格式
    QString format = getAttribute(element, "format");
    if (!format.isEmpty()) {
        progressBar->setFormat(format);
    }
    
    // 是否显示文本
    bool textVisible = getBoolAttribute(element, "textVisible", true);
    progressBar->setTextVisible(textVisible);
    
    // 是否反向
    bool inverted = getBoolAttribute(element, "inverted", false);
    progressBar->setInvertedAppearance(inverted);
    
    applyCommonAttributes(progressBar, element, context);
    return progressBar;
}

} // namespace Quik
