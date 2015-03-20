#include <iostream> 
#include <string>
#include <vector>

#include <easylogging++.h>

#include <FileHidder.hpp>


std::string getPath( )
{
	std::string ret;

	while( ret.length() == 0 )
	{
		std::cout << ">";
		getline( std::cin, ret );
	}

	return ret;
}

std::string getPath( const char *text )
{
	std::cout << text;
	return getPath();
}

std::vector<std::string> getVectorOfPaths( const char *text )
{
	std::vector<std::string> ret;
	std::string tmp;

	std::cout << text;

	while( ( tmp = getPath() ) != "q" && tmp != "Q" )
		ret.push_back( tmp );

	return ret;
}

void hide()
{
	std::vector<std::string> filesToHide, filesOnPartition;
	std::string partitionPath, partitionDevicePath;
	FileHidder fileHidder;

	filesToHide = getVectorOfPaths( "\nPut paths to files to hide. And 'q' after that\n\n" );
	filesOnPartition = getVectorOfPaths("\nPut full paths to files on partiiton. And 'q' after that.\nSee documentation for details\n\n" );
	partitionPath = getPath( "\nPut full path to partition\n\n" );
	partitionDevicePath = getPath( "\nPut full path to partition device(that in /dev/ folder)\n\n" );

	std::cout << "\nStarting hidding files. Be patient...";

	if( fileHidder.hideFiles( filesOnPartition,
		partitionPath,
		filesToHide,
		partitionDevicePath ) )
	{
		std::cout << "\nFiles hidded\n";
	}
	else
		std::cout << "\nCouldn't hide files. See last output log for details.\n";
}

void restore()
{
	std::vector<std::string> filesOnPartition;
	std::string partitionPath, partitionDevicePath, pathToStore;
	FileHidder fileHidder;

	filesOnPartition = getVectorOfPaths( "\nPut full paths to files on partiiton. And 'q' after that.\nSee documentation for details\n\n" );
	partitionPath = getPath( "\nPut full path to partition\n\n" );
	partitionDevicePath = getPath( "\nPut full path to partition device(that in /dev/ folder)\n\n" );
	pathToStore = getPath( "\nPut path where store restored files\n\n" );

	std::cout << "\nStarting restoring files. Be patient...";

	if( fileHidder.restoreMyFiles( filesOnPartition,
		partitionPath,
		partitionDevicePath,
		pathToStore ) )
	{
		std::cout << "\nFiles restored.";
	}
	else
		std::cout << "\nCouldn't restore files. See last output log for details.\n";
}

int menu()
{
	int choice;

	std::cout << "\n\n\t\tMENU\n\n1. Hide files\n2. Restore files\n0. Exit\n>";

	std::cin >> choice;

	while( std::cin.get() != '\n' ) {}

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

void easyLoggingInit( )
{
	el::Configurations conf( "files/conf/logger.conf" );
	el::Loggers::reconfigureAllLoggers( conf );

	LOG( INFO ) << "New session";
}

INITIALIZE_EASYLOGGINGPP

int main()
{
	easyLoggingInit();
	execute();

	return 0;
}
