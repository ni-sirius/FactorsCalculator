#include "Calculator.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication::addLibraryPath("pluginsFolder");
	QApplication a(argc, argv);
	Calculator w;
	w.show();
	return a.exec();
}
