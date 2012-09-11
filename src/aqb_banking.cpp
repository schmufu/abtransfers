/******************************************************************************
 * Copyright (C) 2011-2012 Patrick Wacker
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

#include <aqbanking/bankinfo.h>
#include <aqbanking/abgui.h>

aqb_banking::aqb_banking()
{
	int rv;


	this->ab = AB_Banking_new("abtransfers", NULL, 0);

	//Das GUI muss bereits hier erstellt werden, da es anonsten zu
	//einen Fehler kommen kann. (1x geschehen beim ersetzen der AqBanking
	//Daten von einem anderen Rechner)
	this->gui = new QT4_Gui();
	GWEN_Gui_SetGui(this->gui->getCInterface());

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
	rv = AB_Banking_OnlineInit(this->ab);
	if (rv) {
		fprintf(stderr, "Error on init of online modules (%d)\n", rv);
		AB_Banking_Fini(this->ab);
		return;
	}

	//damit die gemachten Einstellungen des GWEN-Dialogs erhalten bleiben
	AB_Gui_Extend(this->gui->getCInterface(), this->ab);

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

	delete this->gui; //das erstellte GWEN_Qt4_GUI wieder löschen

}

/*! \brief Gibt den Institut-Namen zur BLZ zurück
  */
QString aqb_banking::getInstituteFromBLZ(const QString &BLZ) const
{
	AB_BANKINFO *bankinfo;
	QString Institute = "NO INFORMATION";
	bankinfo = AB_Banking_GetBankInfo(this->ab, "de", "", BLZ.toUtf8());

	if (bankinfo) {
		Institute = QString::fromUtf8(AB_BankInfo_GetBankName(bankinfo));
		AB_BankInfo_free(bankinfo);
	}
	//wenn bankinfo == NULL bleibt Institute auf "NO INFORMATION"
	return Institute;
}

/**
 *  It loads the appropriate bank checker module and lets it check the information.
 *
 * Wenn alles OK ist wird true zurückgegeben, ansonsten false. In result wird
 * immer ein entsprechender Text der dem aufgetretenen Fehler entspricht
 * gespeichert.
 *
 * Parameters:
 * \param country: ISO country code ("de" for Germany, "at" for Austria etc)
 * \param branchId: optional branch id (not needed for "de")
 * \param bankId: bank id ("Bankleitzahl" for "de")
 * \param accountId: account id
 * \param result: descriptive msg of the result
 */
bool aqb_banking::checkAccount(const QString &country, const QString &branchId,
			       const QString &bankId, const QString &accountId,
			       QString &result) const
{
	AB_BANKINFO_CHECKRESULT res;
	res = AB_Banking_CheckAccount(this->ab,
				      country.toUtf8(), branchId.toUtf8(),
				      bankId.toUtf8(), accountId.toUtf8());
	switch (res) {
	case AB_BankInfoCheckResult_Ok:
		result = QObject::tr("OK");
		return true;
		break;
	case AB_BankInfoCheckResult_NotOk:
		result = QObject::tr("Nicht OK");
		break;
	case AB_BankInfoCheckResult_UnknownBank:
		result = QObject::tr("Bank unbekannt");
		break;
	case AB_BankInfoCheckResult_UnknownResult:
		result = QObject::tr("Ergebniss unbekannt");
		break;
	default:
		result = QObject::tr("Unbekannter Ergebnisstyp");
		break;
	}

	return false;
}

/**
  * @brief returns the path to the local user data dir
  *
  * This is normaly /home/USER/.aqbanking but could be any other directory
  * depending on the platform and compile settings.
  *
  */
QString aqb_banking::getUserDataDir() const
{
	GWEN_BUFFER *buf = GWEN_Buffer_new(NULL, 255, 0, 0);
	if (!buf) {
		qWarning() << Q_FUNC_INFO << "could not get a GWEN_BUFFER!"
			   << "GWEN_Buffer_new() returned NULL";
		return QString();
	}

	int ret = AB_Banking_GetUserDataDir(this->ab, buf);
	if (ret) {
		qWarning() << Q_FUNC_INFO << "AB_Banking_GetUserDataDir returned"
			   << ret << " - Aborting.";
		GWEN_Buffer_free(buf);
		return QString();
	}
	QString path = QString::fromUtf8(GWEN_Buffer_GetStart(buf));
	GWEN_Buffer_free(buf);

	return path;
}
