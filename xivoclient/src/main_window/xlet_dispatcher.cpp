/* XiVO Client
 * Copyright (C) 2013-2015 Avencall
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

#include <QMap>

#include <baseengine.h>
#include <xletfactory.h>
#include <QPushButton>

#include "main_window.h"
#include "main_widget.h"
#include "tab_background.h"
#include "xlet_dispatcher.h"

XletDispatcher::XletDispatcher(MainWindow *main_window, MainWidget *main_widget, QObject *parent)
    : QObject(parent),
      m_main_window(main_window),
      m_main_widget(main_widget),
      m_dock_container(NULL),
      m_xlets_dock_widget(),
      m_xlets_dock(),
      m_grid_container(NULL),
      m_xlets_grid_widget(),
      m_xlets_grid(),
      m_tab_container(NULL),
      m_xlets_tab_widget(),
      m_xlets_tab(),
      m_has_tabber(false),
      m_tabber_style(),
      m_fold_signal_mapper(new QSignalMapper(this)),
      m_unfold_signal_mapper(new QSignalMapper(this))
{
    this->connect(b_engine, SIGNAL(logged()), SLOT(setStatusLogged()));
    this->connect(b_engine, SIGNAL(delogged()), SLOT(setStatusNotLogged()));
    this->connect(this->m_fold_signal_mapper, SIGNAL(mapped(const QString &)),
                  this, SLOT(showOneXlet(const QString &)));
    this->connect(this->m_unfold_signal_mapper, SIGNAL(mapped(const QString &)),
                  this, SLOT(showAllXlets()));
}

void XletDispatcher::showOneXlet(const QString &xlet_name)
{
    XLet *identity = m_xlets[xlet_name];
    QMap<QString, XLet *> xlets_to_hide = this->m_xlets_grid_widget;
    xlets_to_hide.remove(xlet_name);

    this->hideXletsDock();

    if(m_has_tabber) {
        b_engine->getSettings()->setValue("display/lastfocusedtab", this->m_tab_container->currentIndex());
        m_tab_container->hide();
    }

    foreach (QWidget *widget, xlets_to_hide.values()) {
        widget->hide();
    }

    m_unfolded_height = m_main_window->height();

    if (m_main_window->isMaximized()) {
        m_main_window->showNormal();
    }

    m_main_window->setFixedHeight(identity->height() \
                                  + m_main_window->statusBar()->height() \
                                  + m_main_window->menuBar()->height() \
                                  );
}

void XletDispatcher::showAllXlets()
{

    foreach (QWidget *widget, this->m_xlets_grid_widget.values()) {
        widget->show();
    }

    if(m_has_tabber) {
        m_tab_container->setVisible(true);
        this->m_tab_container->setCurrentIndex(b_engine->getSettings()->value("display/lastfocusedtab").toInt());
    }

    this->showXletsDock();


    this->restoreMainWindow();
}

XletDispatcher::~XletDispatcher()
{
}

void XletDispatcher::setStatusLogged()
{
    this->prepareAppearance();
    this->prepareXletsGrid();
    this->prepareXletsDock();
    m_unfolded_height = m_main_window->height();
}

void XletDispatcher::setStatusNotLogged()
{
    this->restoreMainWindow();
    this->m_main_window->saveState();

    this->cleanXletsGrid();
    this->cleanXletsDock();
    this->clearAppearance();
}

bool XletDispatcher::hasWidget()
{
    return this->m_xlets_grid.size() != 0;
}

XLet *XletDispatcher::xletFactory(const QString &xlet_id)
{
    XLet *xlet = XLetFactory::spawn(xlet_id, this->m_main_widget);
    if (xlet) {
        xlet->doGUIConnects(this->m_main_window);
        this->m_xlets.insert(xlet_id, xlet);
    } else {
        qDebug() << Q_FUNC_INFO << "cannot instantiate XLet" << xlet_id;
    }
    return xlet;
}

void XletDispatcher::prepareXletsGrid()
{
    if (!this->hasWidget()) {
        return;
    }

    this->m_grid_container = new QVBoxLayout(this->m_main_widget);
    this->m_grid_container->setContentsMargins(0,0,0,0);
    if (! m_grid_container) {
        qDebug() << Q_FUNC_INFO << "Failed to instanciate the grid container";
        return;
    }

    foreach (const XletAndOption &xlet_and_option, this->m_xlets_grid) {
        const QString &name = xlet_and_option.first;
        const QString &options = xlet_and_option.second;
        if (name == "tabber") {
            this->prepareXletsTab();
            QWidget *tab_background = new TabBackground(m_tab_container, this->m_main_widget);
            this->m_grid_container->insertWidget(options.toInt(), tab_background);
        } else {
            XLet *xlet = this->xletFactory(name);
            if (xlet) {
                connect(xlet, SIGNAL(showOnlyMeRequested()),
                        m_fold_signal_mapper, SLOT(map()));
                m_fold_signal_mapper->setMapping(xlet, name);
                connect(xlet, SIGNAL(showOthersRequested()),
                        m_unfold_signal_mapper, SLOT(map()));
                m_unfold_signal_mapper->setMapping(xlet, name);
                this->m_grid_container->insertWidget(options.toInt(), xlet);
                this->m_xlets_grid_widget.insert(name, xlet);
            }
        }
    }
}

void XletDispatcher::cleanXletsGrid()
{
    if (!this->hasWidget()) {
        return;
    }

    foreach (QWidget *widget, this->m_xlets_grid_widget.values()) {
        this->m_grid_container->removeWidget(widget);
        widget->deleteLater();
    }
    if (this->m_has_tabber) {
        this->m_grid_container->removeWidget(this->m_tab_container);
        this->cleanXletsTab();
    }
    this->m_grid_container->deleteLater();
    this->m_xlets_grid_widget.clear();
}

void XletDispatcher::prepareXletsTab()
{
    if (this->m_xlets_tab.size() == 0) {
        return;
    }

    this->m_tab_container = new QTabWidget(this->m_main_widget);
    this->m_tab_container->setTabPosition(QTabWidget::West);
    // A little more space before the first tab
    this->m_tab_container->setStyleSheet("QTabWidget::tab-bar {top: 17px;}");

    this->m_tab_container->tabBar()->setAttribute(Qt::WA_Hover);
    this->m_tab_container->tabBar()->setStyle(&m_tabber_style);
    this->m_tab_container->tabBar()->setIconSize(QSize(30, 30));
    this->m_tab_container->tabBar()->setDrawBase(true);
    this->m_has_tabber = true;

    foreach (const XletAndOption &xlet_and_option, this->m_xlets_tab) {
        const QString &name = xlet_and_option.first;
        XLet *xlet = this->xletFactory(name);
        if (xlet) {
            int tabIndex = this->m_tab_container->addTab(xlet, QIcon(xlet->iconPath()), "");
            this->m_tab_container->setTabToolTip(tabIndex, xlet->title());
            this->m_xlets_tab_widget.insert(name, xlet);
        }
    }

    this->m_tab_container->setCurrentIndex(b_engine->getSettings()->value("display/lastfocusedtab").toInt());
}

void XletDispatcher::cleanXletsTab()
{
    if (this->m_xlets_tab.size() == 0) {
        return;
    }

    b_engine->getSettings()->setValue("display/lastfocusedtab", this->m_tab_container->currentIndex());
    foreach (QWidget *widget, this->m_xlets_tab_widget.values()) {
        widget->deleteLater();
    }
    this->m_tab_container->deleteLater();
    this->m_xlets_tab_widget.clear();
    this->m_has_tabber = false;
}

void XletDispatcher::prepareXletsDock()
{
    if (this->m_xlets_dock.size() == 0) {
        return;
    }

    foreach (const XletAndOption &xlet_and_option, this->m_xlets_dock) {
        const QString &name = xlet_and_option.first;
        const QString &options = xlet_and_option.second;

        XLet *xlet = this->xletFactory(name);
        if (! xlet) {
            qDebug() << Q_FUNC_INFO << "Failed to instanciate xlet" << name;
            continue;
        }

        QDockWidget::DockWidgetFeatures features = this->getXletsDockFeatures(options);
        QDockWidget *dockWidget = new QDockWidget(xlet->title(), this->m_main_widget);
        dockWidget->setFeatures(features);
        dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
        dockWidget->setObjectName(name);
        if (options.contains("s")) { // with scrollbar ?
            QScrollArea *dockWidgetContents = new QScrollArea(this->m_main_widget);
            dockWidgetContents->setWidget(xlet);
            dockWidgetContents->setWidgetResizable(true);
            dockWidget->setWidget(dockWidgetContents);
        } else {
            dockWidget->setWidget(xlet);
        }
        this->m_main_window->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
        this->m_xlets_dock_widget.insert(name, dockWidget);
        dockWidget->hide();
    }
}

QDockWidget::DockWidgetFeatures XletDispatcher::getXletsDockFeatures(const QString &options)
{
    QDockWidget::DockWidgetFeatures features = QDockWidget::NoDockWidgetFeatures;
    if (options.contains("c")) {
        features |= QDockWidget::DockWidgetClosable;
    }
    if (options.contains("f")) {
        features |= QDockWidget::DockWidgetFloatable;
    }
    if (options.contains("m")) {
        features |= QDockWidget::DockWidgetMovable;
    }
    return features;
}

void XletDispatcher::cleanXletsDock()
{
    foreach (QDockWidget *widget, this->m_xlets_dock_widget.values()) {
        this->m_main_window->removeDockWidget(widget);
        widget->deleteLater();
    }
    this->m_xlets_dock_widget.clear();
}

void XletDispatcher::hideXletsDock()
{
    this->m_main_window->saveState();
    foreach (QDockWidget *widget, this->m_xlets_dock_widget.values()) {
        widget->hide();
    }
}

void XletDispatcher::showXletsDock()
{
    foreach (QDockWidget *widget, this->m_xlets_dock_widget.values()) {
        widget->show();
    }
    this->m_main_window->restoreState();
}

/*! \brief show this XLet on top of others
 *
 * This slot Works when tabbed view is selected.
 */
