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

#include "../widgets/widgetaccountdata.h"


pageWidgetTests::pageWidgetTests(QWidget *parent) :
    QWidget(parent)
{
	this->lineEdit1 = new widgetLineEditWithLabel("Label 1", "", Qt::AlignBottom, this);
	this->lineEdit2 = new widgetLineEditWithLabel("Label 2", "", Qt::AlignLeft, this);
	this->lineEdit3 = new widgetLineEditWithLabel("Label 3", "", Qt::AlignTop, this);
	this->lineEdit4 = new widgetLineEditWithLabel("Label 4", "", Qt::AlignRight, this);
	this->lineEdit3->layout()->setContentsMargins(18,18,18,18);
	this->lineEdit3->layout()->setSpacing(18);

	widgetAccountData *accData = new widgetAccountData(this);
	accData->setAllowDropKnownRecipient(false);
	accData->setAllowDropAccount(true);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(this->lineEdit1);
	layout->addWidget(this->lineEdit2);
	layout->addWidget(this->lineEdit3);
	layout->addWidget(this->lineEdit4);
	layout->addWidget(accData);

	this->setLayout(layout);
	//this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

pageWidgetTests::~pageWidgetTests()
{
	delete this->lineEdit1;
	delete this->lineEdit2;
	delete this->lineEdit3;
	delete this->lineEdit4;
}
