#include <iostream>

#include <FileHidder.hpp>
#include <Fat32Manager.hpp>
#include <DistributedMemoryMapper.hpp>

using namespace std;


std::wstring StringToWString( const std::string &s )
{
	std::wstring wsTmp( s.begin( ), s.end( ) );

	return wsTmp;
}

int main()
{
	FileHidder fileHidder;

	vector<wstring> filesToHide;
	vector<wstring> filesOnPartition;
	wstring partitionPath = StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc" );

	filesToHide.push_back( StringToWString( "test.txt" ) );
	//filesToHide.push_back( StringToWString( "test.png" ) );

	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/linux-2.6.32.64.tar.xz" ) );

	fileHidder.hideFiles( filesOnPartition, partitionPath, filesToHide, StringToWString( "fat32example" ) );

	fileHidder.restoreMyFiles( filesOnPartition, partitionPath, StringToWString( "fat32example" ), StringToWString( "finded" ) );

	return 0;
}