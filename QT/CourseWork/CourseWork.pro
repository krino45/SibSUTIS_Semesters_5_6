#-------------------------------------------------
#
# Project created by QtCreator 2024-12-09T23:06:16
#
#-------------------------------------------------

QT       += core gui sql

QMAKE_CXXFLAGS += -DQT_MEMORY_CHECK

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CourseWork
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 debug

SOURCES += \
        addlistingdialog.cpp \
        addreviewdialog.cpp \
        adminpage.cpp \
        appcontext.cpp \
        categorymanager.cpp \
        clickablelabel.cpp \
        complaintspage.cpp \
        databasehandler.cpp \
        dialogpage.cpp \
        dialogspage.cpp \
        editlistingdialog.cpp \
        edituserdialog.cpp \
        favouritespage.cpp \
        flowlayout.cpp \
        headerbar.cpp \
        listingcard.cpp \
        logindialog.cpp \
        main.cpp \
        mainwindow.cpp \
        messagespage.cpp \
        productpage.cpp \
        registerdialog.cpp \
        reportdialog.cpp \
        searchpage.cpp \
        usercontrolpage.cpp \
        userpage.cpp

HEADERS += \
        addlistingdialog.h \
        addreviewdialog.h \
        adminpage.h \
        appcontext.h \
        categorymanager.h \
        clickablelabel.h \
        complaintspage.h \
        databasehandler.h \
        dialogpage.h \
        dialogspage.h \
        editlistingdialog.h \
        edituserdialog.h \
        favouritespage.h \
        flowlayout.h \
        headerbar.h \
        listingcard.h \
        logindialog.h \
        mainwindow.h \
        messagespage.h \
        productpage.h \
        registerdialog.h \
        reportdialog.h \
        searchpage.h \
        usercontrolpage.h \
        userpage.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES +=
