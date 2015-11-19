/* Copyright (C) 2015 Avencall
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QAction>
#include <QDebug>
#include <baseengine.h>
#include <message_factory.h>

#include "people_entry_model.h"
#include "people_entry_view.h"
#include "people_action_generator.h"

PeopleActionGenerator::PeopleActionGenerator(PeopleEntryModel *model, PeopleEntryView *parent)
    : QObject(parent),
      m_people_entry_model(model)
{
    qDebug() << Q_FUNC_INFO;
    m_number_column_index = this->findColumnOfType(NUMBER);
    m_callable_column_indices = this->findAllColumnOfType(CALLABLE);
}

PeopleActionGenerator::~PeopleActionGenerator()
{
    qDebug() << Q_FUNC_INFO;
}

PeopleEntryModel *PeopleActionGenerator::model() const
{
    return m_people_entry_model;
}

QList<int> PeopleActionGenerator::columnTypes()
{
    QList<int> types;
    for (int i = 0; i < model()->columnCount(); ++i) {
        types.append(model()->headerData(i, Qt::Horizontal, Qt::UserRole).toInt());
    }
    return types;
}

int PeopleActionGenerator::findColumnOfType(ColumnType type)
{
    return columnTypes().indexOf(type);
}

QList<int> PeopleActionGenerator::findAllColumnOfType(ColumnType type)
{
    QList<int> indices;
    QList<int> types = columnTypes();

    for (int i = 0; i < types.size(); ++i) {
        if (types[i] == type) {
            indices.append(i);
        }
    }

    return indices;
}

QVariant PeopleActionGenerator::headerAt(int column)
{
    return model()->headerData(column, Qt::Horizontal, Qt::DisplayRole);
}

QVariant PeopleActionGenerator::dataAt(const QModelIndex &index, int column)
{
    QModelIndex cell = index.child(index.row(), column);
    return model()->data(cell, Qt::DisplayRole);
}

QString PeopleActionGenerator::formatColumnNumber(const QString &title, const QString &number) const
{
    return QString("%1 - %2").arg(title).arg(number);
}

QAction *PeopleActionGenerator::newCallAction(const QModelIndex &index)
{
    if (m_number_column_index == -1) {
        return NULL;
    }
    const QString &number = dataAt(index, m_number_column_index).toString();
    if (number.isEmpty()) {
        return NULL;
    }

    QAction *action = new QAction(tr("Call"), parent());
    action->setData(number);
    connect(action, SIGNAL(triggered()), this, SLOT(call()));
    return action;
}

QList<QAction *> PeopleActionGenerator::newCallCallableActions(const QModelIndex &index)
{
    QList<QAction*> actions;

    foreach (int column, m_callable_column_indices) {
        const QString &number = dataAt(index, column).toString();
        const QString &header = headerAt(column).toString();

        QAction *action = new QAction(formatColumnNumber(header, number), parent());
        action->setData(number);
        connect(action, SIGNAL(triggered()), this, SLOT(call()));
        actions.append(action);
    }

    return actions;
}

void PeopleActionGenerator::call()
{
    const QString &number = static_cast<QAction*>(sender())->data().toString();

    b_engine->sendJsonCommand(MessageFactory::dial(number));
}
