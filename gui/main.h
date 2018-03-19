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
#ifndef _GUI_MAIN_H_
#define _GUI_MAIN_H_

#include <fcitxqtconfiguiplugin.h>

namespace fcitx {

class KkcConfigPlugin : public FcitxQtConfigUIPlugin {
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID FcitxQtConfigUIFactoryInterface_iid FILE
                      "kkc-config.json")
    explicit KkcConfigPlugin(QObject *parent = 0);
    FcitxQtConfigUIWidget *create(const QString &key) override;
};
} // namespace fcitx

#endif // _GUI_MAIN_H_
