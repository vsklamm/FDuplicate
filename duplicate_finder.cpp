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
        for (byte c : x) // TODO: or char
            seed ^= c;
        return seed;
    }
};
} // namespace std

duplicate_finder::duplicate_finder()
    : was_canceled(false){};

duplicate_finder::~duplicate_finder(){};

void duplicate_finder::clearData()
{
    duplicate_by_size.clear();
    visited_directories.clear();
    was_canceled = false;
}

void duplicate_finder::process_drive(const std::set<QString> &start_dirs, bool recursively)
{
    clearData();

    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();

    try
    {
        for (auto& current_path : start_dirs) // TODO: changed order :(
        {
            if (was_canceled)
            {
                return; // TODO: or not?
            }

            QDir current_dir(current_path);
            visited_directories.insert(current_dir.path()); // TODO: hmmmm

            QDirIterator it(current_dir.path(), QDir::Hidden | QDir::Files | QDir::NoDotAndDotDot,
                            recursively ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

            while (it.hasNext())
            {
                if (was_canceled)
                    break;

                auto file = it.next();
                if (!it.fileInfo().isSymLink())
                { // TODO: what about them?
                    auto size = it.fileInfo().size();
                    if (size < minsize)
                        continue;
                    duplicate_by_size.emplace(size, extended_file_info(it.fileInfo().fileName(), it.fileInfo().absolutePath(), size));
                }
            }
        }

        for (auto it = duplicate_by_size.begin(); it != duplicate_by_size.end();)
        {
            if (duplicate_by_size.count(it->first) < 2)
                duplicate_by_size.erase(it++);
            else
                ++it;
        }

        fsize_t lastsize = 0;
        int dupes = 0, total_id = 1;
        int group = 0;
        same_size_map same_size;
        for (auto &entry : duplicate_by_size)
        {
            if (entry.second.size == lastsize)
            {
                auto equals = same_size.equal_range(entry.second.initial_hash());
                for (auto it_equals = equals.first; it_equals != equals.second; ++it_equals)
                {
                    extended_file_info &other = it_equals->second;
                    if (other.full_hash() == entry.second.full_hash())
                    {
                        if (other.parent_id == 0) // initial value
                            other.parent_id = ++group;
                        entry.second.parent_id = other.parent_id;
                        ++dupes;
                    }
                }
            }
            else
            {
                add_to_tree(dupes, same_size, false);
                same_size.clear();
                lastsize = entry.second.size;
            }
            entry.second.id = total_id++;
            same_size.emplace(entry.second.initial_hash(), entry.second);
        }
        add_to_tree(dupes, same_size, true);
        emit scanning_finished();
    }
    catch (...)
    {
        return;
    }
}

void duplicate_finder::cancel_scanning()
{
    if (!was_canceled) // TODO: rewrite cancelling
    {
        was_canceled = true;
        emit scanning_canceled();
    }
}

void duplicate_finder::add_to_tree(int dupes, same_size_map &same_size, bool is_end)
{
    for (auto &e : same_size)
    {
        if (e.second.parent_id != 0) // files without diplicate
            dup_buffer.push_back(e.second);
    }
    if (is_end || dup_buffer.size() > max_dup_buffer)
    {
        emit tree_changed(dupes, dup_buffer);
        // dup_buffer.clear(); // TODO: need?
    }
}
