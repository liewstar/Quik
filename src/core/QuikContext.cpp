#include "QuikContext.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QLabel>
#include <QDebug>

namespace Quik {

QuikContext::QuikContext(QObject* parent)
    : QObject(parent)
{
    connect(this, &QuikContext::variableChanged,
            this, &QuikContext::onVariableChanged);
}

QuikContext::~QuikContext() = default;

// ========== 变量管理 ==========

void QuikContext::registerVariable(const QString& name, QWidget* widget) {
    // 添加到组件列表（支持多个组件绑定同一变量）
    m_widgets[name].append(widget);
    
    // 如果变量已有值，先同步到新组件
    if (m_values.contains(name)) {
        syncSingleWidget(widget, m_values[name]);
    }
    
    // 自动连接组件的值变化信号
    autoConnectWidget(name, widget);
    
    qDebug() << "[Quik] Registered variable:" << name << "(total widgets:" << m_widgets[name].size() << ")";
}

void QuikContext::setValue(const QString& name, const QVariant& value) {
    if (m_values.value(name) != value) {
        m_values[name] = value;
        
        // 同步更新UI组件
        syncWidgetFromValue(name, value);
        emit variableChanged(name, value);
        
        // 触发单变量监听回调
        if (m_watchers.contains(name)) {
            m_watchers[name](value);
        }
    }
}

QVariant QuikContext::getValue(const QString& name) const {
    return m_values.value(name);
}

QWidget* QuikContext::getWidget(const QString& name) const {
    const QList<QWidget*>& widgets = m_widgets.value(name);
    return widgets.isEmpty() ? nullptr : widgets.first();
}

// ========== 属性绑定 ==========

void QuikContext::bindVisible(QWidget* widget, const QString& expression) {
    bindProperty(widget, "visible", expression);
}

void QuikContext::bindEnabled(QWidget* widget, const QString& expression) {
    bindProperty(widget, "enabled", expression);
}

void QuikContext::bindProperty(QWidget* widget, const QString& property, const QString& expression) {
    if (!widget || expression.isEmpty()) {
        return;
    }
    
    PropertyBinding binding;
    binding.widget = widget;
    binding.property = property;
    binding.expression = expression;
    
    // 检查是否是复合表达式（包含 and/or）
    QString cleanExpr = expression.trimmed();
    bool isCompound = cleanExpr.contains(" and ", Qt::CaseInsensitive) || 
                      cleanExpr.contains(" or ", Qt::CaseInsensitive);
    
    if (isCompound) {
        // 复合表达式：提取所有变量并添加依赖
        QStringList vars = ExpressionParser::extractVariables(expression);
        for (const QString& var : vars) {
            m_dependencies[var].append(binding);
            qDebug() << "[Quik] Tracking variable for compound expr:" << var;
        }
        binding.condition.isValid = true;  // 标记为有效，但使用expression求值
    } else {
        // 简单表达式
        binding.condition = ExpressionParser::parse(expression);
        
        if (!binding.condition.isValid) {
            qWarning() << "[Quik] Failed to parse expression:" << expression;
            return;
        }
        
        // 添加到左侧变量的依赖列表
        m_dependencies[binding.condition.variable].append(binding);
        
        // 如果右侧也是变量，也添加到右侧变量的依赖列表
        if (binding.condition.isRightVariable && !binding.condition.compareVariable.isEmpty()) {
            m_dependencies[binding.condition.compareVariable].append(binding);
            qDebug() << "[Quik] Also tracking right variable:" << binding.condition.compareVariable;
        }
    }
    
    m_allBindings.append(binding);
    
    qDebug() << "[Quik] Bound" << property << "of widget to expression:" << expression;
}

// ========== 响应式更新 ==========

void QuikContext::initializeBindings() {
    qDebug() << "[Quik] Initializing" << m_allBindings.size() << "bindings";
    
    for (const PropertyBinding& binding : m_allBindings) {
        applyBinding(binding);
    }
}

QVariantMap QuikContext::getContext() const {
    return m_values;
}

void QuikContext::onVariableChanged(const QString& name, const QVariant& value) {
    qDebug() << "[Quik] Variable changed:" << name << "=" << value;
    updateDependentBindings(name);
}

void QuikContext::updateDependentBindings(const QString& varName) {
    const QList<PropertyBinding>& bindings = m_dependencies.value(varName);
    
    for (const PropertyBinding& binding : bindings) {
        applyBinding(binding);
    }
}

void QuikContext::applyBinding(const PropertyBinding& binding) {
    if (!binding.widget) {
        return;
    }
    
    // 使用表达式字符串求值（支持复合表达式 and/or）
    bool result = ExpressionParser::evaluate(binding.expression, m_values);
    
    if (binding.property == "visible") {
        binding.widget->setVisible(result);
    } else if (binding.property == "enabled") {
        binding.widget->setEnabled(result);
    }
    
    qDebug() << "[Quik] Applied" << binding.property << "=" << result 
             << "for expression:" << binding.expression;
}

void QuikContext::autoConnectWidget(const QString& name, QWidget* widget) {
    // QCheckBox
    if (auto* checkBox = qobject_cast<QCheckBox*>(widget)) {
        // 初始化值
        m_values[name] = checkBox->isChecked() ? 1 : 0;
        
        // 连接信号
        connect(checkBox, &QCheckBox::toggled, this, [this, name](bool checked) {
            setValue(name, checked ? 1 : 0);
        });
        return;
    }
    
    // QRadioButton
    if (auto* radioButton = qobject_cast<QRadioButton*>(widget)) {
        m_values[name] = radioButton->isChecked() ? 1 : 0;
        
        connect(radioButton, &QRadioButton::toggled, this, [this, name](bool checked) {
            setValue(name, checked ? 1 : 0);
        });
        return;
    }
    
    // QComboBox - 特殊处理，使用itemData作为值
    if (auto* comboBox = qobject_cast<QComboBox*>(widget)) {
        // 初始化值（使用当前选中项的data，如果没有则用text）
        QVariant data = comboBox->currentData();
        m_values[name] = data.isValid() ? data : comboBox->currentText();
        
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this, name, comboBox](int index) {
            QVariant data = comboBox->itemData(index);
            setValue(name, data.isValid() ? data : comboBox->itemText(index));
        });
        return;
    }
    
    // QLineEdit
    if (auto* lineEdit = qobject_cast<QLineEdit*>(widget)) {
        m_values[name] = lineEdit->text();
        
        connect(lineEdit, &QLineEdit::textChanged, this, [this, name](const QString& text) {
            setValue(name, text);
        });
        return;
    }
    
    // QSpinBox
    if (auto* spinBox = qobject_cast<QSpinBox*>(widget)) {
        m_values[name] = spinBox->value();
        
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this, name](int value) {
            setValue(name, value);
        });
        return;
    }
    
    // QDoubleSpinBox
    if (auto* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
        m_values[name] = doubleSpinBox->value();
        
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this, name](double value) {
            setValue(name, value);
        });
        return;
    }
    
    // QLabel - 只读显示，只需初始化值
    if (auto* label = qobject_cast<QLabel*>(widget)) {
        m_values[name] = label->text();
        return;
    }
    
    qDebug() << "[Quik] No auto-connect for widget type:" << widget->metaObject()->className();
}

