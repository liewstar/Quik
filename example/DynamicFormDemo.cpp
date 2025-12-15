#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QDebug>
#include "Quik/Quik.h"

/**
 * @brief 动态配置切换示例
 * 
 * 演示通用 q-for 功能：
 * 1. ComboBox 选项数量动态
 * 2. 表单数量动态（根据 ComboBox 选择显示对应表单）
 * 3. 动态变量名绑定 (var="formData.$idx.name")
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QDialog dialog;
    dialog.setWindowTitle("Quik");
    dialog.resize(400, 350);
    
    auto* layout = new QVBoxLayout(&dialog);
    
    // 构建 UI
    Quik::XMLUIBuilder builder;
    QWidget* ui = Quik_BUILD(builder, "DynamicFormDemo.xml");
    layout->addWidget(ui);
    
    Quik::QuikViewModel vm(&builder);
    
    // ========== 配置数据 ==========
    auto configs = vm.list("configs");
    auto forms = vm.list("forms");
    int configCounter = 0;
    
    // 辅助函数：添加配置
    auto addConfig = [&]() {
        int idx = configCounter++;
        QString name = QString("配置%1").arg(idx + 1);
        
        // 添加到 ComboBox 选项
        QVariantList configList = configs();
        configList.append(QVariantMap{{"name", name}, {"id", idx}});
        configs(configList);
        
        // 添加表单
        QVariantList formList = forms();
        formList.append(QVariantMap{{"name", name}, {"id", idx}});
        forms(formList);
        
        // 初始化表单数据
        QString prefix = QString("formData.%1").arg(idx);
        builder.setValue(prefix + ".name", name + " default");
        builder.setValue(prefix + ".value", (idx + 1) * 10.0);
        builder.setValue(prefix + ".enabled", 0);
        
        qDebug() << "添加配置:" << name;
    };
    
    // 初始化两个配置
    addConfig();
    addConfig();
    
    // 设置默认选中第一个配置
    builder.setValue("selectedConfig", 0);
    
    // 监听配置切换
    vm.var<QString>("selectedConfig").watch([](const QString& idx) {
        qDebug() << "切换到配置:" << idx;
    });
    
    // ========== 按钮事件 ==========
    vm.button("btnAdd").onClick([&]() {
        addConfig();
    });
    
    vm.button("btnRemove").onClick([&]() {
        QVariantList configList = configs();
        QVariantList formList = forms();
        
        if (configList.size() > 1) {
            configList.removeLast();
            formList.removeLast();
            configs(configList);
            forms(formList);
            configCounter--;
            
            // 如果当前选中的被删除了，切换到第一个
            int selected = builder.getValue("selectedConfig").toInt();
            if (selected >= configList.size()) {
                builder.setValue("selectedConfig", 0);
            }
            
            qDebug() << "删除配置，剩余:" << configList.size();
        } else {
            qDebug() << "at least one";
        }
    });
    
    vm.button("btnPrint").onClick([&]() {
        qDebug() << "========== 所有配置数据 ==========";
        QVariantList formList = forms();
        for (int i = 0; i < formList.size(); i++) {
            QString prefix = QString("formData.%1").arg(i);
            qDebug() << "配置" << i << ":";
            qDebug() << "  name:" << builder.getValue(prefix + ".name").toString();
            qDebug() << "  value:" << builder.getValue(prefix + ".value").toDouble();
            qDebug() << "  enabled:" << builder.getValue(prefix + ".enabled").toInt();
        }
        qDebug() << "==================================";
    });
    
    return dialog.exec();
}
