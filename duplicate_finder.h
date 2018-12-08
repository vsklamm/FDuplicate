#ifndef DUBLICATE_FINDER_H
#define DUBLICATE_FINDER_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

#include <map>
#include <set>
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>
#include <algorithm>

#include "crypto++/sha.h"
#include "crypto++/filters.h"
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
  std::multimap<fsize_t, extended_file_info> duplicate_by_size;
  QSet<QString> visited_directories;
  QVector<extended_file_info> dup_buffer;
  bool was_canceled;

public:
  duplicate_finder();
  ~duplicate_finder();

public slots:
  void process_drive(std::set<QString> sDir, bool recursively = true);
  void cancel_scanning();

signals:
  void tree_changed(int dupes, QVector<extended_file_info> &new_duplicates);
  void scanning_finished();
  void scanning_canceled();

private:
  void add_to_tree(int dupes, same_size_map &same_size, bool is_end);
  void clearData();
};

#endif // DUBLICATE_FINDER_H
