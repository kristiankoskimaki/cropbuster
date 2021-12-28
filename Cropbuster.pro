QT       += core gui widgets

QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    action.cpp \
    pic.cpp

HEADERS += \
    clickablelabel.h \
    mainwindow.h \
    pic.h

FORMS += \
    mainwindow.ui

LIBS += \
    $$PWD/bin/libopencv_core452.dll \
    $$PWD/bin/libopencv_imgcodecs452.dll \
    $$PWD/bin/libopencv_imgproc452.dll

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = crop_icon_32.ico

VERSION = 1.0
QMAKE_TARGET_PRODUCT = "Cropbuster"
QMAKE_TARGET_DESCRIPTION = "Cropbuster"
QMAKE_TARGET_COPYRIGHT = "Copyright \\251 2021 Kristian Koskim\\344ki"

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\"
DEFINES += APP_COPYRIGHT=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

#How to compile Cropbuster:
    #Cropbuster is built using the Qt 6.xx (https://www.qt.io/) cross-platform library.
    #MingW-64 is the default compiler used in Qt and was used for Cropbuster development.
    #Windows users need to register on qt.io for the Qt installer to download the dev environment needed to compile.
    #If compilation fails, click on the computer icon in lower left corner of Qt Creator and select a kit (eg. MingW-64)

    #OpenCV 4.xx (64 bit) (https://www.opencv.org/)
    #Compiling OpenCV with MingW can be hard, so download binaries from https://github.com/huihut/OpenCV-MinGW-Build
    #put OpenCV \bin folder in source folder (only libopencv_core, _imgcodecs and _imgproc dll files are needed)
    #the OpenCV \opencv2 folder (contains header files) should be placed in Cropbuster project folder

    #add path to \bin folder: Projects -> Build Environment -> Details -> Path -> c:\your_qt_projects_folder\cropbuster\bin
    #Note: Cropbuster will crash on start if the path to \bin was not set or the OpenCV DLL files are not in \bin
