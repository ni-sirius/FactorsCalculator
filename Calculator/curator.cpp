#include "curator.h"

Curator::Curator(QObject *parent) : QObject(parent)
{
	_factorTable = std::make_unique<QStandardItemModel>(0,0);
}

void Curator::AddFactor(QString name)
{
	if (_factorTable->rowCount() != _factorTable->columnCount())
		return;

	auto elementCount = _factorTable->rowCount();

	_factorTable->setHorizontalHeaderItem(elementCount, new QStandardItem(name));
	_factorTable->setVerticalHeaderItem(elementCount, new QStandardItem(name));
	_factorTable->setData(_factorTable->index(elementCount, elementCount), QString("1"), Qt::DisplayRole);

	for (size_t i = 0; i < _factorTable->columnCount(); i++)
		for (size_t j = 0; j < _factorTable->rowCount(); j++)
		{
			if (!_factorTable->item(j, i))
				_factorTable->setItem(j, i, new QStandardItem("0"));
			if (j >= i)
				_factorTable->item(j, i)->setFlags(Qt::NoItemFlags);			
		}

	_providersTables.push_back(std::make_pair(name, std::make_shared<QStandardItemModel>(0, 0)));
}
void Curator::RemoveFactor(int index)
{
	if (index <= _factorTable->columnCount() || index <= _factorTable->rowCount())
	{
		_factorTable->removeColumn(index);
		_factorTable->removeRow(index);
	}

	_providersTables.erase(_providersTables.begin() + index);
}

void Curator::AddProvider(QString name)
{
	for (const auto &table : _providersTables)
	{
		if (table.second->rowCount() != table.second->columnCount())
			return;

		auto elementCount = table.second->rowCount();

		table.second->setHorizontalHeaderItem(elementCount, new QStandardItem(name));
		table.second->setVerticalHeaderItem(elementCount, new QStandardItem(name));
		table.second->setData(table.second->index(elementCount, elementCount), QString("1"), Qt::DisplayRole);

		for (size_t i = 0; i < table.second->columnCount(); i++)
			for (size_t j = 0; j < table.second->rowCount(); j++)
			{
				if (!table.second->item(j, i))
					table.second->setItem(j, i, new QStandardItem("0"));
				if (j >= i)
					table.second->item(j, i)->setFlags(Qt::NoItemFlags);
			}
	}
}

void Curator::AddProvider(QString name, int tableNum)
{
	const auto &table = _providersTables.at(tableNum);

	if (table.second->rowCount() != table.second->columnCount())
		return;

	auto elementCount = table.second->rowCount();

	table.second->setHorizontalHeaderItem(elementCount, new QStandardItem(name));
	table.second->setVerticalHeaderItem(elementCount, new QStandardItem(name));
	table.second->setData(table.second->index(elementCount, elementCount), QString("1"), Qt::DisplayRole);

	for (size_t i = 0; i < table.second->columnCount(); i++)
		for (size_t j = 0; j < table.second->rowCount(); j++)
		{
			if (!table.second->item(j, i))
				table.second->setItem(j, i, new QStandardItem("0"));
			if (j >= i)
				table.second->item(j, i)->setFlags(Qt::NoItemFlags);			
		}
}

void Curator::RemoveProvider(int index)
{
	for (const auto &table : _providersTables)
	{
		if (index <= table.second->columnCount() || index <= table.second->rowCount())
		{
			table.second->removeColumn(index);
			table.second->removeRow(index);
		}
	}
}
