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

#include "widgetpurpose.h"

#include <QtCore/QDebug>
#include <QtGui/QLayout>

widgetPurpose::widgetPurpose(QWidget *parent) :
	QWidget(parent)
{
	this->plainEdit = new QPlainTextEdit(this);
	this->statusString = new QString(tr("(max %1 Zeilen, a %2 Zeichen) [%3 Zeichen übrig]"));
	this->statusLabel = new QLabel(this);
	QFont labelFont(this->statusLabel->font());
	labelFont.setPointSize(7);
	this->statusLabel->setFont(labelFont);
	this->plainEdit->setReadOnly(false);
	this->plainEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


	this->maxLength = 27;
	this->maxLines = 4;
	this->updateStatusLabel();
	this->plainEdit->document()->setUseDesignMetrics(true);
	this->plainEdit->document()->setTextWidth(10);
	//this->plainEdit->document()->setPageSize();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(this->plainEdit);
	layout->addWidget(this->statusLabel);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(this->plainEdit, SIGNAL(textChanged()),
		this, SLOT(plainTextEdit_TextChanged()));
}

widgetPurpose::~widgetPurpose()
{
	delete this->plainEdit;
	delete this->statusString;
	delete this->statusLabel;
	qDebug() << this << "deleted";
}

//private
void widgetPurpose::updateStatusLabel()
{
	this->statusLabel->setText(
			this->statusString->arg(
				this->maxLines).arg(
				this->maxLength).arg(
				(this->maxLines*this->maxLength)-
				this->plainEdit->toPlainText().length()));

}

//public
QStringList widgetPurpose::getPurpose() const
{
	return this->plainEdit->toPlainText().split("\n", QString::SkipEmptyParts, Qt::CaseSensitive);
}

//private slot
void widgetPurpose::plainTextEdit_TextChanged()
{
	qDebug() << this << "PlainTextLength:" << this->plainEdit->toPlainText().length();
	qDebug() << this << "Blocks:" << this->plainEdit->document()->blockCount();
	this->updateStatusLabel();
}

//public slot
void widgetPurpose::setPurpose(const QString &text)
{
	this->plainEdit->setPlainText(text);
}

//public slot
void widgetPurpose::setPurpose(const QStringList &text)
{
	this->setPurpose(text.join("\n"));
}

//public slot
void widgetPurpose::setLimitMaxLen(int maxLen)
{
	this->maxLength = maxLen;
}

//public slot
void widgetPurpose::setLimitMaxLines(int lines)
{
	this->maxLines = lines;
}

//public slot
void widgetPurpose::setLimitAllowChange(bool b)
{
	this->plainEdit->setReadOnly(!b);
}

//public slot
void widgetPurpose::clearAll()
{
	this->plainEdit->clear();
}
