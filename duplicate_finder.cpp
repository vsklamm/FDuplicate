#include "duplicate_finder.h"
#include "extended_file_info.h"

#include <QDir>
#include <QFileInfo>

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
    : recursively(true), was_canceled(false){};

duplicate_finder::duplicate_finder(bool recursively)
    : recursively(recursively), was_canceled(false){};

duplicate_finder::~duplicate_finder(){};

fsize_t duplicate_finder::get_file_size(const QString &file)
{
    return QFileInfo(file).size();
}

void duplicate_finder::clearData()
{
    duplicate_by_size.clear();
    visited_directories.clear();
    was_canceled = false;
}

bool duplicate_finder::process_drive(const QString &sDir)
{
    std::set<QString> tmp;
    tmp.insert(sDir);
    return process_drive(tmp);
}

bool duplicate_finder::process_drive(const std::set<QString> &start_dirs)
{
    clearData();

    std::queue<QDir> order;
    for (auto &s : start_dirs)
    {
        order.emplace(s);
    }

    while (!was_canceled && !order.empty())
    {
        QDir current_path = order.front();
        order.pop();
        visited_directories.insert(current_path.path());

        if (!current_path.exists())
        {
            // out << DIRECTORY DOESN'T EXIST
        }

        QFileInfoList list = current_path.entryInfoList();
        for (QFileInfo &file_inf : list) // TODO: what about rules?
        {
            if (was_canceled)
                break;
            if (file_inf.isSymLink())
                continue;
            //    file_info = QFileInfo(file_info.symLinkTarget()); // TODO: is it legal?
            if (file_inf.fileName().compare(".") != 0 && file_inf.fileName().compare("..") != 0)
            {
                if (recursively && !file_inf.isFile() && file_inf.isDir())
                {
                    order.emplace(file_inf.absoluteFilePath()); // TODO: path or absolute path ?
                }
                else
                {
                    auto size = file_inf.size();
                    if (size < minsize)
                        continue;
                    duplicate_by_size.emplace(size, extended_file_info(file_inf.fileName(), file_inf.absolutePath(), size));
                }
            }
        }
    }

    for (auto it = duplicate_by_size.begin(); it != duplicate_by_size.end();)
    {
        if (duplicate_by_size.count(it->first) < 2)
            duplicate_by_size.erase(it);
        ++it;
    }

    fsize_t lastsize = 0;
    int dupes = 0, total_id = 1, parent_id = 0;
    same_size_map same_size;
    for (auto &entry : duplicate_by_size)
    {
        if (entry.second.size == lastsize)
        {
            auto equals = same_size.equal_range(entry.second.first_hash());
            for (auto it = equals.first; it != equals.second; ++it)
            {
                auto other = it->second;
                if (other.full_hash() == entry.second.full_hash())
                {
                    parent_id = other.id;
                    ++dupes;
                }
            }
        }
        else
        {
            add_to_tree(dupes, same_size, false);
            same_size.clear();
            parent_id = 0;
            lastsize = entry.second.size;
        }
        entry.second.id = total_id++;
        entry.second.parent_id = parent_id;
        same_size.emplace(entry.second.first_hash(), entry.second);
    }
    add_to_tree(dupes, same_size, true);
    return true;
}

void duplicate_finder::cancel_scanning()
{
    if (!was_canceled)
    {
        was_canceled = true;
        emit scanning_canceled();
    }
}

void duplicate_finder::add_to_tree(int dupes, same_size_map &same_size, bool is_end)
{
    for (auto &e : same_size)
    {
        dup_buffer.push_back(e.second);
    }
    if (is_end || dup_buffer.size() > max_dup_buffer)
    {
        emit tree_changed(dupes, dup_buffer);
        // dup_buffer.clear(); // TODO: need?
    }
}
