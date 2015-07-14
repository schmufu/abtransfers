/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
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

	//The GUI must be created here, otherwise an error could occur.
	//(happened ones, at the time were AqBanking data was replaced with
	//data from another PC)
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

	AB_Banking_GetVersion(&this->major, &this->minor, &this->patch, &this->build);
	this->aqbanking_version = QString::fromUtf8("%1.%2.%3.%4")
					.arg(this->major)
					.arg(this->minor)
					.arg(this->patch)
					.arg(this->build);
	qDebug() << "AqBanking successfully initialized."
		 << "(Version:" << this->aqbanking_version << ")";

	/* This function loads the settings file of AqBanking so the users and
	 * accounts become available after this function successfully returns.
	 */
	rv = AB_Banking_OnlineInit(this->ab);
	if (rv) {
		fprintf(stderr, "Error on init of online modules (%d)\n", rv);
		AB_Banking_Fini(this->ab);
		return;
	}

	//ensure that modifications to GWEN-Dialogs are saved
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

	//no more dialog settings needs to be saved
	AB_Gui_Unextend(this->gui->getCInterface());

	AB_Banking_free(ab);

	qDebug() << "AqBanking successfully deinitialized";


	/* Now we need to do something tricky to workaround a possible bug in GWEN_GUI
	 *
	 * see http://devel.aqbanking.de/svn/gwenhywfar/trunk/src/gui/gui.c for
	 * the GWEN_Gui_* functions referenced here.
	 *
	 * this->gui is a QT4_Gui* which handles all then GWEN_Gui_* C functions
	 * for us. It should be enough to write "delete this->gui;" but its not!
	 *
	 * GWEN_GUI uses an internal static var "GWEN_GUI *gwenhywfar_gui" that
	 * points to the current used GWEN_GUI Object.
	 * QT4_Gui uses GWEN_Gui_new() to create a new GWEN_GUI instance and the
	 * GWEN_GUI internal var "gwenhywfar_gui" is set in our constructor by
	 * GWEN_Gui_SetGui() [see abgui.h from aqbanking for the the example
	 * code that is used here].
	 *
	 * Until now everything is fine and all is running as expected!
	 *
	 * The trouble comes into account when the objects should be deleted and
	 * especially when the environment variable GWEN_LOGLEVEL is set.
	 *
	 * What happens:
	 * After a "delete this->gui;" the GWEN_GUI internal variable
	 * *gwenhywfar_gui is still set to a no longer existing GWEN_GUI (not set
	 * to "NULL" after freeing) [could be verified with GWEN_Gui_GetGui()].
	 * When the debug function from gwen wants to display a message the gui
	 * is found and it tries to use it, this unexpectedly finishes the
	 * program.
	 *
	 * What we do now:
	 * With GWEN_Gui_Attach the reference counter of the GWEN_GUI is
	 * increased and therefore the "delete this->gui;" only decreases
	 * the reference counter and the GWEN_GUI is finally deleted and set
	 * to NULL with GWEN_Gui_SetGui(NULL);
	 */

	if (GWEN_Gui_GetGui() != this->gui->getCInterface()) {
		//gui used by gwen is not ours, we simple delete our gui
		delete this->gui;
		this->gui = NULL;
	} else {
		//increase the refCount of the gwen interal used GWEN_GUI* pointer
		GWEN_Gui_Attach(this->gui->getCInterface());
		//delete the created QT4_Gui (decreases only the internal refCount
		//but not frees the internal GWEN_GUI*)
		delete this->gui;
		this->gui = NULL;
		//Set the used GWEN_GUI to NULL. This decreases the internal
		//refCount and frees it (if refCount == 0) and also sets the
		//internal used gwenhywfar_gui* to NULL ;)
		GWEN_Gui_SetGui(NULL);

		if (GWEN_Gui_GetGui()) {
			qWarning() << Q_FUNC_INFO
				   << "gwenhywfar internal GWEN_GUI is not NULL!"
				   << "this will probably cause a crash when the"
				   << "GWEN_LOGLEVEL environment variable is set.";
		}
	}
}

//public
/** returns true if the last date of standing order is supported to be set
 *
 * AqBanking does not support setting the last date for a standing order,
 * therefore this function should be used to check if setting the last date
 * is supported or not.
 */
bool aqb_banking::isLastDateSupported() const
{
	/** \todo: Adjust the version label when AqBanking supports setting
	 *         the last date.
	 */
	return !(this->aqbanking_version < QString::fromUtf8("9.9.9"));
}


/*! \brief Gibt den Institut-Namen zur BLZ zurück
  */
QString aqb_banking::getInstituteFromBLZ(const QString &BLZ) const
{
	AB_BANKINFO *bankinfo;
	QString Institute = QString::fromUtf8("NO INFORMATION");
	bankinfo = AB_Banking_GetBankInfo(this->ab, "de", "", BLZ.toStdString().c_str());

	if (bankinfo) {
		Institute = QString::fromUtf8(AB_BankInfo_GetBankName(bankinfo));
		AB_BankInfo_free(bankinfo);
	}
	//wenn bankinfo == NULL bleibt Institute auf "NO INFORMATION"
	return Institute;
}

