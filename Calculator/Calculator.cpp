#include "Calculator.h"
#include "ui_Calculator.h"
#include "curator.h"
#include <QtWidgets>
#include <QtMath>

Calculator::Calculator(QWidget *parent)
	: QMainWindow(parent),
	ui(new Ui::CalculatorClass),
	_curator(std::make_unique<Curator>(this))
{
	ui->setupUi(this);

	connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {

		enum { ENTER_TAB, CALCULATE_TAB, RESULT_TAB };

		switch (index) {
		case ENTER_TAB:
			break;
		case CALCULATE_TAB:
			updateCalcTab();
			break;
		case RESULT_TAB:
			updateResultTab();
			break;
		default:
			throw;
		}
	});

	//Enter tab connects
	connect(ui->pushButtonAddFactor, &QPushButton::clicked, this, [this] {
		QDialog questionDialog;
		QVBoxLayout layout(&questionDialog);
		QLineEdit edit;
		layout.addWidget(&edit);
		QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(&buttons, SIGNAL(accepted()), &questionDialog, SLOT(accept()));
		connect(&buttons, SIGNAL(rejected()), &questionDialog, SLOT(reject()));
		layout.addWidget(&buttons);

		auto result = questionDialog.exec();
		if (result == QDialog::Accepted && edit.text() != "")
		{
			_curator->AddFactor(edit.text());
			ui->listWidgetFactors->addItem(edit.text());

			int indexOfLast = ui->listWidgetFactors->count();
			for (int row = 0; row < ui->listWidgetProviders->count(); row++)
			{
				QListWidgetItem *item = ui->listWidgetProviders->item(row);
				
				_curator->AddProvider(item->text(), indexOfLast - 1);
			}
		}
	});
	connect(ui->pushButtonAddProvider, &QPushButton::clicked, this, [this] {
		QDialog questionDialog;
		QVBoxLayout layout(&questionDialog);
		QLineEdit edit;
		layout.addWidget(&edit);
		QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect(&buttons, SIGNAL(accepted()), &questionDialog, SLOT(accept()));
		connect(&buttons, SIGNAL(rejected()), &questionDialog, SLOT(reject()));
		layout.addWidget(&buttons);

		auto result = questionDialog.exec();
		if (result == QDialog::Accepted && edit.text() != "")
		{
			_curator->AddProvider(edit.text());
			ui->listWidgetProviders->addItem(edit.text());
		}
	});
	connect(ui->pushButtonRemoveFactor, &QPushButton::clicked, this, [this] {
		int row = ui->listWidgetFactors->currentRow();
		if (row >= 0)
		{
			_curator->RemoveFactor(row);
			auto item = ui->listWidgetFactors->takeItem(row);
			if (item)
				delete item;
		}
	});
	connect(ui->pushButtonRemoveProvider, &QPushButton::clicked, this, [this] {
		int row = ui->listWidgetProviders->currentRow();
		if (row >= 0)
		{
			_curator->RemoveProvider(row);
			auto item = ui->listWidgetProviders->takeItem(row);
			if (item)
				delete item;
		}
	});
}
Calculator::~Calculator()
{
	delete ui;
}

void Calculator::updateCalcTab()
{
	qDeleteAll(ui->scrollAreaTablesWidget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));

	//Factors Table
	auto factorModel = _curator->GetFactorTable();
	if (factorModel->rowCount() <= 1)
		return;

	auto tables = AddTable(factorModel, "Factors");
	calculateResult(factorModel, tables.second);

	connect(factorModel, &QStandardItemModel::itemChanged, this, [this, factorModel, tables](QStandardItem* item){	
		QSignalBlocker b{ factorModel };

		if (item->text().toDouble())
		{
			double reverseValue = 1 / item->text().toDouble();
			factorModel->setData(factorModel->index(item->column(), item->row()), reverseValue, Qt::DisplayRole);

			calculateResult(factorModel, tables.second);
		}
		else
		{
			item->setText(factorModel->item(item->column(), item->row())->text());
		}
	});

	//Providers Tables
	auto providersModels = _curator->GetProvidersTables();
	for (const auto& providerTable : *providersModels)
	{
		auto pModelPointer = providerTable.second;
		if (pModelPointer->rowCount() <= 1)
			return;

		auto resultTables = AddTable(pModelPointer.get(), providerTable.first);
		calculateResult(pModelPointer.get(), resultTables.second);

		connect(pModelPointer.get(), &QStandardItemModel::itemChanged, this, [this, pModelPointer, resultTables](QStandardItem* item) {
			QSignalBlocker b{ pModelPointer.get() };

			if (item->text().toDouble())
			{
				double reverseValue = 1 / item->text().toDouble();
				pModelPointer->setData(pModelPointer->index(item->column(), item->row()), reverseValue, Qt::DisplayRole);

				calculateResult(pModelPointer.get(), resultTables.second);
			}
			else
			{
				item->setText(pModelPointer->item(item->column(), item->row())->text());
			}
		});
	}
}

