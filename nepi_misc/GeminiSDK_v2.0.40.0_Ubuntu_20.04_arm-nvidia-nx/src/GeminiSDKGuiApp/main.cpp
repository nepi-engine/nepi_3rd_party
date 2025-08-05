#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QtGui/QPalette>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef WIN32
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    a.setStyle(QStyleFactory::create("Fusion"));    // fusion look & feel of controls
    QPalette p (QColor(43, 43, 43));
    a.setPalette(p);

    // check for
    bool transparent = a.arguments().contains(QStringLiteral("-transparent"));
    bool frameless = a.arguments().contains(QStringLiteral("-frameless"));

    MainWindow w(transparent, frameless);
    w.show();

    // set Escape key to exit (required for frameless windows)
    QObject::connect (new QShortcut(QKeySequence(Qt::Key_Escape), &w), &QShortcut::activated,
             &w, &MainWindow::close);

    return a.exec();
}
