#ifndef _INCLUDE_STRUCTS_
#define _INCLUDE_STRUCTS_

struct Date
{
	size_t second,
	minute,
	hour,
	day,
	month,
	year;

	Date() :
		second( 0 ),
		minute( 0 ),
		hour( 0 ),
		day( 0 ),
		month( 0 ),
		year( 0 )
	{}
	~Date() {}

	bool operator==( const Date &d ) const
	{
		return d.second == second &&
				d.minute == minute &&
				d.hour == hour &&
				d.day == day &&
				d.month == month &&
				d.year == year;
	}
};

struct ClusterInfo
{
	size_t clusterNo,
	freeBytes,
	freeBytesOffset;
};

#endif