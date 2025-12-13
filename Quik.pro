QT += core gui widgets xml

TEMPLATE = lib
CONFIG += staticlib c++11

TARGET = Quik

# Include paths
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/src

# Headers
HEADERS += \
    include/Quik/QuikAPI.h \
    include/Quik/Quik.h \
    src/core/QuikContext.h \
    src/core/QuikViewModel.h \
    src/parser/ExpressionParser.h \
    src/parser/XMLUIBuilder.h \
    src/widget/WidgetFactory.h

# Sources
SOURCES += \
    src/core/QuikContext.cpp \
    src/core/QuikViewModel.cpp \
    src/parser/ExpressionParser.cpp \
    src/parser/XMLUIBuilder.cpp \
    src/widget/WidgetFactory.cpp
