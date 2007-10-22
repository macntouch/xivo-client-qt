/* XIVO CTI clients
Copyright (C) 2007  Proformatique

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

/* $Revision$
 * $Date$
*/

#ifndef __POPUP_H__
#define __POPUP_H__

#include <QWidget>
#include <QIODevice>
#include <QVBoxLayout>
#include <QTcpSocket>
#include "xmlhandler.h"

class QComboBox;
class QLineEdit;
class QUrl;

/*! \brief Profile popup widget
 *
 * Constructed from the message received */
class Popup: public QWidget
{
	Q_OBJECT
public:
	//! Construct from a QIODevice used to read XML input
	Popup(QIODevice *inputstream,
              const bool & sheetui,
              QWidget * parent = 0);
	//! Add a Text field (name, value)
	void addInfoText(const QString & name, const QString & value);
	//! Add a url field
	void addInfoLink(const QString & name, const QString & value);
	void addInfoLinkX(const QString & name, const QString & value, const QString & dispvalue);
	//! Add a Picture
	void addInfoPicture(const QString & name, const QString & value);
	//! Add a Phone number
	void addInfoPhone(const QString & name, const QString & value);
	//! Add a Phone number
	void addInfoPhoneURL(const QString & name, const QString & value);
	//! getter for the message
	void setMessage(const QString & msg);
	//! access to the message
	const QString & message() const;
	//! finalize the Construction of the window and show it
	void finishAndShow();
signals:
	void wantsToBeShown(Popup *);	//!< sent when the widget want to show itself
	void emitDial(const QString &);	//!< sent when the widget wants to dial
        void save(const QString &);
public slots:
	void streamNewData();		//!< new input data is available
	void streamAboutToClose();	//!< catch aboutToClose() signal from the socket
	void socketDisconnected();	//!< connected to disconnected() signal
	void socketError(QAbstractSocket::SocketError err);	//!< socket error handling
        void dialThisNumber();
        void dispurl(const QUrl &);
        void httpGetNoreply();
        void saveandclose();
protected:
	void closeEvent(QCloseEvent *event);	//!< catch close event
private:
	QIODevice * m_inputstream;	//!< input stream where the XML is read from
	/* the following properties are for XML parsing */
	//! QXmlInputSource constructed from m_inputstream
	QXmlInputSource m_xmlInputSource;
	//! XML parser object.
	QXmlSimpleReader m_reader;
	//! Handler for event generated by the XML parser
	XmlHandler m_handler;
	bool m_parsingStarted;		//! Is the XML already started or not ?
	//! layout for the widget : vertical box
	QVBoxLayout * m_vlayout;
	QString m_message;	//! Message property
        bool m_sheetui;
        QWidget * m_sheetui_widget;
};

#endif
