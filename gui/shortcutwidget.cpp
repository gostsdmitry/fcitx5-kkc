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

#include "shortcutwidget.h"
#include "addshortcutdialog.h"
#include "rulemodel.h"
#include "shortcutmodel.h"
#include "ui_shortcutwidget.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcntl.h>

namespace fcitx {

KkcShortcutWidget::KkcShortcutWidget(QWidget *parent)
    : FcitxQtConfigUIWidget(parent), m_ui(new ::Ui::KkcShortcutWidget) {
    m_ruleModel = new RuleModel(this);
    m_shortcutModel = new ShortcutModel(this);
    m_ui->setupUi(this);
    m_ui->ruleLabel->setText(_("&Rule:"));
    m_ui->ruleComboBox->setModel(m_ruleModel);
    m_ui->shortcutView->setModel(m_shortcutModel);
    m_ui->shortcutView->sortByColumn(3);

    connect(m_ui->ruleComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &KkcShortcutWidget::ruleChanged);
    connect(m_ui->addShortcutButton, &QPushButton::clicked, this,
            &KkcShortcutWidget::addShortcutClicked);
    connect(m_ui->removeShortCutButton, &QPushButton::clicked, this,
            &KkcShortcutWidget::removeShortcutClicked);
    connect(m_shortcutModel, &ShortcutModel::needSaveChanged, this,
            &KkcShortcutWidget::shortcutNeedSaveChanged);
    connect(m_ui->shortcutView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &KkcShortcutWidget::currentShortcutChanged);

    load();
    currentShortcutChanged();
}

KkcShortcutWidget::~KkcShortcutWidget() { delete m_ui; }

QString KkcShortcutWidget::addon() { return "fcitx-kkc"; }

void KkcShortcutWidget::load() {
    auto fd = StandardPath::global().open(StandardPath::Type::PkgConfig,
                                          "kkc/rule", O_RDONLY);
    QString sline;
    do {
        if (fd.fd() >= 0) {
            break;
        }

        QFile f;
        QByteArray line;
        if (f.open(fd.fd(), QIODevice::ReadOnly)) {
            line = f.readLine();
            f.close();
        }

        sline = QString::fromUtf8(line).trimmed();

        if (sline.isEmpty()) {
            sline = "default";
        }
    } while (0);
    m_ruleModel->load();
    int idx = m_ruleModel->findRule(sline);
    idx = idx < 0 ? 0 : idx;
    m_ui->ruleComboBox->setCurrentIndex(idx);

    Q_EMIT changed(false);
}

void KkcShortcutWidget::save() {
    m_shortcutModel->save();

    QString name =
        m_ruleModel
            ->data(m_ruleModel->index(m_ui->ruleComboBox->currentIndex(), 0),
                   Qt::UserRole)
            .toString();

    StandardPath::global().safeSave(StandardPath::Type::PkgData, "kkc/rule",
                                    [name](int fd) {
                                        QFile f;
                                        if (f.open(fd, QIODevice::WriteOnly)) {
                                            f.write(name.toUtf8());
                                            f.close();
                                        } else {
                                            return false;
                                        }
                                        return true;
                                    });

    Q_EMIT changed(false);
}

QString KkcShortcutWidget::title() { return _("Shortcut Manager"); }

QString KkcShortcutWidget::icon() { return "fcitx-kkc"; }

void KkcShortcutWidget::ruleChanged(int rule) {
    QString name =
        m_ruleModel->data(m_ruleModel->index(rule, 0), Qt::UserRole).toString();
    if (m_shortcutModel->needSave()) {
        int ret = QMessageBox::question(
            this, _("Save Changes"),
            _("The content has changed.\n"
              "Do you want to save the changes or discard them?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            m_shortcutModel->save();
        } else if (ret == QMessageBox::Cancel) {
            int idx = m_ruleModel->findRule(m_name);
            if (idx < 0) {
                idx = 0;
            }
            m_ui->ruleComboBox->setCurrentIndex(idx);
            return;
        }
    }
    m_shortcutModel->load(name);
    m_name = name;
    Q_EMIT changed(true);
}

void KkcShortcutWidget::addShortcutClicked() {
    AddShortcutDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        if (!m_shortcutModel->add(dialog.shortcut())) {
            QMessageBox::critical(
                this, _("Key Conflict"),
                _("Key to add is conflict with existing shortcut."));
        }
    }
}

void KkcShortcutWidget::removeShortcutClicked() {
    QModelIndex idx = m_ui->shortcutView->currentIndex();
    if (idx.isValid()) {
        m_shortcutModel->remove(idx);
    }
}

void KkcShortcutWidget::shortcutNeedSaveChanged(bool needSave) {
    if (needSave) {
        Q_EMIT changed(true);
    }
}

void KkcShortcutWidget::currentShortcutChanged() {
    m_ui->removeShortCutButton->setEnabled(
        m_ui->shortcutView->currentIndex().isValid());
}

} // namespace fcitx
