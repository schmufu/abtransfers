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

#include "aqb_imexporters.h"

#include "QtCore/QDebug"
#include "globalvars.h"

#include <aqbanking/abgui.h>
#include <aqbanking/dlg_importer.h>




aqb_imexporters::aqb_imexporters(QObject *parent) :
	QObject(parent)
{
	//init of private members
	this->pdl = NULL;
	this->plugins = new QList<aqb_iePlugin*>;

	//get the list of available im/exporters
	//(we must free this through GWEN_PluginDescription_List2_freeAll())
	this->pdl = AB_Banking_GetImExporterDescrs(banking->getAqBanking());

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
	while(!this->plugins->isEmpty()) {
		delete this->plugins->takeFirst();
	}
	delete this->plugins;

	//delete all imexporter Descriptions and the "List2" itself
	GWEN_PluginDescription_List2_freeAll(this->pdl);
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
	this->pd = GWEN_PluginDescription_dup(pd); //store a local copy

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

	GWEN_PluginDescription_free(this->pd);
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

const aqb_iePlugin *aqb_imexporters::getPluginByName(QString &name) const
{
	foreach(const aqb_iePlugin *plugin, *this->plugins) {
		if (plugin->getName() == name) {
			return plugin;
		}
	}

	return NULL; //no plugin found
}

const aqb_iePlugin *aqb_imexporters::getPluginByFilename(QString &filename) const
{
	foreach(const aqb_iePlugin *plugin, *this->plugins) {
		if (plugin->getFilename() == filename) {
			return plugin;
		}
	}

	return NULL; //no plugin found
}




/*******************************************************************************
 * aqb_ieProfile
 ******************************************************************************/

aqb_ieProfile::aqb_ieProfile(GWEN_DB_NODE *dbn, QObject *parent) :
	QObject(parent)
{
	Q_ASSERT_X(GWEN_DB_IsGroup(dbn), "aqb_ieProfile", "ieProfile must be called with a dbGroup");

	this->dbnode = GWEN_DB_Group_dup(dbn); //store a local copy

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

	//we must free the DB_NODE when it is no longer used
	//(duplicated in constructor)
	GWEN_DB_Group_free(this->dbnode);
}

/**
 *
 * returns the value of the variable @a varname, if the @a varname does not
 * exist in the profile or when the value is not set in the Profile, the
 * returned QVariant is invalid (or empty).
 *
 */
QVariant aqb_ieProfile::getValue(const char *varname, int idx /* = 0 */) const
{
	QVariant value = QVariant::Invalid;
	if (!this->names->contains(varname)) {
		qWarning() << Q_FUNC_INFO << this << "does not contain a var" << varname;
		return value;
	}

	GWEN_DB_NODE_TYPE vartype = this->getType(varname);

	switch(vartype) {
	case GWEN_DB_NodeType_ValueChar:
		value = GWEN_DB_GetCharValue(this->dbnode, varname, idx, "");
		break;
	case GWEN_DB_NodeType_ValueInt:
		value = GWEN_DB_GetIntValue(this->dbnode, varname, idx, 0);
		break;
	case GWEN_DB_NodeType_ValuePtr:
		value.setValue<void*>(GWEN_DB_GetPtrValue(this->dbnode, varname,
							  idx, NULL));
		break;
	//not used yet, how did we handle this if it must be used?
	//case GWEN_DB_NodeType_ValueBin:
	//	unsigned int retValueSize;
	//	value = (GWEN_DB_GetBinValue(this->dbnode, varname, idx, NULL,
	//				     0, &retValueSize));
	//	break;
	default:
		break;
	}

	return value;
}

GWEN_DB_NODE_TYPE aqb_ieProfile::getType(const char *varname) const
{
	return GWEN_DB_GetVariableType(this->dbnode, varname);
}


