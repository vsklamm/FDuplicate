#include "duplicate_finder.h"

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QSet>

#include <fstream>
#include <queue>
#include <unordered_map>
#include <set>

dublicate_finder::dublicate_finder()
    : recursively(true), wasCanceled(false)
{ };

dublicate_finder::dublicate_finder(bool recursively)
    : recursively(recursively), wasCanceled(false)
{ };

dublicate_finder::~dublicate_finder()
{ };

dublicate_finder::fsize_t dublicate_finder::get_file_size(const QString &file)
{
    return QFileInfo(file).size();
}

void dublicate_finder::clearData()
{
    duplicate_by_size.clear();
    visited_directories.clear();
    wasCanceled = false;
}

bool dublicate_finder::process_drive(const QString &sDir)
{
    process_drive({sDir});
}

bool dublicate_finder::process_drive(const std::set<QString> &start_dirs)
{
    clearData();

    std::queue<QDir> order;
    for (auto& s : start_dirs) {
        order.emplace(s);
    }

    while(!wasCanceled && !order.empty())
    {
        QDir current_path = order.front();
        order.pop();

        if (!current_path.exists())
        {
            // out << DIRECTORY DOESN'T EXIST
        }

        QFileInfoList list = current_path.entryInfoList();
        for (QFileInfo& file_info : list) // TODO: what about rules?
        {
            if (wasCanceled) break;

            if (file_info.isSymLink())
                file_info = QFileInfo(file_info.symLinkTarget()); // TODO: is it legal?

            if (file_info.fileName().compare(".") != 0 && file_info.fileName().compare("..") != 0) // L"." <=> wchar_t
            {
                if (recursively && !file_info.isFile() && file_info.isDir()) // TODO: ??? !!!
                {
                    order.emplace(file_info.path()); // TODO: path or absolute path ?
                    visited_directories.insert(file_info.path());
                }
                else
                {
                    auto size = file_info.size();
                    if (size < minsize) continue;
                    duplicate_by_size.emplace(size, file_info.path());
                }
            }
        }
    }

    // size filtering
    for(auto it = duplicate_by_size.begin(); it != duplicate_by_size.end();)
    {
        if(duplicate_by_size.count(it->first) < 2) duplicate_by_size.erase(it++);
        else ++it;
    }

//    std::unordered_multimap<digest,file_info> samesizeset;
//    fsize_t i = 0, dupes = 0, lastsize = 0;
//    for(auto& entry : duplicate_by_size) {

//        if(entry.second.size() == lastsize) {
//            auto equals = samesizeset.equal_range(entry.second.firsthash());
//            for(auto it=equals.first; it!=equals.second; ++it) {
//                auto other = it->second;
//                if(compare_files(other, entry.second)) {

//                    dupes++;
//                }
//            }
//        } else {
//            samesizeset.clear();
//            lastsize = entry.second.size();
//        }
//        samesizeset.insert(make_pair(entry.second.firsthash(),entry.second));
//    }
    return true;
}

void dublicate_finder::cancelScanning()
{
    if (!wasCanceled) {
        wasCanceled = true;
        emit scanningCanceled();
    }
}
