#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QDebug>
#include "Quik/Quik.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Create dialog
    QDialog dialog;
    dialog.setWindowTitle("Quik Example");
    dialog.resize(400, 300);
    
    auto* layout = new QVBoxLayout(&dialog);
    
    // Build UI from XML file
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
    
    // Watch variable changes
    builder.watch("chkEnable", [](const QVariant& v) {
        qDebug() << "chkEnable changed:" << v.toBool();
    });
    
    builder.watch("cboMode", [](const QVariant& v) {
        qDebug() << "cboMode changed:" << v.toString();
    });
    
    // Connect buttons
    builder.connectButton("btnApply", [&]() {
        qDebug() << "========== Apply ==========";
        qDebug() << "Enable:" << chkEnable();
        qDebug() << "Value:" << txtValue();
        qDebug() << "Mode:" << cboMode();
        qDebug() << "Count:" << spnCount();
        qDebug() << "===========================";
    });
    
    builder.connectButton("btnCancel", [&dialog]() {
        dialog.reject();
    });
    
    return dialog.exec();
}
