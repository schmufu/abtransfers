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

#include "pagewidgettests.h"
#include <QtGui/QLayout>
#include <QtCore/QDebug>

#include "../widgets/widgetaccountdata.h"
#include "../widgets/widgettextkey.h"



pageWidgetTests::pageWidgetTests(QWidget *parent) :
    QWidget(parent)
{
	widgetAccountData *accData = new widgetAccountData(this);
	accData->setAllowDropKnownRecipient(false);
	accData->setAllowDropAccount(true);

	QList<int> intlist;
	intlist << 51 << 53 << 54;
	widgetTextKey *textKey = new widgetTextKey(&intlist, this);
	textKey->setTextKey(54);

	this->purpose = new widgetPurpose(this);
	purpose->setPurpose("Nur Nen Test\nmit Zeilenumbruch\nUnd einer sehr langen Zeile die in Block2 bestimmt 2 mal umgebrochen werden muss");

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(accData);
	layout->addWidget(purpose);
	layout->addWidget(textKey);

	this->setLayout(layout);
	//this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	qDebug() << "PURPOSE: " << purpose->getPurpose();
}

pageWidgetTests::~pageWidgetTests()
{

}


QStringList pageWidgetTests::getPurpose()
{
	return this->purpose->getPurpose();
}
