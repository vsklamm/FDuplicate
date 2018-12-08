#include "modeldir.h"

QVector<int> childIds(int parent_id, const ModelDir &model_dir)
{
    QVector<int> ret;
    for (auto &r : model_dir.found_files)
        if (r.parent_id == parent_id)
            ret.push_back(r.id);
    return ret;
}

const extended_file_info &recordById(int id, const ModelDir &model_dir)
{
    for (auto &r : model_dir.found_files)
        if (r.id == id)
            return r;
}

ModelDir::ModelDir(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ModelDir::~ModelDir()
{
}

QModelIndex ModelDir::index(int row, int column, const QModelIndex &parent) const
{
    const int parent_id = parent.isValid() ? int(parent.internalId()) : 0;
    return createIndex(row, column, quintptr(childIds(parent_id, *this).at(row)));
}

QModelIndex ModelDir::parent(const QModelIndex &child) const
{
    const int id = int(child.internalId());
    const int parent_id = recordById(id, *this).parent_id;
    return indexById(parent_id);
}

int ModelDir::rowCount(const QModelIndex &parent) const
{
    const int parent_id = parent.isValid() ? int(parent.internalId()) : 0;
    return childIds(parent_id, *this).size();
}

int ModelDir::columnCount([[maybe_unused]] const QModelIndex &parent) const
{
    return 3;
}

QVariant ModelDir::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        auto &r = recordById(int(index.internalId()), *this);
        switch (index.column())
        {
        case 0:
            return r.file_name;
        case 1:
            return r.path;
        case 2:
            return static_cast<long long>(r.size);
        }
    }
    return QVariant();
}

QModelIndex ModelDir::indexById(int id) const
{
    if (id == 0)
        return QModelIndex();
    const int parent_id = recordById(id, *this).parent_id;
    const int row = childIds(parent_id, *this).indexOf(id);
    return index(row, 0, indexById(parent_id));
}
