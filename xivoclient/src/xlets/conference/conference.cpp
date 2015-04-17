/* XiVO Client
 * Copyright (C) 2007-2015 Avencall
 *
 * This file is part of XiVO Client.
 *
 * XiVO Client is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with a Section 7 Additional
 * Permission as follows:
 *   This notice constitutes a grant of such permission as is necessary
 *   to combine or link this software, or a modified version of it, with
 *   the OpenSSL project's "OpenSSL" library, or a derivative work of it,
 *   and to copy, modify, and distribute the resulting work. This is an
 *   extension of the special permission given by Trolltech to link the
 *   Qt code with the OpenSSL library (see
 *   <http://doc.trolltech.com/4.4/gpl.html>). The OpenSSL library is
 *   licensed under a dual license: the OpenSSL License and the original
 *   SSLeay license.
 *
 * XiVO Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XiVO Client.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QAction>
#include <QDebug>

#include <baseengine.h>

#include "conference.h"
#include "conference_list_model.h"
#include "conference_room_model.h"

Conference::Conference(QWidget *parent)
    : XLet(parent, tr("Conference"), ":/images/tab-conference.svg"),
      m_list_model(NULL),
      m_room_model(NULL)
{
    this->ui.setupUi(this);

    QFile qssFile(QString(":/default.qss"));
    if(qssFile.open(QIODevice::ReadOnly)) {
        this->setStyleSheet(qssFile.readAll());
    }

    QAction *conflist_action = this->ui.menu->addAction(tr("Room list"));
    this->ui.menu->addAction();
    this->ui.menu->setSelectedIndex(ROOM_LIST);
    this->showConfList();

    /* CONFLIST */
    // this contains the data, unordered
    m_list_model = new ConferenceListModel(this);

    // this maps the indexes between the sorted view and the unordered model
    QSortFilterProxyModel *list_proxy_model = new QSortFilterProxyModel(this);
    list_proxy_model->setSourceModel(m_list_model);
    list_proxy_model->setDynamicSortFilter(true); /* sorts right on insertion, instead
    of half a second after the window has appeared */

    // this displays the sorted data
    this->ui.list_table->setModel(list_proxy_model);
    this->ui.list_table->sortByColumn(ConferenceListModel::NAME, Qt::AscendingOrder);

    /* CONFROOM */
    m_room_model = new ConferenceRoomModel(this);
    this->ui.room_table->setModel(m_room_model);
    this->ui.room_table->updateHeadersView();

    connect(conflist_action, SIGNAL(triggered()),
            this, SLOT(showConfList()));
    connect(this->ui.list_table, SIGNAL(openConfRoom(QString &)),
            this, SLOT(showConfRoom(QString &)));
    connect(b_engine, SIGNAL(meetmeUpdate(const QVariantMap &)),
            this, SLOT(updateConference(const QVariantMap &)));
    registerMeetmeUpdate();
}

void Conference::updateConference(const QVariantMap & config)
{
    m_confroom_configs = config;
    this->m_list_model->updateConfList(m_confroom_configs);

    QString room_number = this->m_room_model->getRoomNumber();
    if (! room_number.isEmpty() &&
        this->ui.conference_tables->currentIndex() ==
        this->ui.conference_tables->indexOf(this->ui.room_page))
    {
        QVariantMap confroom_members = m_confroom_configs[room_number].toMap()["members"].toMap();
        m_room_model->updateConfRoom(confroom_members);
    }
}

void Conference::registerMeetmeUpdate() const
{
    QVariantMap command;

    command["class"] = "subscribe";
    command["message"] = "meetme_update";

    b_engine->sendJsonCommand(command);
}

void Conference::showConfList()
{
    int index = this->ui.conference_tables->indexOf(this->ui.list_page);
    this->ui.conference_tables->setCurrentIndex(index);
    this->ui.menu->hideIndex(ROOM_NUMBER);
}

void Conference::showConfRoom(QString & room_number)
{
    this->m_room_model->setRoomNumber(room_number);
    QVariantMap confroom_config = m_confroom_configs[room_number].toMap()["members"].toMap();
    m_room_model->updateConfRoom(confroom_config);

    int index = this->ui.conference_tables->indexOf(this->ui.room_page);
    this->ui.conference_tables->setCurrentIndex(index);

    this->ui.menu->showIndex(ROOM_NUMBER);
    this->ui.menu->setTextIndex(ROOM_NUMBER, room_number);
    this->ui.menu->setSelectedIndex(ROOM_NUMBER);
}


XLet* XLetConferencePlugin::newXLetInstance(QWidget *parent)
{
    b_engine->registerTranslation(":/obj/conference_%1");
    return new Conference(parent);
}
