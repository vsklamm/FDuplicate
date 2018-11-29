#ifndef MODELDIR_H
#define MODELDIR_H

#include "foundfile.h"

#include <QAbstractItemModel>
#include <QObject>

class ModelDir : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ModelDir(QObject *parent = nullptr)
        : QAbstractItemModel(parent)
    { }

    ~ModelDir()
    { }

public:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    friend QVector<int> childIds(int parent_id, const ModelDir &model_dir);
    friend const FoundFile& recordById(int id, const ModelDir &model_dir);

protected:
    QModelIndex indexById(int id) const;

public:
    QVector<FoundFile> found_files;
};

#endif // MODELDIR_H
