#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include "Quik/Quik.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Create dialog
    QDialog dialog;
    dialog.setWindowTitle("Quik Example");
    dialog.resize(400, 300);
    
    auto* layout = new QVBoxLayout(&dialog);
    
    // Build UI from XML file (热更新已自动启用)
    Quik::XMLUIBuilder builder;
    QWidget* ui = Quik_BUILD(builder, "ExamplePanel.xml");
    layout->addWidget(ui);
    
    // Create ViewModel
    Quik::QuikViewModel vm(&builder);
    
    // Define variable accessors
    auto chkEnable = vm.var<bool>("chkEnable");
    auto txtValue = vm.var<double>("txtValue");
    auto cboMode = vm.var<QString>("cboMode");
    auto spnCount = vm.var<int>("spnCount");
    auto volume = vm.var<int>("volume");
    auto progress = vm.var<int>("progress");
    
    // 定义 q-for 数据源访问器
    auto modes = vm.list("modes");
    
    // 设置初始数据（支持初始化列表语法）
    // 键名可以自定义，只要和XML中的 $item.xxx 对应即可
    modes = {
        {{"label", "mode1"}, {"value", "mode1"}},
        {{"label", "mode2"}, {"value", "mode2"}},
        {{"label", "mode3"}, {"value", "mode3"}}
    };
    
    // 类型安全的监听 - 无需硬编码变量名
    chkEnable.watch([](bool v) {
        qDebug() << "chkEnable changed:" << v;
    });
    
    cboMode.watch([](QString v) {
        qDebug() << "cboMode changed:" << v;
    });
    
    // 监听滑块变化，同步更新进度条
    volume.watch([&](int v) {
        qDebug() << "volume changed:" << v;
        progress = v;
    });
    
    // 定义按钮访问器
    auto btnApply = vm.button("btnApply");
    auto btnCancel = vm.button("btnCancel");
    
    // Connect buttons - 使用 btnApply.onClick()
    btnApply.onClick([&]() {
        qDebug() << "========== Apply ==========";
        qDebug() << "Enable:" << chkEnable();
        qDebug() << "Value:" << txtValue();
        qDebug() << "Mode:" << cboMode();
        qDebug() << "Count:" << spnCount();
        qDebug() << "Volume:" << volume();
        qDebug() << "Progress:" << progress();
        qDebug() << "===========================";
        
        // 测试：点击应用后动态添加一个新模式
        static int counter = 3;
        modes.append({{"label", QString("new mode%1").arg(++counter)}, {"value", QString("mode%1").arg(counter)}});
        spnCount = spnCount + 1;
        qDebug() << "已动态添加新模式到 ComboBox";
    });
    
    btnCancel.onClick([&dialog]() {
        dialog.reject();
    });
    
    return dialog.exec();
}
