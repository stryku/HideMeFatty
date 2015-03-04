#include <iostream>

#include <Fat32Manager.hpp>
#include <DirectoryEntry.hpp>
#include <FileHidder.hpp>


using namespace std;

int main()
{
	Fat32Manager fat32mngr( "fat32example" );
	DirectoryEntry d;


	if( fat32mngr.isValidFat32() )
		cout << "ta\n";
	else 
		cout << "nie\n";

	fat32mngr.printFiles();

	fat32mngr.close();

	FileHidder fileHidder;

	fileHidder.hideFile( "test.txt", "fat32example" );

	return 0;
}