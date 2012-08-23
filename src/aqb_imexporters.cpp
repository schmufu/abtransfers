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
 *	uses the imexporters from AqBanking and Gwenhywfar for exporting
 *	(perhaps later also importing) executed transactions.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

/** \todo Was darf während den funktionen wieder gelöscht werden und was
 *	  darf erst im destructor wieder freigegeben werden?
 *	  Momentan stürzt das Programm mit der Meldung "abtransfers: db.c:592:
 *	  GWEN_DB_FindVar: Assertion `n->children' failed." ab.
 */


#include "aqb_imexporters.h"

#include "QtCore/QDebug"
#include "globalvars.h"


//#include <aqbanking/banking.h>

//#include <aqbanking/imexporter.h>

//#include <aqbanking/banking_imex.h>

#include <aqbanking/abgui.h>
#include <aqbanking/dlg_importer.h>

/** \brief At instantiation this loads all im-/exporter information from
 *	   AqBanking
 *
 *
 */

aqb_imexporters::aqb_imexporters(QObject *parent) :
	QObject(parent)
{
	//init of private members
	this->pdl = NULL;
	this->plugins = new QList<aqb_iePlugin*>;

	//get the list of available im/exporters
	//(we must free this through GWEN_PluginDescription_List2_freeAll())
	pdl = AB_Banking_GetImExporterDescrs(banking->getAqBanking());

	qDebug() << Q_FUNC_INFO << "available imexpoerters:" << this->getSize();

	GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pdli;
	GWEN_PLUGIN_DESCRIPTION *pd = NULL;
	pdli = GWEN_PluginDescription_List2_First(pdl);
	if (pdli) {
		pd = GWEN_PluginDescription_List2Iterator_Data(pdli);
	}

	while (pd) {
		aqb_iePlugin *aqbPlugin = new aqb_iePlugin(pd);
		this->plugins->append(aqbPlugin);

		pd = GWEN_PluginDescription_List2Iterator_Next(pdli); //next in list
	}

	//free ListIterator
	GWEN_PluginDescription_List2Iterator_free(pdli);

}

aqb_imexporters::~aqb_imexporters()
{
	//delete all imexporter Descriptions and the "List2" itself
	GWEN_PluginDescription_List2_freeAll(this->pdl);

	while(!this->plugins->isEmpty()) {
		delete this->plugins->takeFirst();
	}
	delete this->plugins;


}


int aqb_imexporters::getSize() const
{
	Q_ASSERT(this->pdl);
	return GWEN_PluginDescription_List2_GetSize(pdl);
}






/*******************************************************************************
 * aqb_iePlugin
 ******************************************************************************/

aqb_iePlugin::aqb_iePlugin(GWEN_PLUGIN_DESCRIPTION *pd, QObject *parent) :
	QObject(parent)
{
	this->pd = pd;

	this->name = GWEN_PluginDescription_GetName(this->pd);
	this->type = GWEN_PluginDescription_GetType(this->pd);
	this->desc_short = GWEN_PluginDescription_GetShortDescr(this->pd);
	this->desc_long = GWEN_PluginDescription_GetLongDescr(this->pd);
	this->filename = GWEN_PluginDescription_GetFileName(this->pd);
	this->path = GWEN_PluginDescription_GetPath(this->pd);
	this->author = GWEN_PluginDescription_GetAuthor(this->pd);
	this->version = GWEN_PluginDescription_GetVersion(this->pd);

	this->profiles = new QList<aqb_ieProfile*>;

	this->loadProfiles();

}

aqb_iePlugin::~aqb_iePlugin()
{
	//delete all profile descriptions and the list
	while(!this->profiles->isEmpty()) {
		delete this->profiles->takeFirst();
	}

	delete this->profiles;
}

int aqb_iePlugin::loadProfiles()
{
	GWEN_DB_NODE *dbProfiles = AB_Banking_GetImExporterProfiles(banking->getAqBanking(),
								    this->name);
	GWEN_DB_NODE *n = GWEN_DB_GetFirstGroup(dbProfiles);
	while (n) {
		aqb_ieProfile *profile = new aqb_ieProfile(n);
		this->profiles->append(profile);

		n = GWEN_DB_GetNextGroup(n);
	}
	GWEN_DB_Group_free(dbProfiles);

	return this->profiles->size();
}







/*******************************************************************************
 * aqb_ieProfile
 ******************************************************************************/

aqb_ieProfile::aqb_ieProfile(GWEN_DB_NODE *n, QObject *parent) :
	QObject(parent)
{
	Q_ASSERT_X(GWEN_DB_IsGroup(n), "ieProfile", "ieProfile must be called with a dbGroup");

	this->dbnode = n;

	const char *n_name = GWEN_DB_GetCharValue(n, "name", 0, "notSet");
	const char *n_file = GWEN_DB_GetCharValue(n, "fileName", 0, "notSet");
	int n_global = GWEN_DB_GetIntValue(n, "isGlobal", 0, 2);
	qDebug() << n_name << n_file << n_global;

	this->names = new QStringList();
	GWEN_DB_NODE *nvars = GWEN_DB_GetFirstVar(this->dbnode);
	while (nvars) {
		const char *varname = GWEN_DB_VariableName(nvars);
		this->names->append(varname);

		nvars = GWEN_DB_GetNextVar(nvars);
	}

}

aqb_ieProfile::~aqb_ieProfile()
{
	//we must free the created stringlist
	delete this->names;

	//! \todo must we free the DB_NODE when it is no longer used?
}

/**
 *
 * returns the value of the variable \a varname, if the \a varname does not
 * exist in the profile or when the value is not set in the Profile, the
 * returned QString is empty.
 *
 */
QString aqb_ieProfile::getValue(const char *varname) const
{
	QString value;
	GWEN_DB_NODE_TYPE vartype = this->getType(varname);

	switch(vartype) {
	case GWEN_DB_NodeType_ValueChar:
		value = GWEN_DB_GetCharValue(this->dbnode, varname, 0, "");
		break;
	case GWEN_DB_NodeType_ValueInt:
		//value = QString("%1").arg(GWEN_DB_GetIntValue(n, varname, 0, -9));
		value = GWEN_DB_GetIntValue(this->dbnode, varname, 0, 0);
		break;
	default:
		value = "";
		break;
	}

	return value;
}

GWEN_DB_NODE_TYPE aqb_ieProfile::getType(const char *varname) const
{
	return GWEN_DB_GetVariableType(this->dbnode, varname);
}


