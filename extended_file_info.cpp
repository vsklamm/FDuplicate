#include "extended_file_info.h"

#include <QDir>
#include <QFile>

extended_file_info::extended_file_info()
    : file_name(""), path(""), size(0), id(0)
{
}

extended_file_info::extended_file_info(QString name, QString path, fsize_t size)
    : file_name(name), path(path), size(size), id(0)
{
}

digest extended_file_info::initialHash()
{
    if (!has_initial_hash)
    {
        initial_hash_ = sha256(appendPath(path, file_name), std::min(first_hash_size, size));
        has_initial_hash = true;
    }
    return initial_hash_;
}

digest extended_file_info::fullHash()
{
    if (first_hash_size >= size)
        return initialHash();
    if (!has_full_hash)
    {
        full_hash_ = sha256(appendPath(path, file_name), size);
        has_full_hash = true;
    }
    return full_hash_;
}

digest extended_file_info::sha256(const QString &path, fsize_t maxlen)
{
    digest dig;
    QFile read_file(path);
    if (read_file.exists() && read_file.open(QIODevice::ReadOnly)) {

        char file_data[8192];
        CryptoPP::SHA1 sha2;
        fsize_t read = 0;

        while (read < maxlen)
        {
            fsize_t len = read_file.read(file_data, sizeof(file_data));
            read += len;
            if (len == 0)
                break;
            sha2.Update(reinterpret_cast<byte *>(file_data), size_t(len));
        }
        read_file.close();
        sha2.Final(dig.begin());
    }
    return dig;
}

QString extended_file_info::appendPath(const QString &path1, const QString &path2)
{
    return QDir::cleanPath(path1 + QDir::separator() + path2);
}