void QuikContext::syncWidgetFromValue(const QString& name, const QVariant& value) {
    const QList<QWidget*>& widgets = m_widgets.value(name);
    
    // 同步所有绑定到该变量的组件
    for (QWidget* widget : widgets) {
        syncSingleWidget(widget, value);
    }
    
    if (!widgets.isEmpty()) {
        qDebug() << "[Quik] Synced" << widgets.size() << "widget(s) from value:" << name << "=" << value;
    }
}

void QuikContext::syncSingleWidget(QWidget* widget, const QVariant& value) {
    if (!widget) return;
    
    // 阻止信号，避免循环触发
    widget->blockSignals(true);
    
    // QCheckBox
    if (auto* checkBox = qobject_cast<QCheckBox*>(widget)) {
        checkBox->setChecked(value.toInt() != 0);
    }
    // QRadioButton
    else if (auto* radioButton = qobject_cast<QRadioButton*>(widget)) {
        radioButton->setChecked(value.toInt() != 0);
    }
    // QComboBox - 根据itemData查找并选中
    else if (auto* comboBox = qobject_cast<QComboBox*>(widget)) {
        QString valStr = value.toString();
        for (int i = 0; i < comboBox->count(); ++i) {
            QVariant data = comboBox->itemData(i);
            if ((data.isValid() && data.toString() == valStr) || 
                comboBox->itemText(i) == valStr) {
                comboBox->setCurrentIndex(i);
                break;
            }
        }
    }
    // QLineEdit
    else if (auto* lineEdit = qobject_cast<QLineEdit*>(widget)) {
        lineEdit->setText(value.toString());
    }
    // QSpinBox
    else if (auto* spinBox = qobject_cast<QSpinBox*>(widget)) {
        spinBox->setValue(value.toInt());
    }
    // QDoubleSpinBox
    else if (auto* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
        doubleSpinBox->setValue(value.toDouble());
    }
    // QLabel - 用于LabelList中的Item
    else if (auto* label = qobject_cast<QLabel*>(widget)) {
        label->setText(value.toString());
    }
    
    widget->blockSignals(false);
}

// ========== 单变量监听 ==========

void QuikContext::watch(const QString& name, std::function<void(const QVariant&)> callback) {
    m_watchers[name] = callback;
    qDebug() << "[Quik] Watching variable:" << name;
}

void QuikContext::unwatch(const QString& name) {
    m_watchers.remove(name);
    qDebug() << "[Quik] Unwatched variable:" << name;
}

} // namespace Quik
