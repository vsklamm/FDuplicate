#include "duplicate_finder.h"
#include "extended_file_info.h"

#include "crypto++/filters.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <QThread>

#include <fstream>
#include <queue>

namespace std
{
template <>
struct hash<digest>
{
    size_t operator()(const digest &x) const
    {
        size_t seed = 0;
        for (byte c : x)
            seed ^= c;
        return seed;
    }
};
} // namespace std

duplicate_finder::duplicate_finder()
    : was_canceled(false)
{
};

duplicate_finder::~duplicate_finder()
{
};

void duplicate_finder::clearData()
{
    duplicate_by_size_.clear();
    visited_directories_.clear();
    was_canceled = false;
}

void duplicate_finder::processDrive(const std::set<QString> &start_dirs, bool recursively)
{
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    scan_is_running = true;

    clearData();

    try
    {
        for (auto& current_path : start_dirs)
        {
            if (was_canceled)
            {
                scan_is_running = false;
                emit scanningFinished(0);
                return;
            }

            QDir current_dir(current_path);
            visited_directories_.insert(current_dir.path()); // TODO: hmmmm

            QDirIterator it(current_dir.path(), QDir::Readable | QDir::Hidden | QDir::Files | QDir::NoDotAndDotDot, // TODO: tests for rights
                            recursively ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

            while (it.hasNext())
            {
                if (was_canceled)
                {
                    scan_is_running = false;
                    emit scanningFinished(0);
                    return;
                }

                auto file = it.next();
                const auto file_info = it.fileInfo();
                if (!file_info.isSymLink()) // TODO: what about them?
                {
                    if (QFile(file).open(QIODevice::ReadOnly)) { // TODO: too sloooow work with strings and files
                        auto size = file_info.size();
                        if (size < minsize)
                            continue;
                        duplicate_by_size_.emplace(size, extended_file_info(file_info.fileName(), file_info.absolutePath(), size));
                    }
                }
            }
        }

        for (auto it = duplicate_by_size_.begin(); it != duplicate_by_size_.end();)
        {
            if (was_canceled)
            {
                scan_is_running = false;
                emit scanningFinished(0);
                return;
            }

            if (duplicate_by_size_.count(it->first) < 2)
                duplicate_by_size_.erase(it++);
            else
                ++it;
        }

        emit preprocessFinished(int(duplicate_by_size_.size())); // TODO: or not int

        fsize_t lastsize = 0;
        int dupes = 0, prev_dups = 0;
        int group = 0, total_id = 1;
        same_size_map same_size;
        QVector<QVector<extended_file_info>> table;
        for (auto &entry : duplicate_by_size_)
        {
            if (was_canceled)
            {
                scan_is_running = false;
                emit scanningFinished(dupes);
                return;
            }

            if (entry.second.size == lastsize)
            {
                auto equals = same_size.equal_range(entry.second.initialHash());
                for (auto it_equals = equals.first; it_equals != equals.second; ++it_equals)
                {
                    if (was_canceled)
                    {
                        scan_is_running = false;
                        emit scanningFinished(dupes);
                        return;
                    }

                    extended_file_info &other = it_equals->second;
                    if (other.fullHash() == entry.second.fullHash())
                    {
                        if (other.parent_id == 0) { // initial value
                            ++dupes;
                            other.parent_id = ++group;
                            other.vector_row = table.size();
                            table.push_back(QVector<extended_file_info>(1, other));
                        }
                        ++dupes;
                        entry.second.parent_id = other.parent_id;
                        entry.second.vector_row = other.vector_row;
                        table[entry.second.vector_row].push_back(entry.second);
                        break;
                    }
                }
            }
            else
            {
                addToTree(total_id, table, (prev_dups = dupes - prev_dups), false);
                table.clear();
                same_size.clear();
                lastsize = entry.second.size;
            }
            entry.second.id = total_id++;
            same_size.emplace(entry.second.initialHash(), entry.second);
        }
        addToTree(total_id, table, dupes, true);

        scan_is_running = false;
        emit scanningFinished(dupes);
    }
    catch (...)
    {
        scan_is_running = false;
        emit scanningFinished(0);
    }
}

void duplicate_finder::cancelScanning()
{
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    was_canceled = true;
}

void duplicate_finder::addToTree(int completed_files, QVector<QVector<extended_file_info>> &table, int dups, bool is_end)
{
    for (auto& e : table) {
        qu_table_.push_back(e);
    }
    if (dups > 15 || is_end) {
        emit treeChanged(completed_files, qu_table_);
        qu_table_.clear();
    }
}
