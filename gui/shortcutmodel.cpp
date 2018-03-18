//
// Copyright (C) 2013~2017 by CSSlayer
// wengxt@gmail.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "shortcutmodel.h"
#include "common.h"
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>
#include <libkkc/libkkc.h>

namespace fcitx {

ShortcutModel::ShortcutModel(QObject *parent)
    : QAbstractTableModel(parent), m_userRule(nullptr, &g_object_unref),
      m_needSave(false) {}

ShortcutModel::~ShortcutModel() {}

int ShortcutModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_entries.length();
}

int ShortcutModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : 3;
}

const char *modeName[] = {
    N_("Hiragana"), N_("Katakana"),   N_("Half width Katakana"),
    N_("Latin"),    N_("Wide latin"), N_("Direct input"),
};

QVariant ShortcutModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_entries.size() || index.column() >= 3) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return _(modeName[m_entries[index.row()].mode()]);
        case 1:
            return m_entries[index.row()].keyString();
        case 2:
            return m_entries[index.row()].label();
        }
        return QVariant();
    }
    return QVariant();
}

QVariant ShortcutModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const {
    if (orientation == Qt::Vertical) {
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    switch (role) {
    case Qt::DisplayRole:
        switch (section) {
        case 0:
            return _("Input Mode");
        case 1:
            return _("Key");
        case 2:
            return _("Function");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void ShortcutModel::load(const QString &name) {
    setNeedSave(false);
    beginResetModel();

    do {
        m_userRule.reset();
        m_entries.clear();

        KkcRuleMetadata *ruleMeta =
            kkc_rule_metadata_find(name.toUtf8().constData());
        if (!ruleMeta) {
            return;
        }

        std::string basePath = stringutils::joinPath(
            StandardPath::global().userDirectory(StandardPath::Type::PkgData),
            "kkc/rules");

        auto userRule = makeGObjectUnique(
            kkc_user_rule_new(ruleMeta, basePath.data(), "fcitx-kkc", NULL));
        if (!userRule) {
            break;
        }

        for (int mode = 0; mode <= KKC_INPUT_MODE_DIRECT; mode++) {
            auto keymap = makeGObjectUnique(kkc_rule_get_keymap(
                KKC_RULE(userRule.get()), (KkcInputMode)mode));
            int length;
            KkcKeymapEntry *entries = kkc_keymap_entries(keymap.get(), &length);

            for (int i = 0; i < length; i++) {
                if (entries[i].command) {
                    gchar *label =
                        kkc_keymap_get_command_label(entries[i].command);
                    m_entries << ShortcutEntry(
                        QString::fromUtf8(entries[i].command), entries[i].key,
                        QString::fromUtf8(label), (KkcInputMode)mode);
                    g_free(label);
                }
            }

            for (int i = 0; i < length; i++) {
                kkc_keymap_entry_destroy(&entries[i]);
            }
            g_free(entries);
        }

        m_userRule = std::move(userRule);
    } while (0);

    endResetModel();
}

void ShortcutModel::save() {
    if (m_userRule && m_needSave) {
        for (int mode = 0; mode <= KKC_INPUT_MODE_DIRECT; mode++) {
            kkc_user_rule_write(m_userRule.get(), (KkcInputMode)mode, NULL);
        }
    }

    setNeedSave(false);
}

bool ShortcutModel::add(const ShortcutEntry &entry) {
    auto map = makeGObjectUnique(
        kkc_rule_get_keymap(KKC_RULE(m_userRule.get()), entry.mode()));
    bool result = true;
    if (kkc_keymap_lookup_key(map.get(), entry.event())) {
        result = false;
    }

    if (result) {
        beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
        m_entries << entry;
        kkc_keymap_set(map.get(), entry.event(),
                       entry.command().toUtf8().constData());
        endInsertRows();
        setNeedSave(true);
    }

    return result;
}

void ShortcutModel::remove(const QModelIndex &index) {
    if (!m_userRule) {
        return;
    }

    if (!index.isValid()) {
        return;
    }

    if (index.row() >= m_entries.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    auto map = makeGObjectUnique(kkc_rule_get_keymap(
        KKC_RULE(m_userRule.get()), m_entries[index.row()].mode()));
    kkc_keymap_set(map.get(), m_entries[index.row()].event(), NULL);

    m_entries.removeAt(index.row());
    endRemoveRows();

    setNeedSave(true);
}

void ShortcutModel::setNeedSave(bool needSave) {
    if (m_needSave != needSave) {
        m_needSave = needSave;
        Q_EMIT needSaveChanged(m_needSave);
    }
}

bool ShortcutModel::needSave() { return m_needSave; }

} // namespace fcitx
