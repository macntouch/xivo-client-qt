/* XiVO Client
 * Copyright (C) 2012-2013, Avencall
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

#include <QWidget>
#include <QTimer>
#include <baseengine.h>
#include <message_factory.h>
#include <signal_relayer.h>
#include "ui_current_call.h"
#include "current_call.h"

QString CurrentCall::attended_transfer_label = QObject::tr("Indirect T");
QString CurrentCall::complete_transfer_label = QObject::tr("Complete T");
QKeySequence CurrentCall::attended_transfer_key = QKeySequence("F5");
QString CurrentCall::hangup_label = QObject::tr("Hangup");
QString CurrentCall::cancel_transfer_label = QObject::tr("Cancel T");
QKeySequence CurrentCall::hangup_key = QKeySequence("F8");


CurrentCall::CurrentCall(QObject *parent)
    : QObject(parent),
      m_current_call_widget(NULL),
      m_call_start(0)
{
    this->registerListener("current_calls");

    QTimer * timer_display = new QTimer(this);
    connect(timer_display, SIGNAL(timeout()),
            this, SLOT(updateCallInfo()));
    timer_display->start(1000);
}

CurrentCall::~CurrentCall()
{
}

void CurrentCall::setParentWidget(QWidget *parent)
{
    m_current_call_widget = new Ui::CurrentCallWidget();
    m_current_call_widget->setupUi(parent);

    this->noCallsMode();
    connect(signal_relayer, SIGNAL(attendedTransferSent()),
            this, SLOT(transferringMode()));
}

void CurrentCall::updateCallerID(const QString &name,
                                 const QString &number)
{
    m_caller_id = QString("%1 <%2>").arg(name).arg(number);
}

void CurrentCall::updateCallInfo()
{
    QString time;
    if (m_call_start != 0) {
        time = b_engine->timeElapsed(m_call_start);
    }
    QString info = QString("%1 %2").arg(this->m_caller_id).arg(time);
    this->m_current_call_widget->lbl_call_info->setText(info);
}

void CurrentCall::parseCommand(const QVariantMap &current_calls)
{
    const QVariantList &calls = current_calls["current_calls"].toList();
    if (calls.isEmpty()) {
        this->clear();
    } else {
        this->updateCall(calls);
    }
}

void CurrentCall::updateCall(const QVariantList &calls)
{
    foreach (const QVariant &call, calls) {
        const QVariantMap &call_map = call.toMap();
        if (call_map["call_status"].toString() != "up") {
            continue;
        }
        this->updateCallerID(call_map["cid_name"].toString(),
                             call_map["cid_number"].toString());
        this->m_call_start = call_map["call_start"].toDouble();
        this->updateCallInfo();
        this->answeringMode();
    }
}

void CurrentCall::clear()
{
    this->m_caller_id.clear();
    this->m_call_start = 0;
    this->updateCallInfo();
    this->noCallsMode();
}

void CurrentCall::hangup()
{
    b_engine->sendJsonCommand(MessageFactory::hangup());
}

void CurrentCall::hold()
{
    b_engine->sendJsonCommand(MessageFactory::holdSwitchboard());
}

void CurrentCall::attendedTransfer()
{
    signal_relayer->relayAttendedTransferRequested();
}

void CurrentCall::completeTransfer()
{
    b_engine->sendJsonCommand(MessageFactory::completeTransfer());
}

void CurrentCall::cancelTransfer()
{
    b_engine->sendJsonCommand(MessageFactory::cancelTransfer());
    this->answeringMode();
}

void CurrentCall::noCallsMode()
{
    this->disconnectButtons();
    this->m_current_call_widget->btn_attended_transfer->setEnabled(false);
    m_current_call_widget->btn_attended_transfer->setText(attended_transfer_label);

    this->m_current_call_widget->btn_hold->setEnabled(false);

    this->m_current_call_widget->btn_hangup->setEnabled(false);
    m_current_call_widget->btn_hangup->setText(hangup_label);
}

void CurrentCall::answeringMode()
{
    this->disconnectButtons();
    this->setAttendedTransferButton();

    this->setHoldButton();

    this->setHangupButton();
}

void CurrentCall::transferringMode()
{
    this->disconnectButtons();
    this->setCompleteTransferButton();

    this->m_current_call_widget->btn_hold->setEnabled(false);

    this->setCancelTransferButton();
}


void CurrentCall::disconnectButtons()
{
    disconnect(m_current_call_widget->btn_attended_transfer, SIGNAL(clicked()),
               this, SLOT(attendedTransfer()));
    disconnect(m_current_call_widget->btn_attended_transfer, SIGNAL(clicked()),
               this, SLOT(completeTransfer()));

    disconnect(m_current_call_widget->btn_hold, SIGNAL(clicked()),
               this, SLOT(hold()));

    disconnect(m_current_call_widget->btn_hangup, SIGNAL(clicked()),
               this, SLOT(hangup()));
    disconnect(m_current_call_widget->btn_hangup, SIGNAL(clicked()),
               this, SLOT(cancelTransfer()));
}

void CurrentCall::setAttendedTransferButton()
{
    this->m_current_call_widget->btn_attended_transfer->setEnabled(true);
    m_current_call_widget->btn_attended_transfer->setText(attended_transfer_label);
    m_current_call_widget->btn_attended_transfer->setShortcut(attended_transfer_key);
    connect(m_current_call_widget->btn_attended_transfer, SIGNAL(clicked()),
            this, SLOT(attendedTransfer()));
}

void CurrentCall::setCompleteTransferButton()
{
    this->m_current_call_widget->btn_attended_transfer->setEnabled(true);
    m_current_call_widget->btn_attended_transfer->setText(complete_transfer_label);
    m_current_call_widget->btn_attended_transfer->setShortcut(attended_transfer_key);
    connect(m_current_call_widget->btn_attended_transfer, SIGNAL(clicked()),
            this, SLOT(completeTransfer()));
}

void CurrentCall::setHoldButton()
{
    this->m_current_call_widget->btn_hold->setEnabled(true);
    connect(m_current_call_widget->btn_hold, SIGNAL(clicked()),
            this, SLOT(hold()));
}

void CurrentCall::setHangupButton()
{
    this->m_current_call_widget->btn_hangup->setEnabled(true);
    m_current_call_widget->btn_hangup->setText(hangup_label);
    m_current_call_widget->btn_hangup->setShortcut(hangup_key);
    connect(m_current_call_widget->btn_hangup, SIGNAL(clicked()),
            this, SLOT(hangup()));
}

void CurrentCall::setCancelTransferButton()
{
    this->m_current_call_widget->btn_hangup->setEnabled(true);
    m_current_call_widget->btn_hangup->setText(cancel_transfer_label);
    m_current_call_widget->btn_hangup->setShortcut(hangup_key);
    connect(m_current_call_widget->btn_hangup, SIGNAL(clicked()),
            this, SLOT(cancelTransfer()));
}
