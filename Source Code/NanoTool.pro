QT       += core gui
QT       += charts widgets
QT       +=  core gui opengl


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += ../../../../../../CODE/Nanocode
INCLUDEPATH += C:\Users\aless\OneDrive\Desktop\QtProject\NanoTool\glm

LIBS += -lopengl32

SOURCES += \
    ../../../../../../CODE/Nanocode/Atom.cpp \
    ../../../../../../CODE/Nanocode/Nanoparticle.cpp \
    ../../../../../../CODE/Nanocode/Site.cpp \
    SharedFunctions.cpp \
    credit_dialog.cpp \
    dialog1.cpp \
    dialog2.cpp \
    dialog3.cpp \
    dialogbeau.cpp \
    dialogcope.cpp \
    dialogmix.cpp \
    main.cpp \
    mainwindow.cpp \
    qparticle.cpp \
    sitesdialog.cpp

HEADERS += \
    ../../../../../../CODE/Nanocode/Atom.h \
    ../../../../../../CODE/Nanocode/Function.h \
    ../../../../../../CODE/Nanocode/Nanoparticle.h \
    ../../../../../../CODE/Nanocode/Reaction.h \
    ../../../../../../CODE/Nanocode/Site.h \
    SharedFunctions.h \
    credit_dialog.h \
    dialog1.h \
    dialog2.h \
    dialog3.h \
    dialogbeau.h \
    dialogcope.h \
    dialogmix.h \
    mainwindow.h \
    qparticle.h \
    sitesdialog.h

FORMS += \
    credit_dialog.ui \
    dialog1.ui \
    dialog2.ui \
    dialog3.ui \
    dialogbeau.ui \
    dialogcope.ui \
    dialogmix.ui \
    mainwindow.ui \
    sitesdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
