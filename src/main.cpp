#include <iostream>
#include <Fat32Manager.hpp>

using namespace std;

int main()
{
	Fat32Manager fat32mngr( "fat32example" );

	if( fat32mngr.isValidFat32() )
		cout << "ta\n";
	else cout << "nie\n";

	fat32mngr.printFiles();

	return 0;
}