/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
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


#include "historyitemwidget.h"
#include "ui_historyitemwidget.h"

#include <QDebug>
#include <QMouseEvent>

#include "../abt_jobinfo.h"
#include "../abt_transaction_base.h"
#include "../abt_conv.h"

historyItemWidget::historyItemWidget(QFrame *parent, abt_jobInfo *ji) :
	QFrame(parent),
	ui(new Ui::historyItemWidget)
{
	ui->setupUi(this);

	if (ji) {
		this->setData(ji);
	}

}

historyItemWidget::~historyItemWidget()
{
	delete ui;
}

void historyItemWidget::setData(const abt_jobInfo *ji)
{
	this->ui->label_date->setText(ji->getTransaction()->getDate().toString(Qt::SystemLocaleShortDate));
	this->ui->label_type->setText(ji->getType());
	this->ui->label_status->setText(ji->getStatus());
	this->ui->label_recipient->setText(ji->getTransaction()->getRemoteName().at(0));
	QString valueText = QString ("%1 EUR").arg(abt_conv::ABValueToString(ji->getTransaction()->getValue(), true));
	this->ui->label_value->setText(valueText);
}

void historyItemWidget::enterEvent(QEvent* /* event */)
{
	if (this->backgroundRole() == QPalette::Highlight)
		return;

	this->setBackgroundRole(QPalette::Button);
}

void historyItemWidget::leaveEvent(QEvent* /* event */)
{
	if (this->backgroundRole() != QPalette::Highlight)
		this->setBackgroundRole(QPalette::Background);



}

void historyItemWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		if (this->minimumHeight() == 70) {
			this->setMinimumHeight(25);
		} else {
			this->setMinimumHeight(70);
		}
	} else {
		QWidget::mousePressEvent(event);
	}
}

