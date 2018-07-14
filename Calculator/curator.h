#ifndef CURATOR_H
#define CURATOR_H

#include <QObject>
#include <QStandardItemModel>
#include <memory>

class Curator : public QObject
{
    Q_OBJECT
public:
    explicit Curator(QObject *parent = 0);
    ~Curator() = default;

    void AddFactor(QString name);
    void RemoveFactor(int index);

    void AddProvider(QString name);
	void AddProvider(QString name, int tableNum);
    void RemoveProvider(int index);

	QStandardItemModel* GetFactorTable() { return _factorTable.get(); };
	std::vector<std::pair<QString, std::shared_ptr<QStandardItemModel> > >* GetProvidersTables() { return &_providersTables; };

private:
	std::unique_ptr<QStandardItemModel> _factorTable;
	std::vector<std::pair<QString,std::shared_ptr<QStandardItemModel> > > _providersTables;
};

#endif // CURATOR_H
