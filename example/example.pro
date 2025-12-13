QT += core gui widgets xml

CONFIG += c++11 console

TARGET = QuikExample

# Include Quik library
INCLUDEPATH += \
    $$PWD/../include \
    $$PWD/../src

# Link Quik library (after building)
# LIBS += -L$$PWD/../build -lQuik

# For now, include sources directly
HEADERS += \
    $$PWD/../include/Quik/QuikAPI.h \
    $$PWD/../include/Quik/Quik.h \
    $$PWD/../src/core/QuikContext.h \
    $$PWD/../src/core/QuikViewModel.h \
    $$PWD/../src/parser/ExpressionParser.h \
    $$PWD/../src/parser/XMLUIBuilder.h \
    $$PWD/../src/widget/WidgetFactory.h

SOURCES += \
    main.cpp \
    $$PWD/../src/core/QuikContext.cpp \
    $$PWD/../src/core/QuikViewModel.cpp \
    $$PWD/../src/parser/ExpressionParser.cpp \
    $$PWD/../src/parser/XMLUIBuilder.cpp \
    $$PWD/../src/widget/WidgetFactory.cpp