void Calculator::updateResultTab()
{
	//Factor header populate
	auto factorModel = _curator->GetFactorTable();
	if (!factorModel || factorModel->rowCount() <= 1)
		return;

	ui->tableWidgetFinal->setRowCount(factorModel->columnCount());
	for (size_t i = 0; i < factorModel->columnCount(); i++)
	{
		QString headerItemText = factorModel->horizontalHeaderItem(i)->text();
		ui->tableWidgetFinal->setVerticalHeaderItem(i, new QTableWidgetItem(headerItemText));
	}

	//Providers header populate
	if (_curator->GetProvidersTables()->size() <= 0)
		return;
	auto providerModel = _curator->GetProvidersTables()->front().second;
	if (!providerModel || providerModel->rowCount() <= 1)
		return;

	ui->tableWidgetFinal->setColumnCount(providerModel->columnCount() + 1);
	ui->tableWidgetFinal->setHorizontalHeaderItem(0, new QTableWidgetItem("Weit"));
	ui->tableWidgetBest->setColumnCount(providerModel->columnCount());
	ui->tableWidgetBest->setRowCount(1);

	for (size_t i = 0; i < providerModel->columnCount(); i++)
	{
		QString headerItemText = providerModel->horizontalHeaderItem(i)->text();
		ui->tableWidgetFinal->setHorizontalHeaderItem(i+1, new QTableWidgetItem(headerItemText));

		ui->tableWidgetBest->setHorizontalHeaderItem(i, new QTableWidgetItem(headerItemText));
	}
	

	//Main Populate
	auto factorsWeits = calculateResult(factorModel);
	for (size_t i = 0; i < ui->tableWidgetFinal->rowCount(); i++)
	{
		ui->tableWidgetFinal->setItem(i, 0, new QTableWidgetItem(QString::number(factorsWeits.at(i))));
	}

	auto providersModels = _curator->GetProvidersTables();
	for (size_t i = 0; i < providersModels->size(); i++)
	{
		const auto& providerModel = providersModels->at(i).second;

		auto providerWeits = calculateResult(providerModel.get());

		for (size_t j = 1; j < ui->tableWidgetFinal->columnCount(); j++)
		{
			ui->tableWidgetFinal->setItem(i, j, new QTableWidgetItem(QString::number(providerWeits.at(j - 1))));
		}
	}

	//CalculateBestFactor
	{
		std::pair<int, double> bestIndex; //best Index/best value
		for (size_t i = 0; i < ui->tableWidgetFinal->rowCount(); i++)
		{
			double factorValue = ui->tableWidgetFinal->item(i, 0)->text().toDouble();
			if (factorValue >= bestIndex.second)
			{
				bestIndex.first = i;
				bestIndex.second = factorValue;
				ui->tableWidgetFinal->item(0, i)->setBackgroundColor(Qt::white);
			}

			ui->tableWidgetFinal->item(0, bestIndex.first)->setBackgroundColor(Qt::red);
		}
	}

	//CalculateBestProvider
	{
		std::pair<int, double> bestIndex; //best Index/best value
		for (size_t i = 1; i < ui->tableWidgetFinal->columnCount(); i++)
		{
			double rating(0.0);
			for (size_t j = 0; j < ui->tableWidgetFinal->rowCount(); j++)
			{
				rating += ui->tableWidgetFinal->item(j, i)->text().toDouble() * ui->tableWidgetFinal->item(j, 0)->text().toDouble();
			}

			ui->tableWidgetBest->setItem(0, i - 1, new QTableWidgetItem(QString::number(rating)));
			if (rating >= bestIndex.second)
			{
				bestIndex.first = i - 1;
				bestIndex.second = rating;
			}

			ui->tableWidgetBest->item(0, i - 1)->setBackgroundColor(Qt::white);
		}
		ui->tableWidgetBest->item(0, bestIndex.first)->setBackgroundColor(Qt::red);
	}

}

