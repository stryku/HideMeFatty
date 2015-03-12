#include <iostream>

#include <FileHidder.hpp>


using namespace std;

int main()
{

	FileHidder fileHidder;

	fileHidder.hideFile( "test.txt", "fat32example" );

	return 0;
}