void XletDispatcher::showWidgetOnTop(QWidget *widget)
{
    if (this->m_tab_container)
        this->m_tab_container->setCurrentWidget(widget);
}

void XletDispatcher::prepareAppearance()
{
    const QVariantList &xlets_infos = b_engine->getCapaXlets();
    foreach (const QVariant &qvariant_xlet_infos, xlets_infos) {
        const QStringList &xlet_infos_raw = qvariant_xlet_infos.toStringList();

        if (xlet_infos_raw.size() < 2) {
            continue;
        }

        const QString &name = xlet_infos_raw[0];
        const QString &type = xlet_infos_raw[1];
        QString options = "";
        if ((name == "customerinfo") && (! b_engine->checkedFunction(name))) {
                continue;
        }
        if (xlet_infos_raw.size() > 2) {
            options = xlet_infos_raw[2];
        }

        XletAndOption pair = XletAndOption(name, options);
        if (type == "dock" && name != "tabber") {
            this->m_xlets_dock << pair;
        } else if (type == "grid") {
            this->m_xlets_grid << pair;
        } else if (type == "tab" && name != "tabber") {
            this->m_xlets_tab << pair;
        }
    }
}

void XletDispatcher::clearAppearance()
{
    m_xlets_dock.clear();
    m_xlets_grid.clear();
    m_xlets_tab.clear();
}

void XletDispatcher::restoreMainWindow()
{
    m_main_window->setMaximumHeight(QWIDGETSIZE_MAX);
    QRect transformed_rect = m_main_window->geometry();
    transformed_rect.setHeight(m_unfolded_height);

    m_main_window->setGeometry(transformed_rect);
    m_main_window->updateGeometry();
}
