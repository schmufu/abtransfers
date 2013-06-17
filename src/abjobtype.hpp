#ifndef ABJOBTYPE_HPP
#define ABJOBTYPE_HPP

#include <QString>

class AbJobType
{
public:
        AbJobType();
        virtual ~AbJobType();
        //virtual void setMyTransactionFromJob() = 0;
        //virtual void supportedByAbtransfers() = 0;
        virtual const QString getJobTypeToQString() const = 0;
};

class AbJobTypeCreateDatedTransfer : public AbJobType
{
public:
        AbJobTypeCreateDatedTransfer();
        //~AbJobTypeCreateDatedTransfer();
        const QString getJobTypeToQString() const;
};

class AbJobTypeCreateStandingOrder : public AbJobType
{
public:
      AbJobTypeCreateStandingOrder();
      //~AbJobTypeCreateStandingOrder();
      const QString getJobTypeToQString() const;
};

class AbJobTypeDebitNote : public AbJobType
{
public:
       AbJobTypeDebitNote();
       //~AbJobTypeDebitNote();
       const QString getJobTypeToQString() const;
};

class AbJobTypeDeleteDatedTransfer : public AbJobType
{
public:
       AbJobTypeDeleteDatedTransfer();
       //~AbJobTypeDeleteDatedTransfer();
       const QString getJobTypeToQString() const;
};

class AbJobTypeDeleteStandingOrder : public AbJobType
{
public:
       AbJobTypeDeleteStandingOrder();
       //~AbJobTypeDeleteStandingOrder();
       const QString getJobTypeToQString() const;
};

class AbJobTypeEuTransfer : public AbJobType
{
public:
       AbJobTypeEuTransfer();
       //~AbJobTypeEuTransfer();
       const QString getJobTypeToQString() const;
};

class AbJobTypeGetBalance : public AbJobType
{
public:
       AbJobTypeGetBalance();
       //~AbJobTypeGetBalance();
       const QString getJobTypeToQString() const;
};

class AbJobTypeGetDatedTransfers : public AbJobType
{
public:
       AbJobTypeGetDatedTransfers();
       const QString getJobTypeToQString() const;
};

class AbJobTypeGetStandingOrders : public AbJobType
{
public:
       AbJobTypeGetStandingOrders();
       const QString getJobTypeToQString() const;
};

class AbJobTypeGetTransactions : public AbJobType
{
public:
       AbJobTypeGetTransactions();
       const QString getJobTypeToQString() const;
};

class AbJobTypeInternalTransfer : public AbJobType
{
public:
       AbJobTypeInternalTransfer();
       const QString getJobTypeToQString() const;
};

class AbJobTypeLoadCellPhone : public AbJobType
{
public:
       AbJobTypeLoadCellPhone();
       const QString getJobTypeToQString() const;
};

class AbJobTypeModifyDatedTransfer : public AbJobType
{
public:
       AbJobTypeModifyDatedTransfer();
       const QString getJobTypeToQString() const;
};

class AbJobTypeModifyStandingOrder : public AbJobType
{
public:
       AbJobTypeModifyStandingOrder();
       const QString getJobTypeToQString() const;
};

class AbJobTypeSepaDebitNote : public AbJobType
{
public:
       AbJobTypeSepaDebitNote();
       const QString getJobTypeToQString() const;
};

class AbJobTypeSepaTransfer : public AbJobType
{
public:
       AbJobTypeSepaTransfer();
       const QString getJobTypeToQString() const;
};

class AbJobTypeTransfer : public AbJobType
{
public:
       AbJobTypeTransfer();
       const QString getJobTypeToQString() const;
};

class AbJobTypeUnknown : public AbJobType
{
public:
       AbJobTypeUnknown();
       const QString getJobTypeToQString() const;
};




#endif // ABJOBTYPE_HPP
