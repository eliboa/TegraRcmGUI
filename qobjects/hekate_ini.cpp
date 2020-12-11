#include "hekate_ini.h"

HekateIni::HekateIni(QByteArray hekate_ini)
{
    if (!hekate_ini.size())
        return;

    m_hekate_ini = hekate_ini;
    parseIni();
}

bool HekateIni::parseIni(HConfigs *in_configs)
{
    QTextStream ts(m_hekate_ini);
    QString line;
    HConfig *cur_cfg = nullptr;
    HConfigs *configs = in_configs != nullptr ? in_configs : &m_configs;

    configs->clear();

    while (ts.readLineInto(&line)) if (line.size())
    {
        if (line.at(0) == "[" )
        {
            auto cfg_name = QString(line.begin()+1, line.lastIndexOf("]")-1);
            if (cfg_name != "config")
            {
                configs->addConfig(cfg_name);
                cur_cfg = configs->last();
            }
            else cur_cfg = configs->mainConfig();

        }
        else if (line.at(0) != "{" && line.contains("=") && cur_cfg)
        {
            int pos = line.indexOf("=");
            cur_cfg->addEntry(line.left(pos), QVariant(line.right(line.size() - pos - 1)));
        }
    }

    return configs->size();
}

bool HekateIni::setConfigsIds()
{
    if (!m_configs.size())
        return false;

    bool result = false;
    for (auto cfg : m_configs.data()) if (!cfg->exists("id"))
    {
        // Create new id
        QString id_str;
        QString cf_name_tr = cfg->name().trimmed().replace(" ", "_");
        for (int i(0); i < 10; i++) if (!m_configs.getConfigById(QString(cf_name_tr.begin(), 6).append(QString::number(i))))
        {
            id_str = QString(cf_name_tr.begin(), 6).append(QString::number(i));
            break;
        }

        if (id_str.size())
        {
            cfg->setValue("id", id_str);
            result = true;
        }
    }

    if (result)
        rewriteIniData();

    return result;
}

bool HekateIni::rewriteIniData()
{
    QString out_str, line;
    QTextStream ts(m_hekate_ini), out_ts(&out_str);
    HConfig *cur_cfg = nullptr, *new_cfg = nullptr;
    bool out_appened = false, first = true;
    HConfigs old_cfgs, new_cfgs;

    // Look for new configs by inspecting old configs
    if (m_hekate_ini.size() && parseIni(&old_cfgs))
    {
        // Add new entries from main config
        for (auto entry : m_configs.mainConfig()->entries()) if (!old_cfgs.mainConfig()->exists(entry.name))
            new_cfgs.mainConfig()->addEntry(entry.name, entry.value);

        // Loop inside current configs
        for (auto cur_cfg : m_configs.data())
        {
            auto old_cfg = old_cfgs.getConfig(cur_cfg->name());
            if (!old_cfg) // Current config not found in old configs
            {
                new_cfgs.addConfig(cur_cfg);
                continue;
            }

            HConfig *new_cfg = new HConfig(cur_cfg->name());
            // Add new entries (not found in old entries)
            for (auto entry : cur_cfg->entries()) if (!old_cfg->exists(entry.name))
                new_cfg->addEntry(entry.name, entry.value);

            if (new_cfg->size())
                new_cfgs.addConfig(new_cfg);
        }
    }
    else new_cfgs = m_configs; // Add all configs

    // Rewrite ini from old data
    while (ts.readLineInto(&line))
    {
        if (!line.size() && (cur_cfg || first))
        {
            out_ts << "\n";
            continue;
        }

        // Old config entry
        if (line.at(0) == "[")
        {
            first = false;
            cur_cfg = nullptr;
            new_cfg = nullptr;
            auto cfg_name = QString(line.begin()+1, line.lastIndexOf("]")-1);

            cur_cfg = cfg_name == "config" ? m_configs.mainConfig() : m_configs.getConfig(cfg_name);
            if (cur_cfg)
            {
                new_cfg = cfg_name == "config" ? new_cfgs.mainConfig() : new_cfgs.getConfig(cur_cfg->name());

                // Write config
                out_ts << line << "\n";

                // Write new entries
                if (new_cfg) for (auto entry : new_cfg->entries())
                {
                    out_ts << entry.name << "=" << entry.value.toString() << "\n";
                    out_appened = true;
                }
            }
            continue;
        }

        if (!cur_cfg || !first)
            continue; // Do not write old lines if current config was deleted

        if (line.at(0) == "{" || !line.contains("="))
        {
            out_ts << line << "\n";
            continue;
        }

        // (re)write existing entries
        int pos = line.indexOf("=");
        auto entry_name = line.left(pos);
        auto entry_old_value = QVariant(line.right(line.size() - pos - 1));
        auto entry_new_value = cur_cfg->getValue(entry_name);
        if (!entry_new_value.isNull() && !entry_old_value.isNull() && entry_old_value != entry_new_value)
        {
            out_ts << entry_name << "=" << entry_new_value.toString() << "\n";
            out_appened = true;
        }
        else
            out_ts << line << "\n";
    }

    // Append main config if absent
    if (auto main_cfg = old_cfgs.getConfig("config"))
    {
        out_ts << "\n[config]\n";
        for (auto entry : main_cfg->entries())
            out_ts << entry.name << "=" << entry.value.toString();

        out_appened = true;
    }
    // Append new configs to ini data
    for (auto cfg : new_cfgs.data()) if (!old_cfgs.getConfig(cfg->name()))
    {
        out_ts << "\n" << "[" << cfg->name() << "]\n";
        for (auto entry : cfg->entries())
            out_ts << entry.name << "=" << entry.value.toString();

        out_appened = true;
    }

    if (out_appened)
    {
        // Replace ini's data
        m_hekate_ini.clear();
        m_hekate_ini.append(out_str);
        return true;
    }

    return false;
}


