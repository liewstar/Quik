#ifndef ALLWIDGETSNATIVE_H
#define ALLWIDGETSNATIVE_H

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSlider>
#include <QDial>
#include <QProgressBar>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QDateTimeEdit>
#include <QListWidget>
#include <QTabBar>
#include <QLabel>
#include <QGroupBox>

class AllWidgetsNative : public QWidget
{
    Q_OBJECT
public:
    explicit AllWidgetsNative(QWidget *parent = nullptr);

private:
    void setupUI();

    // Buttons
    QPushButton *btnDefault;
    QPushButton *btnToggle;
    QPushButton *btnFlat;
    QPushButton *btnTool;
    QPushButton *btnMenu;
    QRadioButton *radio1;
    QRadioButton *radio2;
    QRadioButton *radio3;
    QCheckBox *triCheck;

    // Simple Input Widgets
    QLineEdit *password;
    QSpinBox *spinbox;
    QDateTimeEdit *datetime;
    QSlider *slider;
    QDial *dial;

    // Tab Views
    QTabBar *tabView;
    QListWidget *listView;

    // More Input Widgets
    QComboBox *styleCombo;
    QDoubleSpinBox *doubleSpinbox;
    QLineEdit *validatedEdit;

    // Text Editors
    QTextEdit *richTextEdit;
    QPlainTextEdit *plainTextEdit;

    // Progress
    QProgressBar *progress;
    QLabel *progressLabel;

    // Bottom Buttons
    QPushButton *btnClose;
    QPushButton *btnHelp;
};

#endif // ALLWIDGETSNATIVE_H
