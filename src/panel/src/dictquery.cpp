#include "dictquery.h"

#include "config.h"
#include "wbpy.cpp"

DictQuery::DictQuery(QObject *parent) : QObject(parent)
{
    m_dictFile = INSTALL_DIR + "/data/dict/freeime_chinese_dict.db";

    init_wbpy_table();
}

DictQuery::~DictQuery()
{
}

void DictQuery::init_wbpy_table()
{
    int len = sizeof(g_wbpyTable) / sizeof(g_wbpyTable[0]);

    for (int i = 0; i < len; i++)
    {
        QString line = QString(g_wbpyTable[i]);
        QStringList tmp = line.split(":");
        if (tmp.length() > 1)
        {
            m_wbpyTable.insert(tmp.at(0), tmp.at(1));
        }
    }
    // qDebug() << m_wbpyTable.size();
}

void DictQuery::set_cur_dict(const QString &dbFile)
{
    m_dictFile = dbFile;
}

sqlite3 *DictQuery::open_dict()
{
    sqlite3 *db = nullptr;
    if (sqlite3_open(m_dictFile.toUtf8().data(), &db) != SQLITE_OK)
    {
        qWarning() << sqlite3_errmsg(db);
        return nullptr;
    }
    else
    {
        return db;
    }
}

void DictQuery::close_dict(sqlite3 *db)
{
    if (db)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

QString DictQuery::rstrip(const QString &str)
{
    int n = str.size() - 1;
    for (; n >= 0; --n)
    {
        if (!str.at(n).isSpace() && !(str.at(n) >= 'a' && str.at(n) <= 'z') && str.at(n) != '[' && str.at(n) != ']')
        {
            return str.left(n + 1);
        }
    }
    return "";
}

QString DictQuery::dict_query_word_paraphrase(const QString &word)
{
    int ret;
    QString paraphrase;
    sqlite3_stmt *stmt = nullptr;

    sqlite3 *db = open_dict();
    if (db)
    {
        QString str = word;
        str = rstrip(str);
        QString sql = QString(_("select content from Library where code=\"%1\";")).arg(str);
        sqlite3_prepare(db, sql.toUtf8().data(), -1, &stmt, nullptr);
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            int size = sqlite3_column_bytes(stmt, 0);
            if (size)
            {
                const unsigned short *data = static_cast<const unsigned short *>(sqlite3_column_blob(stmt, 0));
                unsigned short *unicode = new unsigned short[size / 2 + 1];
                int i;
                for (i = 0; i < size / 2; i++)
                {
                    unicode[i] = data[i] ^ 0x18;
                }
                unicode[i] = 0;
                // print_hex( reinterpret_cast<unsigned char *>(unicode), size );
                paraphrase += QString::fromUtf16(unicode);
                delete[] unicode;
            }
        }
        sqlite3_finalize(stmt);
    }

    close_dict(db);

    return paraphrase;
}

QString DictQuery::dict_query_word_wubi_pinyin(const QString &word)
{
    QString ret;

    if (m_wbpyTable.contains(word))
    {
        ret = m_wbpyTable[word];
    }

    return ret;
}

QStringList DictQuery::dict_query_word_list(const QString &word)
{
    int ret, size;
    QStringList wordList;
    sqlite3_stmt *stmt = nullptr;

    sqlite3 *db = open_dict();
    if (db)
    {
        QString sql = QString(_("select code from Library where code like \"%1%%\";")).arg(word);

        sqlite3_prepare(db, sql.toUtf8().data(), -1, &stmt, nullptr);
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            size = sqlite3_column_bytes(stmt, 0);
            if (size)
            {
                const char *data = static_cast<const char *>(sqlite3_column_blob(stmt, 0));
                wordList << QString(data);
            }
        }
        sqlite3_finalize(stmt);
    }
    close_dict(db);

    return wordList;
}

int DictQuery::dict_add_custom_word(const QString &word, const QString &explain)
{
    int ret = -1;
    sqlite3_stmt *stmt = nullptr;
    QString sqlFind = QString(_("select content from Library where code=\"%1\";")).arg(word);
    QString sqlInsert = QString(_("insert into Library(code,content) values(?, ?);"));
    QString sqlUpdate = QString(_("update Library set content=? where code=\"%1\";")).arg(word);

    sqlite3 *db = open_dict();
    if (db)
    {
        int len = explain.length();
        unsigned short *blob = new unsigned short[len];
        for (int i = 0; i < len; i++)
        {
            blob[i] = explain.at(i).unicode() ^ 0x18;
        }

        sqlite3_prepare(db, sqlFind.toUtf8().data(), -1, &stmt, nullptr);
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            sqlite3_prepare(db, sqlUpdate.toUtf8().data(), -1, &stmt, nullptr);
            sqlite3_bind_blob(stmt, 1, static_cast<const void *>(blob), len * 2, SQLITE_TRANSIENT);
            ret = sqlite3_step(stmt);
            sqlite3_finalize(stmt);

        }
        else if (ret == SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            sqlite3_prepare(db, sqlInsert.toUtf8().data(), -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, static_cast<const char *>(word.toUtf8().data()), word.toUtf8().length(), SQLITE_TRANSIENT);
            sqlite3_bind_blob(stmt, 2, static_cast<const void *>(blob), len * 2, SQLITE_TRANSIENT);
            ret = sqlite3_step(stmt);
            sqlite3_finalize(stmt);

        }
        else
        {
            sqlite3_finalize(stmt);
        }

        delete[] blob;
    }
    close_dict(db);

    if (ret == SQLITE_DONE)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int DictQuery::dict_del_custom_word(const QString &word)
{
    int ret = -1;
    sqlite3_stmt *stmt = nullptr;

    sqlite3 *db = open_dict();
    if (db)
    {
        QString sql = QString(_("delete from Library where code=\"%1\";")).arg(word);
        sqlite3_prepare(db, sql.toUtf8().data(), -1, &stmt, nullptr);
        ret = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    close_dict(db);

    if (ret == SQLITE_DONE)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
