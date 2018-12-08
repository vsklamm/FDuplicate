#ifndef EXTENDED_FILE_INFO_H
#define EXTENDED_FILE_INFO_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

#include "crypto++/sha.h"

constexpr int hashlen = CryptoPP::SHA1::DIGESTSIZE;
constexpr int64_t first_hash_size = 1 << 16;

using digest = std::array<byte, hashlen>;
using fsize_t = int64_t;

struct extended_file_info
{
  extended_file_info();
  extended_file_info(QString name, QString path, fsize_t size);
  extended_file_info(QString name, QString path, fsize_t size, int id, int parent_id);

  digest initial_hash();
  digest full_hash();

private:
  digest sha1(const QString &path, fsize_t max_len);
  QString append_path(const QString &path1, const QString &path2);

public:
  QString file_name;
  QString path;
  fsize_t size;
  int id;
  int parent_id;

private:
  digest initial_hash_;
  digest full_hash_;
  bool has_initial_hash = false;
  bool has_full_hash = false;
};

#endif // EXTENDED_FILE_INFO_H
