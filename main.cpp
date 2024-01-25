#include "clientwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //ClientWindow w;
    auto w = ClientWindow::createClient();
    if (w) {
        w->addClient(w);
        w->show();
    }
    else return 0;

    return a.exec();
}
