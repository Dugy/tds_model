#include "tds_model.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TDSmodel w;
	w.show();

	return a.exec();
}
