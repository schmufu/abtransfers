/******************************************************************************
 * Copyright (C) 2011 Patrick Wacker
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 ******************************************************************************
 * Dont forget: svn propset svn:keywords "Date Author Rev HeadURL" filename
 ******************************************************************************
 * $HeadURL$
 * $Author$
 * $Date$
 * $Rev$
 *
 * description:
 *	Nur ein TestWidget um die wiget... klassen zu testen
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef PAGEWIDGETTESTS_H
#define PAGEWIDGETTESTS_H

#include <QtGui>
#include <QtCore>

#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobsingledebitnote.h>
#include <aqbanking/jobinternaltransfer.h>
#include <aqbanking/jobeutransfer.h>
#include <aqbanking/jobsepatransfer.h>
#include <aqbanking/jobsepadebitnote.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobloadcellphone.h>

#include <aqbanking/jobcreatedatedtransfer.h>
#include <aqbanking/jobmodifydatedtransfer.h>
#include <aqbanking/jobdeletedatedtransfer.h>
#include <aqbanking/jobgetdatedtransfers.h>

#include <aqbanking/jobcreatesto.h>
#include <aqbanking/jobmodifysto.h>
#include <aqbanking/jobdeletesto.h>
#include <aqbanking/jobgetstandingorders.h>

#include "../globalvars.h"
#include "../abt_conv.h"
#include "../aqb_accounts.h"

/*! \brief NUR FÜR TESTZECKE!
 *
 * Diese Klasse wird nur genutzt um widgets zu testen, im eigentlichen
 * Programm wird Sie nicht verwendet!
 */

class pageWidgetTests : public QWidget
{
	Q_OBJECT
public:
	explicit pageWidgetTests(aqb_Accounts *accs, QWidget *parent = 0);
	~pageWidgetTests();

private:
        QPushButton *m_button1;
        QPushButton *m_button2;
        QPushButton *m_button3;
        QPushButton *m_button4;
        QPushButton *m_button5;
        QPushButton *m_button6;
        QPushButton *m_button7;
        QPushButton *m_button8;
        QPlainTextEdit *m_textEdit;

        aqb_Accounts *m_accounts;

        AB_IMEXPORTER_CONTEXT *m_iec1;
        AB_IMEXPORTER_CONTEXT *m_iec2;
        AB_IMEXPORTER_CONTEXT *m_iec3;

        AB_IMEXPORTER *m_ie1;
        AB_IMEXPORTER *m_ie2;
        AB_IMEXPORTER *m_ie3;

        AB_IMEXPORTER_ACCOUNTINFO *m_iea1;
        AB_IMEXPORTER_ACCOUNTINFO *m_iea2;
        AB_IMEXPORTER_ACCOUNTINFO *m_iea3;

	void addlog(const QString &logMsg);
	void parseContext(AB_IMEXPORTER_CONTEXT *ctx);

signals:

private slots:
	void onButton1Clicked();
	void onButton2Clicked();
	void onButton3Clicked();
	void onButton4Clicked();
	void onButton5Clicked();
	void onButton6Clicked();
	void onButton7Clicked();
	void onButton8Clicked();

public slots:


};

#endif // PAGEWIDGETTESTS_H
