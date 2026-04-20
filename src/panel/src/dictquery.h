/****************************************************************************************
** @作者：lcj
**
** @说明：
**      DictQuery是一个从QObject类继承而来的非UI类，，该类设计目的为被DictQueryWin和InputWin类所包
**      含实例化，为用户提供字典查询功能.
******************************************************************************×*********/

#ifndef DICTQUERY_H
#define DICTQUERY_H

#include <QDebug>
#include <QObject>

#include "sqlite3/sqlite3.h"

class DictQuery : public QObject
{
    Q_OBJECT
public:
    explicit DictQuery(QObject *parent = nullptr);
    ~DictQuery();

public:
    void set_cur_dict(const QString &dbFile);
    QStringList dict_query_word_list(const QString &word);
    QString dict_query_word_paraphrase(const QString &word);
    QString dict_query_word_wubi_pinyin(const QString &word);
    int dict_add_custom_word(const QString &word, const QString &explain);
    int dict_del_custom_word(const QString &word);
    QString rstrip(const QString &str);

protected:
    void init_wbpy_table();
    sqlite3 *open_dict();
    void close_dict(sqlite3 *db);

private:
    QHash<QString, QString> m_wbpyTable;
    QString m_dictFile;
};

#endif
