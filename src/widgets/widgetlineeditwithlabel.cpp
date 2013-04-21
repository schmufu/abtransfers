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
        this->m_lineEdit = new QLineEdit(editText, this);
        this->m_lineEdit->setMinimumHeight(25);
        this->m_label = new QLabel(labelText, this);
        this->m_label->setMinimumHeight(15);
	//layout anlegen und lineEdit sowie label ausrichten
	this->alignEditAndLabel(labelAt);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}


widgetLineEditWithLabel::~widgetLineEditWithLabel()
{
        delete this->m_lineEdit;
        delete this->m_label;
        delete this->m_mainLayout;

        this->m_lineEdit = NULL;
        this->m_label = NULL;
        this->m_mainLayout = NULL;
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
                this->m_mainLayout = new QVBoxLayout();
		if (align & Qt::AlignTop) {
                        this->m_mainLayout->addWidget(this->m_label);
                        this->m_mainLayout->addWidget(this->m_lineEdit);
		} else {
                        this->m_mainLayout->addWidget(this->m_lineEdit);
                        this->m_mainLayout->addWidget(this->m_label);
		}
                this->setLayout(this->m_mainLayout);
		return;
	}

	if ((align & Qt::AlignLeft) || (align & Qt::AlignRight)) {
		//Wir benötigen ein horizontales Layout
                this->m_mainLayout = new QHBoxLayout();
		if (align & Qt::AlignLeft) {
                        this->m_mainLayout->addWidget(this->m_label);
                        this->m_mainLayout->addWidget(this->m_lineEdit);
		} else {
                        this->m_mainLayout->addWidget(this->m_lineEdit);
                        this->m_mainLayout->addWidget(this->m_label);
		}
                this->setLayout(this->m_mainLayout);
		return;
	}
}
