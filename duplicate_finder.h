#ifndef DUBLICATE_FINDER_H
#define DUBLICATE_FINDER_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

#include <set>
#include <vector>
#include <unordered_map>
#include <atomic>

#include "crypto++/sha.h"
#include "crypto++/hex.h"

#include "extended_file_info.h"

struct duplicate_finder : QObject
{
    Q_OBJECT

public:
    using same_size_map = std::unordered_multimap<digest, extended_file_info>;

    const fsize_t minsize = 1;
    const int max_dup_buffer = 100;

private:
    std::multimap<fsize_t, extended_file_info> duplicate_by_size_;
    QVector<QVector<extended_file_info>> qu_table_;
    QSet<QString> visited_directories_;
    std::atomic<bool> was_canceled;
public:
    duplicate_finder();
    ~duplicate_finder();

public slots:
    void processDrive(const std::set<QString> &start_dirs, bool recursively = true);
    void cancelScanning();

signals:
    void preprocessFinished(int files_count); // TODO: int or ulong
    void treeChanged(int completed_files, QVector<QVector<extended_file_info>> new_duplicates);
    void scanningFinished(int dupes);

private:
    void addToTree(int completed_files, QVector<QVector<extended_file_info>> &table, int dups, bool is_end); // TODO: int or ulong
    void clearData();
};

#endif // DUBLICATE_FINDER_H
