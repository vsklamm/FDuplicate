#ifndef DUBLICATE_FINDER_H
#define DUBLICATE_FINDER_H

#include <QString>
#include <map>
#include <set>
#include <vector>
#include <array>
#include <QObject>
#include <QSet>

struct dublicate_finder : QObject
{
    Q_OBJECT

public:
    using fsize_t = int64_t;

    const fsize_t minsize = 1; // ignore files smaller than x bytes

private:
    // std::vector<char *> start_directories; // vector ?

    std::multimap<fsize_t, QString> duplicate_by_size;
    QSet<QString> visited_directories;
    const QString start_directory;
    bool recursively;
    bool wasCanceled;

public:
    dublicate_finder();
    dublicate_finder(bool recursively);
    ~dublicate_finder();

public slots:
    bool process_drive(const QString &drive); // wchar_t *
    bool process_drive(const std::set<QString> &sDir);
    void cancelScanning();

signals:
    // void setProgress(int progress);
    void scanningCompleted();
    void scanningCanceled();

private:
    void compare_files(QString &first, QString &second);
    fsize_t get_file_size(const QString &file);
    void find_next_file();
    void clearData();
};

#endif // DUBLICATE_FINDER_H
