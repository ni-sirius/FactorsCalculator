#pragma once

#include <QtWidgets/QMainWindow>
#include <memory>
#include "curator.h"

namespace Ui {
	class CalculatorClass;
}

class Curator;
class QTableView;
class QTableWidget;

class Calculator : public QMainWindow
{
	Q_OBJECT

public:
	explicit Calculator(QWidget *parent = Q_NULLPTR);
	~Calculator();

private:
	Ui::CalculatorClass *ui;
	std::unique_ptr<Curator> _curator;

	void updateCalcTab();
	void updateResultTab();

	void calculateResult(QStandardItemModel* model, QTableWidget* resultTable);
	std::vector<double> calculateResult(QStandardItemModel* model);

protected:
	void initCurator();

	std::pair<QTableView*, QTableWidget*> AddTable(QStandardItemModel* model, QString title);
};
