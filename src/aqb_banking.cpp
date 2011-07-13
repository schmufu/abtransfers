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
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#include "aqb_banking.h"

#include <QDebug>

#include <gwen-gui-qt4/qt4_gui.hpp>
#include <aqbanking/bankinfo.h>

aqb_banking::aqb_banking()
{
	int rv;
	QT4_Gui *gui;

	gui = new QT4_Gui();
	GWEN_Gui_SetGui(gui->getCInterface());

	this->ab = AB_Banking_new("ab_transfer", NULL, 0);

	/* This is the basic init function. It only initializes the minimum (like
	 * setting up plugin and data paths). After this function successfully
	 * returns you may freely use any non-online function. To use online
	 * banking functions (like getting the list of managed accounts, users
	 * etc) you will have to call AB_Banking_OnlineInit().
	 */
	rv = AB_Banking_Init(this->ab);
	if (rv) {
	  fprintf(stderr, "Error on init (%d)\n", rv);
	  return;
	}
	qDebug() << "AqBanking successfully initialized.";

	/* This function loads the settings file of AqBanking so the users and
	 * accounts become available after this function successfully returns.
	 */
	rv = AB_Banking_OnlineInit(ab);
	if (rv) {
	  fprintf(stderr, "Error on init of online modules (%d)\n", rv);
	  return;
	}

}

aqb_banking::~aqb_banking()
{
	int rv;

	/* This function MUST be called in order to let AqBanking save the changes
	 * to the users and accounts (like they occur after executing jobs).
	 */
	rv = AB_Banking_OnlineFini(this->ab);
	if (rv) {
	  fprintf(stderr, "ERROR: Error on deinit online modules (%d)\n", rv);
	  return;
	}

	/* This function deinitializes AqBanking. It undoes the effects of
	 * AB_Banking_Init() and should be called before destroying an AB_BANKING
	 * object.
	 */
	rv = AB_Banking_Fini(ab);
	if (rv) {
	  fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
	  return;
	}
	AB_Banking_free(ab);

	qDebug() << "AqBanking successfully deinitialized";

}

/*! \brief Gibt den Institut-Namen zur BLZ zurück
  */
QString aqb_banking::getInstituteFromBLZ(const QString &BLZ) const
{
	AB_BANKINFO *bankinfo;
	QString Institute = "NO INFORMATION";
	bankinfo = AB_Banking_GetBankInfo(this->ab, "de", "", BLZ.toUtf8());

	if (!bankinfo) {
		//Keine Informationen vorhanden
		return Institute;
	} else {
		Institute = AB_BankInfo_GetBankName(bankinfo);
		AB_BankInfo_free(bankinfo);
		return Institute;
	}
}