//! @todo: implement getInstituteFromBIC - perhaps possible with libktoblzcheck.
///*! \brief Gibt den Institut-Namen zur BIC zurück
//  */
//QString aqb_banking::getInstituteFromBIC(const QString &BIC) const
//{
//	AB_BANKINFO *bankinfo;
//	QString Institute = "NO INFORMATION";
//	bankinfo = AB_Banking_GetBankInfo(this->ab, "de", "", BIC.toUtf8());

//	if (bankinfo) {
//		Institute = QString::fromUtf8(AB_BankInfo_GetBankName(bankinfo));
//		AB_BankInfo_free(bankinfo);
//	}
//	//wenn bankinfo == NULL bleibt Institute auf "NO INFORMATION"
//	return Institute;
//}


/**
 * It loads the appropriate bank checker module and lets it check the
 * information.
 *
 * If the @a accountId and @a bankId could exist true is returned otherwise
 * false. The @a result string always contains a textual expression of the
 * result.
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
				      country.toStdString().c_str(),
				      branchId.toStdString().c_str(),
				      bankId.toStdString().c_str(),
				      accountId.toStdString().c_str());
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
		result = QObject::tr("Ergebnis unbekannt");
		break;
	default:
		result = QObject::tr("Unbekannter Ergebnistyp");
		break;
	}

	return false;
}


/**
 * see http://www.pruefziffernberechnung.de/I/IBAN.shtml for further details.
 *
 * @param iban the IBAN to check
 * @param result descriptive msg of the result
 * @returns false if the @a iban is not correct
 * @returns true if the @a iban is correct
 */
bool aqb_banking::checkIBAN(const QString &iban, QString &result) const
{

	//The first two chars must be characters
	for(int i=0; i<2; i++) {
		if ( ! iban.at(i).isLetter()) {
			result = QObject::tr("Die ersten 2 Zeichen müssen "
					     "Buchstaben sein. (Ländercode)");
			return false;
		}
	}

	QString resStr = QString();
	QString countryCode = iban.left(2);
	QString pruefziffer = iban.mid(2,2);
	QString blz = iban.mid(4,8);
	QString ktonr = iban.mid(12);

	if (countryCode.toUpper() == QString::fromUtf8("DE")) {
		//the IBAN is for a german bank account, so we can also check
		//the account-number and bankcode
		if (!this->checkAccount(QString::fromUtf8("de"), QString(), blz, ktonr, resStr)) {
			result = QObject::tr("Überprüfung von Kontonummer und "
					     "Bankleitzahl für Deutsches Konto "
					     "fehlerhaft: %1").arg(resStr);
			return false;
		}
	}

	//convert the country code and the check digit for checksum calculation.
	//These are the first 4 places of the iban.
	QString pp_substitution;
	for(int i=0; i<4; i++) {
		if (iban.at(i).isLetter()) {
			//A=10, B=11, C=12 etc.
			int value = iban.at(i).toUpper().toLatin1()-55;
			pp_substitution.append(QString::number(value));
		} else {
			pp_substitution.append(iban.at(i));
		}
	}

	//convert the rest of the iban to only digits
	//(could also contain characters)
	QString sIban;
	for(int i=4; i<iban.size(); i++) {
		if (iban.at(i).isLetter()) {
			int value = iban.at(i).toUpper().toLatin1()-55;
			sIban.append(QString::number(value));
		} else {
			sIban.append(iban.at(i));
		}
	}

	//append the substituted "check digit" to the end of the IBAN
	sIban.append(pp_substitution);

	//the number is too large to be calculated at once (~36 digits).
	//so we calculate the checksum step by step (with max. 9 digits at
	//a time, so that it fits into 32bit unsigned integers)
	//[later this could be expanded to 18 for 64bit integers]
	bool convOK = false;
	int modulus = 0;
	quint64 value = 0;
	QString subIBAN;

	while(!sIban.isEmpty()) {
		if (modulus == 0) {
			subIBAN = sIban.left(9);
			sIban.remove(0,9);
		} else {
			QString mod = QString::number(modulus);
			subIBAN = sIban.left(9 - mod.length());
			subIBAN.prepend(mod);
			sIban.remove(0, 9 - mod.length());
		}
		value = subIBAN.toULongLong(&convOK);
		if (!convOK) {
			qWarning() << Q_FUNC_INFO << "conversion failed! ("
				   << subIBAN << ") - IBAN could not be checked.";
			result = QObject::tr("Umwandlung fehlgeschlagen - "
					     "Überprüfung konnte nicht "
					     "durchgeführt werden");
			return false;
		}
		modulus = value % 97;
	}

	if (modulus == 1) { //for a correct IBAN the modulus must be 1
		result = QObject::tr("OK");
		return true;
	} else { //incorrect IBAN
		result = QObject::tr("Prüfsumme fehlerhaft (Modulus = %1)")
			 .arg(modulus);
		return false;
	}
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
