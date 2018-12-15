#include "duplicate_finder.h"
#include "extended_file_info.h"

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
{
};

duplicate_finder::~duplicate_finder()
{
};

void duplicate_finder::clearData()
{
    duplicate_by_size.clear();
    visited_directories.clear();
    was_canceled = false;
}

void duplicate_finder::process_drive(const std::set<QString> &start_dirs, bool recursively)
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
                emit scanning_finished(0);
                return;
            }

            QDir current_dir(current_path);
            visited_directories.insert(current_dir.path()); // TODO: hmmmm

            QDirIterator it(current_dir.path(), QDir::Hidden | QDir::Files | QDir::NoDotAndDotDot,
                            recursively ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

            while (it.hasNext())
            {
                if (was_canceled)
                {
                    scan_is_running = false;
                    emit scanning_finished(0);
                    return;
                }

                auto file = it.next();
                const auto file_info = it.fileInfo();
                if (!file_info.isSymLink()) // TODO: what about them?
                {
                    auto size = file_info.size();
                    if (size < minsize)
                        continue;
                    duplicate_by_size.emplace(size, extended_file_info(file_info.fileName(), file_info.absolutePath(), size));
                }
            }
        }

        for (auto it = duplicate_by_size.begin(); it != duplicate_by_size.end();)
        {
            if (was_canceled)
            {
                scan_is_running = false;
                emit scanning_finished(0);
                return;
            }

            if (duplicate_by_size.count(it->first) < 2)
                duplicate_by_size.erase(it++);
            else
                ++it;
        }

        emit preprocess_finished(int(duplicate_by_size.size())); // TODO: or not int

        fsize_t lastsize = 0;
        int dupes = 0, total_id = 1;
        int group = 0;
        same_size_map same_size;
        QVector<QVector<extended_file_info>> table;
        for (auto &entry : duplicate_by_size)
        {
            if (was_canceled)
            {
                scan_is_running = false;
                emit scanning_finished(dupes);
                return;
            }

            if (entry.second.size == lastsize)
            {
                auto equals = same_size.equal_range(entry.second.initial_hash());
                for (auto it_equals = equals.first; it_equals != equals.second; ++it_equals)
                {
                    if (was_canceled)
                    {
                        scan_is_running = false;
                        emit scanning_finished(dupes);
                        return;
                    }

                    extended_file_info &other = it_equals->second;
                    if (other.full_hash() == entry.second.full_hash())
                    {
                        if (other.parent_id == 0) { // initial value
                            ++dupes;
                            other.parent_id = ++group;
                            other.vector_row = entry.second.vector_row = table.size();
                            table.push_back(QVector<extended_file_info>(1, other));
                        }
                        ++dupes;
                        entry.second.parent_id = other.parent_id;
                        entry.second.vector_row = other.vector_row;
                        table[entry.second.vector_row].push_back(entry.second);
                    }
                }
            }
            else
            {
                add_to_tree(total_id, table, false);
                table.clear();
                same_size.clear();
                lastsize = entry.second.size;
            }
            entry.second.id = total_id++;
            same_size.emplace(entry.second.initial_hash(), entry.second);
        }
        add_to_tree(total_id, table, true);

        scan_is_running = false;
        emit scanning_finished(dupes);
    }
    catch (...)
    {
        scan_is_running = false;
    }
}

void duplicate_finder::remove_files(std::vector<QString> &files_to_remove)
{
    for (auto& file_name : files_to_remove) {
        if (!QFile(file_name).remove()) {
             // TODO: smth
        }
    }
}

void duplicate_finder::cancel_scanning()
{
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    was_canceled = true;
}

void duplicate_finder::add_to_tree(int completed_files, QVector<QVector<extended_file_info>> &table, bool is_end)
{
    for (auto& e : table) {
        qu_table.push_back(e);
    }
    if (qu_table.size() > 35 || is_end) {
        emit tree_changed(completed_files, qu_table);
        qu_table.clear();
    }
}
