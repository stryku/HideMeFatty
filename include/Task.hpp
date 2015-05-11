#ifndef _INCLUDE_TASK_HPP_
#define _INCLUDE_TASK_HPP_

#include <QString>
#include <boost/timer/timer.hpp>

using boost::timer::cpu_times;

class Task
{
private:
    static std::string defaultFormat;

    boost::timer::cpu_timer timer;
    QString taskName;

public:
    Task() {}
    Task( const QString &taskName ) :
        taskName( taskName )
    {}

    QString getTimeAsQString() const;

    QString getName() const;
};

#endif //_INCLUDE_TASK_HPP_
