#ifndef QuikViewModel_H
#define QuikViewModel_H

#include "Quik/QuikAPI.h"
#include <QString>
#include <QVariant>
#include <QMap>
#include <QVector3D>
#include <QPair>
#include <functional>
#include <initializer_list>

namespace Quik {

class XMLUIBuilder;

/**
 * @brief 变量基类 - 用于多变量监听
 */
class QUIK_API VarBase {
public:
    virtual ~VarBase() = default;
    virtual QString name() const = 0;
};

/**
 * @brief 变量访问器 - 提供类型安全的getter/setter/watch
 * 
 * 使用示例：
 *   auto maxSize = vm.var<double>("maxSize");
 *   double v = maxSize;          // 隐式获取值
 *   maxSize = 0.5;               // 赋值，UI自动更新
 *   maxSize.watch([](double v) { ... });  // 类型安全的监听
 */
template<typename T>
class Var : public VarBase {
public:
    Var() : m_getter(nullptr), m_setter(nullptr), m_watcher(nullptr) {}
    
    Var(const QString& name,
        std::function<T()> getter, 
        std::function<void(const T&)> setter,
        std::function<void(std::function<void(const T&)>)> watcher = nullptr)
        : m_name(name), m_getter(getter), m_setter(setter), m_watcher(watcher) {}
    
    // 获取变量名
    QString name() const override { return m_name; }
    
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
    
    // 监听变化 - 类型安全，无需硬编码变量名
    void watch(std::function<void(const T&)> callback) {
        if (m_watcher) {
            m_watcher(callback);
        }
    }

private:
    QString m_name;
    std::function<T()> m_getter;
    std::function<void(const T&)> m_setter;
    std::function<void(std::function<void(const T&)>)> m_watcher;
};

/**
 * @brief 按钮访问器 - 提供类型安全的按钮事件绑定
 * 
 * 使用示例：
 *   auto btnApply = vm.button("btnApply");
 *   btnApply.onClick([&]() { ... });
 */
class QUIK_API ButtonVar {
public:
    ButtonVar() : m_connector(nullptr) {}
    
    ButtonVar(std::function<void(std::function<void()>)> connector)
        : m_connector(connector) {}
    
    // 绑定点击事件
    void onClick(std::function<void()> callback) {
        if (m_connector) m_connector(callback);
    }

private:
    std::function<void(std::function<void()>)> m_connector;
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
 * @brief 三维坐标访问器 - 用于 PointLineEdit
 * 
 * 使用示例：
 *   auto origin = vm.point("origin");
 *   origin = QVector3D(1, 2, 3);
 *   origin.watch([](QVector3D pos) { ... });
 *   origin.button().onClick([&]() { ... });
 */
class QUIK_API PointVar : public Var<QVector3D> {
public:
    PointVar() : Var<QVector3D>(), m_button() {}
    
    PointVar(const QString& name,
             std::function<QVector3D()> getter,
             std::function<void(const QVector3D&)> setter,
             std::function<void(std::function<void(const QVector3D&)>)> watcher,
             ButtonVar button)
        : Var<QVector3D>(name, getter, setter, watcher), m_button(button) {}
    
    // 继承父类的赋值运算符
    using Var<QVector3D>::operator=;
    
    // 获取关联的选择按钮
    ButtonVar button() const { return m_button; }

private:
    ButtonVar m_button;
};

/**
 * @brief 两点坐标访问器 - 用于 TwoPointLineEdit
 * 
 * 使用示例：
 *   auto range = vm.twoPoint("range");
 *   range = qMakePair(QVector3D(0,0,0), QVector3D(100,100,100));
 *   range.watch([](QPair<QVector3D, QVector3D> pts) { ... });
 */
class QUIK_API TwoPointVar : public Var<QPair<QVector3D, QVector3D>> {
public:
    using PointPair = QPair<QVector3D, QVector3D>;
    
    TwoPointVar() : Var<PointPair>() {}
    
    TwoPointVar(const QString& name,
                std::function<PointPair()> getter,
                std::function<void(const PointPair&)> setter,
                std::function<void(std::function<void(const PointPair&)>)> watcher)
        : Var<PointPair>(name, getter, setter, watcher) {}
    
    // 继承父类的赋值运算符
    using Var<PointPair>::operator=;
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
            name,
            [this, name]() -> T { return getValue<T>(name); },
            [this, name](const T& v) { setValue<T>(name, v); },
            [this, name](std::function<void(const T&)> callback) {
                watchVar<T>(name, callback);
            }
        );
    }
    
    /**
     * @brief 创建列表数据访问器（用于 q-for）
     * @param name 数据源名称（对应 q-for="item in name" 中的 name）
     * @return 列表访问器
     */
    ListVar list(const QString& name);
    
    /**
     * @brief 创建按钮访问器
     * @param name 按钮变量名
     * @return 按钮访问器
     */
    ButtonVar button(const QString& name);
    
    /**
     * @brief 创建三维坐标访问器（用于 PointLineEdit）
     * @param name 变量名
     * @return 坐标访问器（支持 watch 和 button）
     */
    PointVar point(const QString& name);
    
    /**
     * @brief 创建两点坐标访问器（用于 TwoPointLineEdit）
     * @param name 变量名
     * @return 两点坐标访问器（支持 watch）
     */
    TwoPointVar twoPoint(const QString& name);
    
    /**
     * @brief 监听多个变量（任一变化触发）
     * @param vars 变量列表
     * @param callback 回调函数
     * 
     * 使用示例：
     * @code
     * vm.watch({enable, mode, count}, [&]() {
     *     // 任意一个变化时触发
     * });
     * @endcode
     */
    void watch(std::initializer_list<std::reference_wrapper<const VarBase>> vars,
               std::function<void()> callback);
    
    /**
     * @brief 监听所有变量变化
     * @param callback 回调函数，参数为变量名和新值
     * 
     * 使用示例：
     * @code
     * vm.watchAll([&](const QString& name, const QVariant& value) {
     *     // 任意变量变化时触发
     * });
     * @endcode
     */
    void watchAll(std::function<void(const QString&, const QVariant&)> callback);
    
    // 获取原始builder
    XMLUIBuilder* builder() const { return m_builder; }

private:
    template<typename T>
    T getValue(const QString& name) const;
    
    template<typename T>
    void setValue(const QString& name, const T& value);
    
    template<typename T>
    void watchVar(const QString& name, std::function<void(const T&)> callback);
    
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

template<> QUIK_API void QuikViewModel::watchVar<bool>(const QString& name, std::function<void(const bool&)> callback);
template<> QUIK_API void QuikViewModel::watchVar<int>(const QString& name, std::function<void(const int&)> callback);
template<> QUIK_API void QuikViewModel::watchVar<double>(const QString& name, std::function<void(const double&)> callback);
template<> QUIK_API void QuikViewModel::watchVar<QString>(const QString& name, std::function<void(const QString&)> callback);

} // namespace Quik

#endif // QuikViewModel_H
