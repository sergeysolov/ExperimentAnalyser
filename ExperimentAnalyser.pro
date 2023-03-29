QT       += core gui
QT       += core gui charts
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addnewexperimentform.cpp \
    chagechartsettingsform.cpp \
    experiment_name_form.cpp \
    main.cpp \
    mainwindow.cpp \
    result_chart_form.cpp \
    savetofileform.cpp

HEADERS += \
    AxesRange.h \
    SaveSettings.h \
    addnewexperimentform.h \
    chagechartsettingsform.h \
    experiment_name_form.h \
    mainwindow.h \
    result_chart_form.h \
    savetofileform.h

FORMS += \
    addnewexperimentform.ui \
    chagechartsettingsform.ui \
    experiment_name_form.ui \
    mainwindow.ui \
    result_chart_form.ui \
    savetofileform.ui

RESOURCES += icons.qrc

TRANSLATIONS += \
    ExperimentAnalyser_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
