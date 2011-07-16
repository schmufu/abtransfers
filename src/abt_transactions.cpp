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

#include "abt_transactions.h"
#include "globalvars.h"

#include <QDebug>

trans_StandingOrder::trans_StandingOrder(int id)
{
	this->id = id;

	this->aqb_transaction = AB_Transaction_new();

	if (this->id != 0) {
		load(id);
	}
}

void trans_StandingOrder::load(int id)
{
	//! \todo Angegebene ID aus der Datei laden
}

void trans_StandingOrder::save()
{
	//! \todo Alle daten in der Datei speichern

	if (this->id == 0) {
		qWarning() << "Speichern nicht möglich! Keine ID für DA gesetzt!";
		return; //Abbruch
	}

	QSettings *d;
	QString Filename;
	Filename.append(settings->getDataDir());
	Filename.append("Dauerauftraege.ini");
//	Filename.append(this->getLocalAccountNumber());
//	Filename.append("_");
//	Filename.append(this->getLocalBankCode());
//	Filename.append(".ini");

	d = new QSettings(Filename, QSettings::IniFormat);

	d->beginGroup(QString("%1").arg(this->id));
	d->setValue("LocalCountry", this->getLocalCountry());
	d->setValue("LocalBankCode", this->getLocalBankCode() );
	d->setValue("LocalBranchId", this->getLocalBranchId() );
	d->setValue("LocalAccountNumber", this->getLocalAccountNumber() );
	d->setValue("LocalSuffix", this->getLocalSuffix() );
	d->setValue("LocalIban", this->getLocalIban() );
	d->setValue("LocalName", this->getLocalName() );
	d->setValue("LocalBic", this->getLocalBic() );
	d->setValue("RemoteCountry", this->getRemoteCountry() );
	d->setValue("RemoteBankName", this->getRemoteBankName() );
	d->setValue("RemoteBankLocation", this->getRemoteBankLocation() );
	d->setValue("RemoteBankCode", this->getRemoteBankCode() );
	d->setValue("RemoteBranchId", this->getRemoteBranchId() );
	d->setValue("RemoteAccountNumber", this->getRemoteAccountNumber() );
	d->setValue("RemoteSuffix", this->getRemoteSuffix() );
	d->setValue("RemoteIban", this->getRemoteIban() );
	d->setValue("RemoteName", this->getRemoteName() );
	d->setValue("RemoteBic", this->getRemoteBic() );
	d->setValue("ValutaDate", this->getValutaDate() );
	d->setValue("Date", this->getDate() );
	d->setValue("Value", this->getValue() );
	d->setValue("TextKey", this->getTextKey() );
	d->setValue("TextKeyExt", this->getTextKeyExt() );
	d->setValue("TransactionKey", this->getTransactionKey() );
	d->setValue("CustomerReference", this->getCustomerReference() );
	d->setValue("BankReference", this->getBankReference() );
	d->setValue("EntToEndReference", this->getEndToEndReference() );
	d->setValue("MandateReference", this->getMandateReference() );
	d->setValue("CretitorIdentifier", this->getCreditorIdentifier() );
	d->setValue("OriginatorIdentifier", this->getOriginatorIdentifier() );
	d->setValue("TransactionCode", this->getTransactionCode() );
	d->setValue("TransactionText", this->getTransactionText() );
	d->setValue("Primanota", this->getPrimanota() );
	d->setValue("FiId", this->getFiId() );
	d->setValue("Purpose", this->getPurpose() );
	d->setValue("Category", this->getCategory() );
	d->setValue("Period", this->getPeriod() );
	d->setValue("Cycle", this->getCycle() );
	d->setValue("ExecutionDay", this->getExecutionDay() );
	d->setValue("FirstExecutionDate", this->getFirstExecutionDate() );
	d->setValue("LastExecutionDate", this->getLastExecutionDate() );
	d->setValue("NextExecutionDate", this->getNextExecutionDate() );
	d->setValue("Type", this->getType() );
	d->setValue("SubType", this->getSubType() );
	d->setValue("Status", this->getStatus() );
	d->setValue("Charge", this->getCharge() );
	d->setValue("RemoteAddrStreet", this->getRemoteAddrStreet() );
	d->setValue("RemoteAddrZipcode", this->getRemoteAddrZipcode() );
	d->setValue("RemoteAddrCity", this->getRemoteAddrCity() );
	d->setValue("RemotePhone", this->getRemotePhone() );
	d->setValue("UnitId", this->getUnitId() );
	d->setValue("UnitIdNameSpace", this->getUnitIdNameSpace() );
	d->setValue("Units", this->getUnits() );
	d->setValue("UnitPrice", this->getUnitPrice() );
	d->setValue("Commission", this->getCommission() );
	d->setValue("UniqueId", this->getUniqueId() );
	d->setValue("IdForApplication", this->getIdForApplication() );
	d->setValue("GroupId", this->getGroupId() );
	d->setValue("Fees", this->getFees() );

	d->endGroup();
	d->setValue("Main/test1/test2/test3", "TestWert");

	delete d;

}


trans_DatedTransfer::trans_DatedTransfer()
{

}


trans_SingleTransfer::trans_SingleTransfer()
{

}


trans_SingleDebitNote::trans_SingleDebitNote()
{

}


trans_EuTransfer::trans_EuTransfer()
{

}


trans_InternalTransfer::trans_InternalTransfer()
{

}


trans_SepaTransfer::trans_SepaTransfer()
{

}




