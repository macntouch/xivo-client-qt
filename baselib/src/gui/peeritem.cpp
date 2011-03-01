/* XiVO Client
 * Copyright (C) 2007-2011, Proformatique
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

/* $Revision$
 * $Date$
 */

#include <QDebug>

#include "peeritem.h"
#include "peerwidget.h"
#include "userinfo.h"
#include "xivoconsts.h"

/*! \brief Constructor
 */
PeerItem::PeerItem(UserInfo * ui)
    : m_ui(ui)
{
    m_peerwidget = NULL;
}

PeerItem::PeerItem()
{
    m_peerwidget = NULL;
}

/*! \brief Copy constructor
 */
PeerItem::PeerItem(const PeerItem &peer)
{
    m_ui = peer.m_ui;
    m_peerwidget = peer.m_peerwidget;
}

/*! \brief update status of the peer
 *
 * Change what is displayed according to new status values.
 */
void PeerItem::updateStatus()
{
    if(m_peerwidget != NULL)
        updateDisplayedStatus();
}

void PeerItem::updateAgentStatus(const QVariant & agentstatus)
{
    m_agentstatus = agentstatus;
    if(m_peerwidget != NULL)
        updateDisplayedStatus();
}

/*! \brief update name if changed
 */
void PeerItem::updateName(const QString & newname)
{
    if(newname != m_ui->fullname()) {
        m_ui->setFullName(newname);
        if(m_peerwidget != NULL)
            updateDisplayedName();
    }
}

/*! \brief update status of the peer
 *
 * Change what is displayed according to new status values.
 */
void PeerItem::updateDisplayedStatus()
{
    if(m_peerwidget == NULL)
        return;

    m_peerwidget->updatePresence();
    m_peerwidget->updatePhonesStates();
}

/*! \brief update name if changed
 */
void PeerItem::updateDisplayedName()
{
    if(m_peerwidget == NULL) {
        return;
    }

    m_peerwidget->setName(m_ui->fullname());
    return;
}

UserInfo * PeerItem::userinfo()
{
    return m_ui;
}
