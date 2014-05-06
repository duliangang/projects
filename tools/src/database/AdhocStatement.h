
#ifndef _ADHOCSTATEMENT_H
#define _ADHOCSTATEMENT_H
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "SQLOperation.h"
class ResultSet;
/*! Raw, ad-hoc query. */
typedef boost::function<void ( boost::shared_ptr<ResultSet> ) > BasicStatementCallbackFunc;

class BasicStatementTask : public SQLOperation
{
    public:
        BasicStatementTask(const char* sql);
        BasicStatementTask(const char* sql,BasicStatementCallbackFunc callback);
        ~BasicStatementTask();

        bool Execute();

    private:
        const char* m_sql;      //- Raw query to be executed
        bool m_has_result;
		BasicStatementCallbackFunc m_callback;
};

#endif