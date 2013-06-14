#ifndef SHORTCUTMODEL_H
#define SHORTCUTMODEL_H
#include <QAbstractTableModel>
#include <libkkc/libkkc.h>

class ShortcutEntry {
public:
    ShortcutEntry(const QString& command, KkcKeyEvent* event, const QString& label, KkcInputMode mode):
        m_command(command),
        m_event(KKC_KEY_EVENT(g_object_ref(event))),
        m_label(label),
        m_mode(mode)
    {
        gchar* keystr = kkc_key_event_to_string(m_event);
        m_keyString = QString::fromUtf8(keystr);
        g_free(keystr);
    }

    ShortcutEntry(const ShortcutEntry& other) :
        ShortcutEntry(other.m_command, KKC_KEY_EVENT(g_object_ref(other.m_event)), other.m_label, other.m_mode)
    {
    }

    ~ShortcutEntry() {
        g_object_unref(m_event);
    }

    const QString& label() const {
        return m_label;
    }

    const QString& command() const {
        return m_command;
    }

    KkcInputMode mode() const {
        return m_mode;
    }

    KkcKeyEvent* event() const {
        return m_event;
    }

    const QString& keyString() const {
        return m_keyString;
    }

private:
    QString m_command;
    KkcKeyEvent* m_event;
    QString m_label;
    KkcInputMode m_mode;
    QString m_keyString;
};

class ShortcutModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ShortcutModel(QObject* parent = 0);
    virtual ~ShortcutModel();
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool add(const ShortcutEntry& entry);
    void remove(const QModelIndex& index);
    void load(const QString& name);
    void save();
    bool needSave();

Q_SIGNALS:
    void needSaveChanged(bool needSave);

private:
    void setNeedSave(bool arg1);

private:
    QList<ShortcutEntry> m_entries;
    KkcUserRule* m_userRule;
    bool m_needSave;
};

#endif // DICTMODEL_H
