#include <iostream>

#include <FileHidder.hpp>
#include <Fat32Manager.hpp>

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
	Fat32Manager fat( "fat32example" );
	Test tests[] =
	{
		Test( std::string( "katalog_1/katalog_1_1/" ), std::string( "true" ) ),
		Test( std::string( "katalog_1/katalog_1_1" ), std::string( "true" ) ),
		Test( std::string( "katalog_1/katalog_1_1/plik_1_2.txt" ), std::string( "false" ) ),
		Test( std::string( "katalog_1/katalog_1_1/plik_1_3.txt" ), std::string( "false" ) ),
		Test( std::string( "katalog_1/katalog_1_1/plik_1_4.txt" ), std::string( "false" ) ),
		Test( std::string( "katalog_1/katalog_1_1/bad/plik_1_4.txt" ), std::string( "false" ) ),
	};

	for( auto &i : tests )
	{
		i.print( wcout, fat.isPathCorrect( i.path ) );
	}

	return 0;
}