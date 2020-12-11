#ifndef HEKATEINI_H
#define HEKATEINI_H

#include <QWidget>
#include <QVariant>
#include <QTextStream>

struct ini_entry_t {
    QString name;
    QVariant value;
};

struct ini_cfg_enty_t {
    QString name;
    QVector<ini_entry_t> entries;
};

using H_Config  = ini_cfg_enty_t;
using H_Configs = QVector<ini_cfg_enty_t>;
using H_Entry = ini_entry_t;
using H_Entries = QVector<ini_entry_t>;

class HConfig
{
public:
    HConfig(const QString &cfg_name);

    QVariant getValue(const QString &entry_name);
    bool setValue(const QString &entry_name, const QVariant &entry_value);
    bool exists(const QString &entry_name);
    QString name(){ return m_name; }
    bool addEntry(const QString &entry_name, const QVariant &entry_value);
    H_Entry* findEntry(const QString &entry_name);
    H_Entries entries() { return m_entries; }
    int size() { return m_entries.size(); }

private:
    QString m_name;
    H_Entries m_entries;

    H_Entry* getEntry(const QString &entry_name);
};

class HConfigs
{
public:
    HConfigs() {}
    ~HConfigs();
    bool addConfig(const QString &cfg_name);
    bool addConfig(HConfig *cfg);

    HConfig* mainConfig() { return m_main_config; }
    HConfig* getConfig(const QString &cfg_name);
    HConfig* getConfigById(const QString &id);
    QVector<HConfig*> data() { return m_configs; }
    void clear() { m_configs.clear(); }
    int size() { return m_configs.size(); }
    HConfig* last() { return !m_configs.size() ? nullptr : m_configs[m_configs.size()-1]; }

private:
    HConfig *m_main_config = new HConfig("config");
    QVector<HConfig*> m_configs;
};


class HekateIni
{
public:
    HekateIni(QByteArray hekate_ini);

    HConfigs* configs() { return &m_configs; }
    HConfig* config(const QString &cfg) { return m_configs.getConfig(cfg); }
    QByteArray data() { return m_hekate_ini; }

    bool setConfigsIds();
    bool rewriteIniData();

private:
    QByteArray m_hekate_ini;
    HConfigs m_configs;

    bool parseIni(HConfigs *configs = nullptr);

};


#endif // HEKATEINI_H
