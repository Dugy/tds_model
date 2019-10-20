#ifndef TDSMODEL_H
#define TDSMODEL_H

#include <QMainWindow>
#include "computation.h"

namespace Ui {
class TDSmodel;
}

class TDSmodel : public QMainWindow
{
	Q_OBJECT

public:
	explicit TDSmodel(QWidget *parent = 0);
	~TDSmodel();

private slots:
	void on_exitButton_clicked();

	void on_simulateButton_clicked();

    void on_pushButton_clicked();

	void on_fitButton_clicked();

	void on_resetChangeRangesButton_clicked();

private:
	void readSettings();
	void plot();

	Ui::TDSmodel *ui;
	float* compared;
    float dataMultiplier;
	float getMatch(const float* pressures);
	entry given_;
};

#endif // TDSMODEL_H
