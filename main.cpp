#include "sphereui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Nullcorp");

    SphereUI w;
    w.show();

    return a.exec();
}
