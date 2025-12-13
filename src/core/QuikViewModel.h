#ifndef QuikViewModel_H
#define QuikViewModel_H

#include "Quik/QuikAPI.h"
#include <QString>
#include <QVariant>
#include <QMap>
#include <functional>

namespace Quik {

class XMLUIBuilder;

/**
 * @brief 变量访问器 - 提供类型安全的getter/setter
 * 
 * 使用示例：
 *   Var<double> maxSize = vm.var<double>("maxSize");
 *   double v = maxSize();        // 获取值
 *   maxSize(0.5);                // 设置值，UI自动更新
 */
template<typename T>
class Var {
public:
    Var() : m_getter(nullptr), m_setter(nullptr) {}
    
    Var(std::function<T()> getter, std::function<void(const T&)> setter)
        : m_getter(getter), m_setter(setter) {}
    
    // 获取值 - 函数调用风格
    T operator()() const {
        if (m_getter) {
            return m_getter();
        }
        return T();
    }
    
    // 设置值 - 函数调用风格
    void operator()(const T& val) {
        if (m_setter) {
            m_setter(val);
        }
    }
    
    // 隐式转换为T - 可以直接当T用
    operator T() const {
        if (m_getter) {
            return m_getter();
        }
        return T();
    }
    
    // 赋值操作符 - 可以直接赋值
    Var& operator=(const T& val) {
        if (m_setter) {
            m_setter(val);
        }
        return *this;
    }
    
    // 获取值（显式方法）
    T value() const {
        if (m_getter) {
            return m_getter();
        }
        return T();
    }
    
    // 设置值（显式方法）
    void setValue(const T& val) {
        if (m_setter) {
            m_setter(val);
        }
    }

private:
    std::function<T()> m_getter;
    std::function<void(const T&)> m_setter;
};

/**
 * @brief 列表数据访问器 - 用于 q-for 数据源
 * 
 * 使用示例：
 *   ListVar modes = vm.list("modes");
 *   modes = {
 *       {{"text", "模式一"}, {"val", "mode1"}},
 *       {{"text", "模式二"}, {"val", "mode2"}}
 *   };
 */
class QUIK_API ListVar {
public:
    ListVar() : m_getter(nullptr), m_setter(nullptr) {}
    
    ListVar(std::function<QVariantList()> getter, std::function<void(const QVariantList&)> setter)
        : m_getter(getter), m_setter(setter) {}
    
    // 获取值
    QVariantList operator()() const {
        return m_getter ? m_getter() : QVariantList();
    }
    
    // 设置值
    void operator()(const QVariantList& val) {
        if (m_setter) m_setter(val);
    }
    
    // 赋值操作符
    ListVar& operator=(const QVariantList& val) {
        if (m_setter) m_setter(val);
        return *this;
    }
    
    // 支持初始化列表赋值
    ListVar& operator=(std::initializer_list<QVariantMap> items) {
        QVariantList list;
        for (const auto& item : items) {
            list << item;
        }
        if (m_setter) m_setter(list);
        return *this;
    }
    
    // 追加项
    void append(const QVariantMap& item) {
        QVariantList list = m_getter ? m_getter() : QVariantList();
        list << item;
        if (m_setter) m_setter(list);
    }
    
    // 清空
    void clear() {
        if (m_setter) m_setter(QVariantList());
    }

private:
    std::function<QVariantList()> m_getter;
    std::function<void(const QVariantList&)> m_setter;
};

/**
 * @brief 响应式视图模型 - 提供类型安全的变量访问
 * 
 * 使用示例：
 *   QuikViewModel vm(builder);
 *   
 *   // 定义变量访问器
 *   auto enableRefine = vm.var<bool>("enableRefine");
 *   auto maxSize = vm.var<double>("maxSize");
 *   auto mode = vm.var<QString>("boundaryOption");
 *   
 *   // 使用 - IDE能补全！
 *   if (enableRefine) {
 *       double v = maxSize;      // 隐式转换
 *       maxSize = 0.5;           // 赋值，UI自动更新
 *   }
 *   mode = "on";                 // QString也支持
 *   
 *   // q-for 数据源
 *   auto modes = vm.list("modes");
 *   modes = {{{"text", "模式一"}, {"val", "mode1"}}};
 */
class QUIK_API QuikViewModel {
public:
    explicit QuikViewModel(XMLUIBuilder* builder);
    
    /**
     * @brief 创建类型安全的变量访问器
     * @tparam T 变量类型（bool, int, double, QString）
     * @param name 变量名（对应XML中的var属性）
     * @return 变量访问器
     */
    template<typename T>
    Var<T> var(const QString& name) {
        return Var<T>(
            [this, name]() -> T { return getValue<T>(name); },
            [this, name](const T& v) { setValue<T>(name, v); }
        );
    }
    
    /**
     * @brief 创建列表数据访问器（用于 q-for）
     * @param name 数据源名称（对应 q-for="item in name" 中的 name）
     * @return 列表访问器
     */
    ListVar list(const QString& name);
    
    // 获取原始builder
    XMLUIBuilder* builder() const { return m_builder; }

private:
    template<typename T>
    T getValue(const QString& name) const;
    
    template<typename T>
    void setValue(const QString& name, const T& value);
    
    XMLUIBuilder* m_builder;
};

// 模板特化声明
template<> QUIK_API bool QuikViewModel::getValue<bool>(const QString& name) const;
template<> QUIK_API int QuikViewModel::getValue<int>(const QString& name) const;
template<> QUIK_API double QuikViewModel::getValue<double>(const QString& name) const;
template<> QUIK_API QString QuikViewModel::getValue<QString>(const QString& name) const;

template<> QUIK_API void QuikViewModel::setValue<bool>(const QString& name, const bool& value);
template<> QUIK_API void QuikViewModel::setValue<int>(const QString& name, const int& value);
template<> QUIK_API void QuikViewModel::setValue<double>(const QString& name, const double& value);
template<> QUIK_API void QuikViewModel::setValue<QString>(const QString& name, const QString& value);

} // namespace Quik

#endif // QuikViewModel_H
