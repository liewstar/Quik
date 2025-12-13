#include "QuikViewModel.h"
#include "parser/XMLUIBuilder.h"
#include <QVector3D>

namespace Quik {

QuikViewModel::QuikViewModel(XMLUIBuilder* builder)
    : m_builder(builder)
{
}

// bool 特化
template<>
bool QuikViewModel::getValue<bool>(const QString& name) const {
    return m_builder->getValue(name).toInt() != 0;
}

template<>
void QuikViewModel::setValue<bool>(const QString& name, const bool& value) {
    m_builder->setValue(name, value ? 1 : 0);
}

// int 特化
template<>
int QuikViewModel::getValue<int>(const QString& name) const {
    return m_builder->getValue(name).toInt();
}

template<>
void QuikViewModel::setValue<int>(const QString& name, const int& value) {
    m_builder->setValue(name, value);
}

// double 特化
template<>
double QuikViewModel::getValue<double>(const QString& name) const {
    return m_builder->getValue(name).toDouble();
}

template<>
void QuikViewModel::setValue<double>(const QString& name, const double& value) {
    m_builder->setValue(name, QString::number(value));
}

// QString 特化
template<>
QString QuikViewModel::getValue<QString>(const QString& name) const {
    return m_builder->getValue(name).toString();
}

template<>
void QuikViewModel::setValue<QString>(const QString& name, const QString& value) {
    m_builder->setValue(name, value);
}

// QVector3D 特化 - 用于PointLineEdit
template<>
QVector3D QuikViewModel::getValue<QVector3D>(const QString& name) const {
    double x = m_builder->getValue(name + "_0").toDouble();
    double y = m_builder->getValue(name + "_1").toDouble();
    double z = m_builder->getValue(name + "_2").toDouble();
    return QVector3D(x, y, z);
}

template<>
void QuikViewModel::setValue<QVector3D>(const QString& name, const QVector3D& value) {
    m_builder->setValue(name + "_0", QString::number(value.x()));
    m_builder->setValue(name + "_1", QString::number(value.y()));
    m_builder->setValue(name + "_2", QString::number(value.z()));
}

// QPair<QVector3D, QVector3D> 特化 - 用于TwoPointLineEdit
template<>
QPair<QVector3D, QVector3D> QuikViewModel::getValue<QPair<QVector3D, QVector3D>>(const QString& name) const {
    QVector3D p1(
        m_builder->getValue(name + "_p1_0").toDouble(),
        m_builder->getValue(name + "_p1_1").toDouble(),
        m_builder->getValue(name + "_p1_2").toDouble()
    );
    QVector3D p2(
        m_builder->getValue(name + "_p2_0").toDouble(),
        m_builder->getValue(name + "_p2_1").toDouble(),
        m_builder->getValue(name + "_p2_2").toDouble()
    );
    return qMakePair(p1, p2);
}

template<>
void QuikViewModel::setValue<QPair<QVector3D, QVector3D>>(const QString& name, const QPair<QVector3D, QVector3D>& value) {
    m_builder->setValue(name + "_p1_0", QString::number(value.first.x()));
    m_builder->setValue(name + "_p1_1", QString::number(value.first.y()));
    m_builder->setValue(name + "_p1_2", QString::number(value.first.z()));
    m_builder->setValue(name + "_p2_0", QString::number(value.second.x()));
    m_builder->setValue(name + "_p2_1", QString::number(value.second.y()));
    m_builder->setValue(name + "_p2_2", QString::number(value.second.z()));
}

// 列表数据访问器（用于 q-for）
ListVar QuikViewModel::list(const QString& name) {
    return ListVar(
        [this, name]() -> QVariantList { return m_builder->getListData(name); },
        [this, name](const QVariantList& v) { m_builder->setListData(name, v); }
    );
}

// watchVar 特化 - 类型安全的监听
template<>
void QuikViewModel::watchVar<bool>(const QString& name, std::function<void(const bool&)> callback) {
    m_builder->watch(name, [callback](const QVariant& v) {
        callback(v.toInt() != 0);
    });
}

template<>
void QuikViewModel::watchVar<int>(const QString& name, std::function<void(const int&)> callback) {
    m_builder->watch(name, [callback](const QVariant& v) {
        callback(v.toInt());
    });
}

template<>
void QuikViewModel::watchVar<double>(const QString& name, std::function<void(const double&)> callback) {
    m_builder->watch(name, [callback](const QVariant& v) {
        callback(v.toDouble());
    });
}

template<>
void QuikViewModel::watchVar<QString>(const QString& name, std::function<void(const QString&)> callback) {
    m_builder->watch(name, [callback](const QVariant& v) {
        callback(v.toString());
    });
}

// 创建按钮访问器
ButtonVar QuikViewModel::button(const QString& name) {
    return ButtonVar(
        [this, name](std::function<void()> callback) {
            m_builder->connectButton(name, callback);
        }
    );
}

} // namespace Quik
