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

#include "widgetlineeditwithlabel.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QDebug>

widgetLineEditWithLabel::widgetLineEditWithLabel(const QString &labelText,
						 const QString &editText,
						 Qt::Alignment labelAt,
						 QWidget *parent) :
    QWidget(parent)
{
	this->lineEdit = new QLineEdit(editText, this);
	this->label = new QLabel(labelText, this);
	//layout anlegen und lineEdit sowie label ausrichten
	this->alignEditAndLabel(labelAt);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}


widgetLineEditWithLabel::~widgetLineEditWithLabel()
{
	delete this->lineEdit;
	delete this->label;
	delete this->mainLayout;
}

/*! erstellt das mainLayout diese Widgets und ordnet das Label entsprechend
 *  der Vorgabe an.
 */
void widgetLineEditWithLabel::alignEditAndLabel(Qt::Alignment align)
{
	//Prüfen ob nur 1 Alignment vorkommt!
	if ( ! ( ((align ^ Qt::AlignTop) == 0) || ((align ^ Qt::AlignBottom) == 0) ||
		 ((align ^ Qt::AlignLeft) == 0) || ((align ^ Qt::AlignRight) == 0)
	       )
	   ) { //Alignment Angabe wird nicht unterstüzt, default setzen;
		qWarning() << this << "ERROR: alignment of labelAt not"
				<< "supported, using default Qt::AlignTop!";
		align = Qt::AlignTop;
	}

	if ((align & Qt::AlignTop) || (align & Qt::AlignBottom)) {
		//Wir benötigen ein vertikales Layout
		this->mainLayout = new QVBoxLayout();
		if (align & Qt::AlignTop) {
			this->mainLayout->addWidget(this->label);
			this->mainLayout->addWidget(this->lineEdit);
		} else {
			this->mainLayout->addWidget(this->lineEdit);
			this->mainLayout->addWidget(this->label);
		}
		this->setLayout(this->mainLayout);
		return;
	}

	if ((align & Qt::AlignLeft) || (align & Qt::AlignRight)) {
		//Wir benötigen ein horizontales Layout
		this->mainLayout = new QHBoxLayout();
		if (align & Qt::AlignLeft) {
			this->mainLayout->addWidget(this->label);
			this->mainLayout->addWidget(this->lineEdit);
		} else {
			this->mainLayout->addWidget(this->lineEdit);
			this->mainLayout->addWidget(this->label);
		}
		this->setLayout(this->mainLayout);
		return;
	}
}






