#ifndef Quik_H
#define Quik_H

/**
 * @file Quik.h
 * @brief 响应式XML UI框架 - 主头文件
 * 
 * 这是一个基于Qt的响应式XML UI框架，支持：
 * - XML声明式UI定义
 * - 数据双向绑定
 * - 条件表达式（visible="$varName==value"）
 * - 自动响应式更新
 * - 类型安全的ViewModel访问
 * 
 * 使用示例（推荐 - ViewModel方式）：
 * @code
 * #include "Quik/Quik.h"
 * 
 * Quik::XMLUIBuilder builder;
 * QWidget* ui = builder.buildFromString(xmlContent);
 * 
 * // 创建ViewModel - 类型安全的变量访问
 * Quik::QuikViewModel vm(&builder);
 * 
 * // 定义变量访问器 - IDE能补全！
 * auto enableRefine = vm.var<bool>("enableRefine");
 * auto maxSize = vm.var<double>("maxSize");
 * auto mode = vm.var<QString>("boundaryOption");
 * 
 * // 使用 - 像普通变量一样
 * if (enableRefine) {                // 隐式转换为bool
 *     double v = maxSize;            // 隐式转换为double
 *     maxSize = 0.5;                 // 赋值，UI自动更新
 * }
 * mode = "on";                       // QString也支持
 * @endcode
 * 
 * XML语法示例：
 * @code{.xml}
 * <MyPanel>
 *     <GroupBox title="Settings">
 *         <CheckBox title="Enable Feature" var="chkEnable" default="1"/>
 *         <LineEdit title="Value" var="lneValue" visible="$chkEnable==1" valid="double"/>
 *         <ComboBox title="Type" var="cboType">
 *             <Choice text="Option A" val="vA"/>
 *             <Choice text="Option B" val="vB"/>
 *         </ComboBox>
 *         <LineEdit title="Extra" var="lneExtra" visible="$cboType==vB"/>
 *     </GroupBox>
 *     <PushButton text="Apply" var="btnApply"/>
 * </MyPanel>
 * @endcode
 */

#include "Quik/QuikAPI.h"
#include "parser/ExpressionParser.h"
#include "core/QuikContext.h"
#include "widget/WidgetFactory.h"
#include "parser/XMLUIBuilder.h"
#include "core/QuikViewModel.h"
#include <QFileInfo>

/**
 * @brief 获取与当前源文件同级目录下的XML文件路径
 * @param filename XML文件名（如 "MyPanel.xml"）
 * 
 * 使用示例：
 * @code
 * // 假设当前文件是 MyManager.cpp，XML文件 MyPanel.xml 放在同级目录
 * QString xmlPath = Quik_XML("MyPanel.xml");
 * QWidget* ui = builder.buildFromFile(xmlPath);
 * @endcode
 */
inline QString QuikXmlPath(const char* file, const char* xmlName) {
    QString path = QString::fromLocal8Bit(file);
    int lastSlash = path.lastIndexOf('/');
    int lastBackslash = path.lastIndexOf('\\');
    int pos = qMax(lastSlash, lastBackslash);
    if (pos >= 0) {
        return path.left(pos + 1) + QString::fromLocal8Bit(xmlName);
    }
    // __FILE__ 没有路径信息，返回原始文件名（需要用户确保工作目录正确）
    return QString::fromLocal8Bit(xmlName);
}
#define Quik_XML(filename) QuikXmlPath(__FILE__, filename)

/**
 * @brief 从与当前源文件同级目录加载XML并构建UI（一行代码完成）
 * @param builder XMLUIBuilder实例
 * @param filename XML文件名
 * 
 * 使用示例：
 * @code
 * Quik::XMLUIBuilder builder;
 * QWidget* ui = Quik_BUILD(builder, "MyPanel.xml");
 * @endcode
 */
#define Quik_BUILD(builder, filename) \
    (builder).buildFromFile(Quik_XML(filename))

#endif // Quik_H