HConfig::HConfig(const QString &cfg_name)
{
    m_name = cfg_name;
}

bool HConfig::addEntry(const QString &entry_name, const QVariant &entry_value)
{
    if (!entry_name.size() || entry_value.isNull())
        return false;

    H_Entry entry;
    entry.name = entry_name;
    entry.value = entry_value;
    m_entries.push_back(entry);

    return true;
}

H_Entry* HConfig::getEntry(const QString &entry_name)
{
    for (int i(0); i < m_entries.size(); i++) if (!m_entries[i].name.compare(entry_name, Qt::CaseInsensitive))
        return &m_entries[i];

    return nullptr;
}

QVariant HConfig::getValue(const QString &entry_name)
{
    for (auto entry : m_entries) if (!entry.name.compare(entry_name, Qt::CaseInsensitive))
        return entry.value;

    return QVariant();
}

bool HConfig::setValue(const QString &entry_name, const QVariant &entry_value)
{
    if (!entry_name.size() || entry_value.isNull())
        return false;

    if (auto old_entry = getEntry(entry_name))
    {
        old_entry->value = entry_value;
        return true;
    }
    else return addEntry(entry_name, entry_value);
}

bool HConfig::exists(const QString &entry_name)
{
    for (auto entry : m_entries) if (!entry.name.compare(entry_name, Qt::CaseInsensitive))
        return true;

    return false;
}

HConfigs::~HConfigs() {
    delete m_main_config;
    for (auto cfg : m_configs)
        delete cfg;
}

bool HConfigs::addConfig(const QString &cfg_name)
{
    if (!cfg_name.size() || getConfig(cfg_name))
        return false;

    m_configs.push_back(new HConfig(cfg_name));
    return true;
}

bool HConfigs::addConfig(HConfig *cfg) {

    if (getConfig(cfg->name()))
        return false;

    m_configs.push_back(cfg);
    return true;
}

HConfig* HConfigs::getConfig(const QString &cfg_name)
{
    for (int i(0); i < m_configs.size(); i++) if (!m_configs[i]->name().compare(cfg_name, Qt::CaseInsensitive))
        return m_configs[i];

    return nullptr;
}

HConfig* HConfigs::getConfigById(const QString &id)
{
    for (int i(0); i < m_configs.size(); i++) if (!m_configs[i]->getValue("id").toString().compare(id, Qt::CaseInsensitive))
        return m_configs[i];

    return nullptr;
}
