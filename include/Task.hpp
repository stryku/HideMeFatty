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

    QString getTimeAsQString() const
    {
        const uint16_t places = 3;

        return QString::fromStdString( timer.format(places, defaultFormat ) );
    }

    QString getNameWithTime() const
    {
        return taskName + " " + getTimeAsQString();
    }

    QString getName() const
    {
        return taskName;
    }
};

#endif //_INCLUDE_TASK_HPP_
