/******************************************************************************
 * Copyright (C) 2012-2013 Patrick Wacker
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

#include <aqbanking/abgui.h>
#include <aqbanking/dlg_importer.h>




aqb_imexporters::aqb_imexporters(AB_BANKING* ab, QObject *parent) :
	QObject(parent)
{
	//init of private members
	this->pdl = NULL;
	this->plugins = NULL;

	this->m_ab = ab;

	this->loadAll(); //creates the QList and loads all Plugins from AqBanking
}

aqb_imexporters::~aqb_imexporters()
{
	this->freeAll();
}

//private
void aqb_imexporters::freeAll()
{
	while(!this->plugins->isEmpty()) {
		delete this->plugins->takeFirst();
	}
	delete this->plugins;
	this->plugins = NULL;

	//delete all imexporter Descriptions and the "List2" itself
	GWEN_PluginDescription_List2_freeAll(this->pdl);
}

void aqb_imexporters::loadAll()
{
	Q_ASSERT_X(this->plugins == NULL, "loadAll()",
		   "plugins must be NULL before calling loadAll()");

	//we create the plugin list
	this->plugins = new QList<aqb_iePlugin*>;

	//get the list of available im/exporters
	//(we must free this through GWEN_PluginDescription_List2_freeAll())
	this->pdl = AB_Banking_GetImExporterDescrs(this->m_ab);

	GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pdli;
	GWEN_PLUGIN_DESCRIPTION *pd = NULL;
	pdli = GWEN_PluginDescription_List2_First(this->pdl);
	if (pdli) {
		pd = GWEN_PluginDescription_List2Iterator_Data(pdli);
	}

	while (pd) {
		aqb_iePlugin *aqbPlugin = new aqb_iePlugin(this->m_ab, pd);
		this->plugins->append(aqbPlugin);

		pd = GWEN_PluginDescription_List2Iterator_Next(pdli); //next in list
	}

	//free ListIterator
	GWEN_PluginDescription_List2Iterator_free(pdli);

	emit imexportersLoaded();
}


int aqb_imexporters::getSize() const
{
	Q_ASSERT(this->pdl);
	return GWEN_PluginDescription_List2_GetSize(pdl);
}


//public Slot
void aqb_imexporters::reloadImExporterData()
{
	this->freeAll();
	this->loadAll();
}



/*******************************************************************************
 * aqb_iePlugin
 ******************************************************************************/

aqb_iePlugin::aqb_iePlugin(AB_BANKING *ab, GWEN_PLUGIN_DESCRIPTION *pd, QObject *parent) :
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

	this->loadProfiles(ab);

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

int aqb_iePlugin::loadProfiles(AB_BANKING* ab)
{
	GWEN_DB_NODE *dbProfiles = AB_Banking_GetImExporterProfiles(ab,
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

const aqb_iePlugin *aqb_imexporters::getPluginByName(const QString &name) const
{
	if (name.isEmpty() || name.isNull())
		return NULL; //a plugin with no name cant be available

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

/**
 * @brief shows a edit dialog from AqBanking and saves the changes to the profile
 *
 * @param dbProfile: a pointer to the GWEN_DB_NODE which handles the profile
 * @param pluginName: the name of the plugin the profile belongs to
 * @param filename: the filename for storing the profile data
 *
 * @returns 0: success
 * @returns 1: user clicked cancel at the dialog
 * @returns -1: no EditProfileDialog could be retrieved from AqBanking
 * @returns -2: something went wrong at the dialog execution
 * @returns -3: error at saving the changed profile
 */
//public
int aqb_imexporters::editProfileWithAqbDialog(GWEN_DB_NODE *dbProfile, const char *pluginName, const char *filename) const
{
	int ret; //used to store the return values of AB_xxx functions
	GWEN_DIALOG *pDlg = NULL;

	//AqBanking remains the owner of 'ie', so we must not free it!
	AB_IMEXPORTER *ie = AB_Banking_GetImExporter(this->m_ab,
						     pluginName);

	ret = AB_ImExporter_GetEditProfileDialog(ie, dbProfile, filename, &pDlg);
	if (pDlg == NULL || ret != 0) {
		qWarning() << Q_FUNC_INFO << "AB_ImExporter_GetEditProfileDialog()"
			   << "returned" << ret;
		return -1;
	}


	//get the dialog id and execute it
	uint32_t guiid = GWEN_Dialog_GetGuiId(pDlg);
	ret = GWEN_Gui_ExecDialog(pDlg, guiid);

	GWEN_Dialog_free(pDlg); //free the dialog after execution

	if (ret < 0) { //ret <0: error code, 0: aborted, 1: accepted
		qDebug() << Q_FUNC_INFO << "return of GWEN_Gui_ExecDialog() is:" << ret;
		return -2;
	}

	if (ret == 0) {
		//user canceled the dialog
		return 1;
	}


	//User did not cancel the dialog, save profile
	ret = AB_Banking_SaveLocalImExporterProfile(this->m_ab,
						    pluginName, dbProfile,
						    filename);

	if (ret != 0) {
		qDebug() << Q_FUNC_INFO << "return of "
			 << "AB_Banking_SaveLocalImExporterProfile() is:" << ret;
		return -3;
	}

	return 0; //everything went fine
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


