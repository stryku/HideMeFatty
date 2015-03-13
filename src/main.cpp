#include <iostream>

#include <FileHidder.hpp>
#include <Fat32Manager.hpp>
#include <DistributedMemoryMapper.hpp>

using namespace std;

struct Test
{
	std::wstring path;
	std::wstring expected;

	Test() {}
	Test( const std::string &_path,
		  const std::string &_expected )
	{
		path.assign( _path.begin( ), _path.end( ) );
		expected.assign( _expected.begin( ), _expected.end( ) );
	}

	void print( std::wostream &out, bool result )
	{
		out << "\n==========================\n\n";
		out << "Path:\t\t" << path << "\n";
		out << "Expected:\t" << expected << "\n";
		out << "Test result:\t" << std::boolalpha << result << "\n";
		out << "\n==========================\n";
	}
};

int main()
{
	char tab[100];

	DistributedMemoryMapper dmm;

	for( int i = 0; i < 100; ++i )
		tab[i] = 'a';

	dmm.addMemoryChunk( tab, 2 );
	dmm.addMemoryChunk( tab + 4, 2 );
	dmm.addMemoryChunk( tab + 8, 2 );
	dmm.addMemoryChunk( tab + 12, 2 );

	char c = 'b';
	for( int i = 0; i < 8; ++i, ++c )
		dmm[i] = c;

	return 0;
}