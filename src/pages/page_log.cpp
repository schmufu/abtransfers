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

#include "page_log.h"
#include "ui_page_log.h"

page_log::page_log(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::page_log)
{
	ui->setupUi(this);
}

page_log::~page_log()
{
	delete ui;
}

void page_log::changeEvent(QEvent *e)
{
	QFrame::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void page_log::setLogText(const QStringList *strList)
{
	this->ui->textEdit->setHtml(strList->join(QString::fromUtf8("<br />")));
}

void page_log::appendLogText(const QString &str)
{
	this->ui->textEdit->append(str);
}
