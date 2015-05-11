#include <Task.hpp>

std::string Task::defaultFormat = " %ws";

QString Task::getTimeAsQString() const
{
    const uint16_t places = 3;

    return QString::fromStdString( timer.format(places, defaultFormat ) );
}

QString Task::getName() const
{
    return taskName;
}
