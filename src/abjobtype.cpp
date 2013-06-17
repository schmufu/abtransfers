#include <QObject>

#include "abjobtype.hpp"

AbJobType::AbJobType()
{
}

AbJobType::~AbJobType()
{
}

AbJobTypeCreateDatedTransfer::AbJobTypeCreateDatedTransfer()
{
}

const QString AbJobTypeCreateDatedTransfer::getJobTypeToQString() const
{
        return (QObject::tr("Terminüberweisung anlegen"));
}

AbJobTypeCreateStandingOrder::AbJobTypeCreateStandingOrder()
{
}

const QString AbJobTypeCreateStandingOrder::getJobTypeToQString() const
{
        return (QObject::tr("Dauerauftrag anlegen"));
}

AbJobTypeDebitNote::AbJobTypeDebitNote()
{
}

const QString AbJobTypeDebitNote::getJobTypeToQString() const
{
        return (QObject::tr("Lastschrift anlegen"));
}

AbJobTypeDeleteDatedTransfer::AbJobTypeDeleteDatedTransfer()
{
}

const QString AbJobTypeDeleteDatedTransfer::getJobTypeToQString() const
{
        return (QObject::tr("Terminüberweisung löschen"));
}

AbJobTypeDeleteStandingOrder::AbJobTypeDeleteStandingOrder()
{
}

const QString AbJobTypeDeleteStandingOrder::getJobTypeToQString() const
{
        return (QObject::tr("Dauerauftrag löschen"));
}

AbJobTypeEuTransfer::AbJobTypeEuTransfer()
{
}

const QString AbJobTypeEuTransfer::getJobTypeToQString() const
{
        return (QObject::tr("Internationale Überweisung"));
}

AbJobTypeGetBalance::AbJobTypeGetBalance()
{
}

const QString AbJobTypeGetBalance::getJobTypeToQString() const
{
        return (QObject::tr("Kontostand abfragen"));
}

AbJobTypeGetDatedTransfers::AbJobTypeGetDatedTransfers()
{
}

const QString AbJobTypeGetDatedTransfers::getJobTypeToQString() const
{
        return (QObject::tr("Terminüberweisungen abfragen"));
}

AbJobTypeGetStandingOrders::AbJobTypeGetStandingOrders()
{
}

const QString AbJobTypeGetStandingOrders::getJobTypeToQString() const
{
        return (QObject::tr("Daueraufträge abfragen"));
}

AbJobTypeGetTransactions::AbJobTypeGetTransactions()
{
}

const QString AbJobTypeGetTransactions::getJobTypeToQString() const
{
        return (QObject::tr("Buchungen abfragen"));
}

AbJobTypeInternalTransfer::AbJobTypeInternalTransfer()
{
}

const QString AbJobTypeInternalTransfer::getJobTypeToQString() const
{
        return (QObject::tr("Umbuchung durchführen"));
}

AbJobTypeLoadCellPhone::AbJobTypeLoadCellPhone()
{
}

const QString AbJobTypeLoadCellPhone::getJobTypeToQString() const
{
        return (QObject::tr("Handy Prepaid-Karte aufladen"));
}

AbJobTypeModifyDatedTransfer::AbJobTypeModifyDatedTransfer()
{
}

const QString AbJobTypeModifyDatedTransfer::getJobTypeToQString() const
{
        return (QObject::tr("Terminüberweisung ändern"));
}

AbJobTypeModifyStandingOrder::AbJobTypeModifyStandingOrder()
{
}

const QString AbJobTypeModifyStandingOrder::getJobTypeToQString() const
{
        return (QObject::tr("Dauerauftrag ändern"));
}

AbJobTypeSepaDebitNote::AbJobTypeSepaDebitNote()
{
}

const QString AbJobTypeSepaDebitNote::getJobTypeToQString() const
{
        return (QObject::tr("SEPA Lastschrift anlegen"));
}

AbJobTypeSepaTransfer::AbJobTypeSepaTransfer()
{
}

const QString AbJobTypeSepaTransfer::getJobTypeToQString() const
{
        return (QObject::tr("SEPA Überweisung"));
}

AbJobTypeTransfer::AbJobTypeTransfer()
{
}

const QString AbJobTypeTransfer::getJobTypeToQString() const
{
        return (QObject::tr("Überweisung durchführen"));
}

AbJobTypeUnknown::AbJobTypeUnknown()
{
}

const QString AbJobTypeUnknown::getJobTypeToQString() const
{
        return (QObject::tr("AqBanking Typ unbekannt"));
}
