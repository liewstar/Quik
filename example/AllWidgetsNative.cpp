#include "AllWidgetsNative.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDateTime>

AllWidgetsNative::AllWidgetsNative(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AllWidgetsNative::setupUI()
{
    setWindowTitle("Widget Gallery (Native)");
    resize(700, 600);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // ========== 左侧列 ==========
    QVBoxLayout *leftColumn = new QVBoxLayout();

    // --- 按钮组 ---
    QGroupBox *buttonsGroup = new QGroupBox("Buttons");
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsGroup);

    // 左侧按钮列
    QVBoxLayout *buttonsLeftLayout = new QVBoxLayout();
    btnDefault = new QPushButton("Default Push Button");
    btnToggle = new QPushButton("Toggle Push Button");
    btnFlat = new QPushButton("Flat Push Button");
    QHBoxLayout *toolMenuLayout = new QHBoxLayout();
    btnTool = new QPushButton("Tool Button");
    btnTool->setFixedWidth(80);
    btnMenu = new QPushButton("Menu Button");
    btnMenu->setFixedWidth(80);
    toolMenuLayout->addWidget(btnTool);
    toolMenuLayout->addWidget(btnMenu);
    buttonsLeftLayout->addWidget(btnDefault);
    buttonsLeftLayout->addWidget(btnToggle);
    buttonsLeftLayout->addWidget(btnFlat);
    buttonsLeftLayout->addLayout(toolMenuLayout);

    // 右侧单选框列
    QVBoxLayout *buttonsRightLayout = new QVBoxLayout();
    radio1 = new QRadioButton("Radio button 1");
    radio1->setChecked(true);
    radio2 = new QRadioButton("Radio button 2");
    radio3 = new QRadioButton("Radio button 3");
    triCheck = new QCheckBox("Tri-state check box");
    buttonsRightLayout->addWidget(radio1);
    buttonsRightLayout->addWidget(radio2);
    buttonsRightLayout->addWidget(radio3);
    buttonsRightLayout->addWidget(triCheck);

    buttonsLayout->addLayout(buttonsLeftLayout);
    buttonsLayout->addLayout(buttonsRightLayout);
    leftColumn->addWidget(buttonsGroup);

    // --- 简单输入组件 ---
    QGroupBox *simpleInputGroup = new QGroupBox("Simple Input Widgets");
    QVBoxLayout *simpleInputLayout = new QVBoxLayout(simpleInputGroup);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    password = new QLineEdit();
    password->setPlaceholderText("Enter password");
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(password);
    simpleInputLayout->addLayout(passwordLayout);

    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    QLabel *spinboxLabel = new QLabel("SpinBox");
    spinboxLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    spinbox = new QSpinBox();
    spinbox->setRange(0, 100);
    spinbox->setValue(50);
    spinboxLayout->addWidget(spinboxLabel);
    spinboxLayout->addWidget(spinbox);
    simpleInputLayout->addLayout(spinboxLayout);

    QHBoxLayout *datetimeLayout = new QHBoxLayout();
    QLabel *datetimeLabel = new QLabel("DateTime");
    datetimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    datetime = new QDateTimeEdit();
    datetime->setDisplayFormat("yyyy/M/d HH:mm");
    datetime->setDateTime(QDateTime::currentDateTime());
    datetime->setCalendarPopup(true);
    datetimeLayout->addWidget(datetimeLabel);
    datetimeLayout->addWidget(datetime);
    simpleInputLayout->addLayout(datetimeLayout);

    QHBoxLayout *sliderDialLayout = new QHBoxLayout();
    slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(50);
    dial = new QDial();
    dial->setRange(0, 100);
    dial->setValue(50);
    dial->setFixedSize(50, 50);
    sliderDialLayout->addWidget(slider);
    sliderDialLayout->addWidget(dial);
    simpleInputLayout->addLayout(sliderDialLayout);

    leftColumn->addWidget(simpleInputGroup);

    // --- 选项卡视图 ---
    QGroupBox *tabViewGroup = new QGroupBox("Tab Views");
    QVBoxLayout *tabViewLayout = new QVBoxLayout(tabViewGroup);
    tabView = new QTabBar();
    tabView->addTab("Tree View");
    tabView->addTab("Table");
    tabView->addTab("List");
    tabView->addTab("Icon Mode List");
    listView = new QListWidget();
    listView->setFixedHeight(120);
    tabViewLayout->addWidget(tabView);
    tabViewLayout->addWidget(listView);
    leftColumn->addWidget(tabViewGroup);

    mainLayout->addLayout(leftColumn);

    // ========== 右侧列 ==========
    QVBoxLayout *rightColumn = new QVBoxLayout();

    // --- 更多输入组件 ---
    QGroupBox *moreInputGroup = new QGroupBox("More Input Widgets");
    QVBoxLayout *moreInputLayout = new QVBoxLayout(moreInputGroup);

    QHBoxLayout *styleLayout = new QHBoxLayout();
    QLabel *styleLabel = new QLabel("Style");
    styleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    styleCombo = new QComboBox();
    styleCombo->addItem("windowsvista", "vista");
    styleCombo->addItem("Windows", "windows");
    styleCombo->addItem("Fusion", "fusion");
    styleLayout->addWidget(styleLabel);
    styleLayout->addWidget(styleCombo);
    moreInputLayout->addLayout(styleLayout);

    QHBoxLayout *doubleLayout = new QHBoxLayout();
    QLabel *doubleLabel = new QLabel("Double");
    doubleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    doubleSpinbox = new QDoubleSpinBox();
    doubleSpinbox->setRange(0, 100);
    doubleSpinbox->setValue(3.14);
    doubleSpinbox->setDecimals(2);
    doubleLayout->addWidget(doubleLabel);
    doubleLayout->addWidget(doubleSpinbox);
    moreInputLayout->addLayout(doubleLayout);

    QHBoxLayout *validatedLayout = new QHBoxLayout();
    QLabel *validatedLabel = new QLabel("Validated");
    validatedLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    validatedEdit = new QLineEdit();
    validatedLayout->addWidget(validatedLabel);
    validatedLayout->addWidget(validatedEdit);
    moreInputLayout->addLayout(validatedLayout);

    rightColumn->addWidget(moreInputGroup);

    // --- 文本编辑器 ---
    QGroupBox *textEditGroup = new QGroupBox("Text Edit");
    QVBoxLayout *textEditLayout = new QVBoxLayout(textEditGroup);
    richTextEdit = new QTextEdit();
    richTextEdit->setFixedHeight(100);
    richTextEdit->setHtml("<i>Twinkle, twinkle, little star,</i><br/>How I wonder what you are.<br/>Up above the world so high,<br/>Like a diamond in the sky.<br/><i>Twinkle, twinkle, little star,</i><br/><b>How I wonder what you are!</b>");
    textEditLayout->addWidget(richTextEdit);
    rightColumn->addWidget(textEditGroup);

    // --- 纯文本编辑器 ---
    QGroupBox *plainTextGroup = new QGroupBox("Plain Text Edit");
    QVBoxLayout *plainTextLayout = new QVBoxLayout(plainTextGroup);
    plainTextEdit = new QPlainTextEdit();
    plainTextEdit->setFixedHeight(60);
    plainTextEdit->setPlaceholderText("Enter plain text here...");
    plainTextLayout->addWidget(plainTextEdit);
    rightColumn->addWidget(plainTextGroup);

    // --- 进度条 ---
    progress = new QProgressBar();
    progress->setRange(0, 100);
    progress->setValue(22);
    rightColumn->addWidget(progress);

    progressLabel = new QLabel("22%");
    progressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightColumn->addWidget(progressLabel);

    // --- 底部按钮 ---
    QHBoxLayout *bottomButtonsLayout = new QHBoxLayout();
    bottomButtonsLayout->addStretch();
    btnClose = new QPushButton("Close");
    btnClose->setFixedWidth(80);
    btnHelp = new QPushButton("Help");
    btnHelp->setFixedWidth(80);
    bottomButtonsLayout->addWidget(btnClose);
    bottomButtonsLayout->addWidget(btnHelp);
    rightColumn->addLayout(bottomButtonsLayout);

    mainLayout->addLayout(rightColumn);
}