void Calculator::calculateResult(QStandardItemModel * model, QTableWidget * resultTable)
{
	//Calc Multiplication
	std::vector<double> resultMultip;
	double resultPowMain(0);
	for (size_t i = 0; i < model->rowCount(); i++)
	{	
		double rowMultipResult( 1.0 );
		for (size_t j = 0; j < model->columnCount(); j++)
			rowMultipResult *= model->item(i, j)->text().toDouble();
		
		rowMultipResult = qPow(rowMultipResult, 1.0 / model->rowCount());
		resultPowMain += rowMultipResult;
		resultMultip.push_back(rowMultipResult);
	}

	std::pair<int, double> bestIndex; //best Index/best value
	for (size_t i = 0; i < model->rowCount(); i++)
	{
		double finalWeit = resultMultip.at(i) / resultPowMain;
		if (finalWeit >= bestIndex.second)
		{
			bestIndex.first = i;
			bestIndex.second = finalWeit;
		}

		resultTable->item(0, i)->setText(QString::number(finalWeit));
		resultTable->item(0, i)->setBackgroundColor(Qt::white);
	}

	resultTable->item(0, bestIndex.first)->setBackgroundColor(Qt::red);
}

std::vector<double> Calculator::calculateResult(QStandardItemModel * model)
{
	//Calc Multiplication
	std::vector<double> resultMultip;
	double resultPowMain(0);
	for (size_t i = 0; i < model->rowCount(); i++)
	{
		double rowMultipResult(1.0);
		for (size_t j = 0; j < model->columnCount(); j++)
			rowMultipResult *= model->item(i, j)->text().toDouble();

		rowMultipResult = qPow(rowMultipResult, 1.0 / model->rowCount());
		resultPowMain += rowMultipResult;
		resultMultip.push_back(rowMultipResult);
	}

	std::vector<double> result;
	std::pair<int, double> bestIndex; //best Index/best value
	for (size_t i = 0; i < model->rowCount(); i++)
	{
		double finalWeit = resultMultip.at(i) / resultPowMain;
		result.push_back(finalWeit);
	}

	return result;
}

void Calculator::initCurator()
{
}

std::pair<QTableView*, QTableWidget*> Calculator::AddTable(QStandardItemModel * model, QString title)
{
	QLabel* label = new QLabel(ui->scrollAreaTablesWidget); label->setText(title);
	QTableView* table = new QTableView(ui->scrollAreaTablesWidget);
	table->horizontalHeader()->setStretchLastSection(true);
	table->setMinimumHeight(200);
	table->setModel(model);

	QTableWidget* resultTable = new QTableWidget(1, model->columnCount(), ui->scrollAreaTablesWidget);
	resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	resultTable->horizontalHeader()->setStretchLastSection(true);
	for (size_t i = 0; i < model->columnCount(); i++)
	{
		auto caption = model->verticalHeaderItem(i)->text();
		resultTable->setHorizontalHeaderItem(i, new QTableWidgetItem(caption));
		resultTable->setItem(0, i, new QTableWidgetItem("0"));
	}	

	QFrame* frame = new QFrame(ui->scrollAreaTablesWidget); frame->setFrameShape(QFrame::HLine);

	ui->verticalLayoutTables->addWidget(label);
	ui->verticalLayoutTables->addWidget(table);
	ui->verticalLayoutTables->addWidget(resultTable);
	ui->verticalLayoutTables->addWidget(frame);
	
	return  std::make_pair(table, resultTable);
}