#include <iostream>
#include <string>
#include <vector>

#include <FileHidder.hpp>

std::wstring stringToWString( const std::string &s )
{
	return std::wstring( s.begin( ), s.end( ) );
}

std::wstring getPath( )
{
	std::string ret;

	getline( std::cin, ret );

	return stringToWString( ret );
}

std::wstring getPath( const char *text )
{
	std::cout << text;
	return getPath();
}

std::vector<std::wstring> getVectorOfPaths( const char *text )
{
	std::vector<std::wstring> ret;
	std::wstring tmp;

	std::cout << text;

	while( ( tmp = getPath() ) != stringToWString( "q" ) )
	{
		ret.push_back( tmp );
		std::cout << ">";
	}

	return ret;
}


void hide()
{
	std::vector<std::wstring> filesToHide, filesOnPartition;
	std::wstring partitionPath, partitionDevicePath;
	FileHidder fileHidder;

	filesToHide = getVectorOfPaths( "Put paths to files to hide. And 'q' after that\n\n>" );
	filesOnPartition = getVectorOfPaths("Put full paths to files on partiiton. And 'q' after that.\n\
										See documentation for details\n\n>" );
	partitionPath = getPath( "Put full path to partition\n\n>" );
	partitionDevicePath = getPath( "Put full path to partition device(that in /dev/ folder)\n\n>" );

	std::cout << "Starting hidding files. Be patient...";

	if( fileHidder.hideFiles( filesOnPartition,
		partitionPath,
		filesToHide,
		partitionDevicePath ) )
	{
		std::cout << "\nFiles hidded\n";
	}
	else
		std::cout << "Couldn't hide files. See last output log for details.\n";
}

void restore()
{
	std::vector<std::wstring> filesOnPartition;
	std::wstring partitionPath, partitionDevicePath, pathToStore;
	FileHidder fileHidder;

	filesOnPartition = getVectorOfPaths( "Put full paths to files on partiiton. And 'q' after that.\n\
										 										See documentation for details\n\n>" );
	partitionPath = getPath( "Put full path to partition\n\n>" );
	partitionDevicePath = getPath( "Put full path to partition device(that in /dev/ folder)\n\n>" );
	pathToStore = getPath( "Put path where store restored files\n\n>" );

	std::cout << "Starting restoring files. Be patient...";

	if( fileHidder.restoreMyFiles( filesOnPartition,
		partitionPath,
		partitionDevicePath,
		pathToStore ) )
	{
		std::cout << "Files restored.";
	}
	else
		std::cout << "Couldn't restore files. See last output log for details.\n";
}

int menu()
{
	int choice;

	std::cout << "\n\n\t\tMENU\n\n1. Hide files\n2. Restore files\n0. Exit\n>";

	std::cin >> choice;

	return choice;
}

void execute()
{

	while( true )
	{
		switch( menu() )
		{
			case 1: hide(); break;
			case 2: restore(); break;
			case 0: std::cout << "Bye bye"; return; break;

			default: std::cout << "Wrong number\n";
		}
	}
}

int main()
{
	execute();

	return 0;
}