#include <iostream>
#include <string>

#include <FileHidder.hpp>

std::string getPartitionPath()
{
	std::string ret;

	std::cout << "Put full path to partition\n>";
	getline( std::cin, ret );

	return ret;
}

std::string getPartitionDevPath()
{
	std::string ret;

	std::cout << "Put full path to partition device(that in /dev/ folder)\n>";
	getline( std::cin, ret );

	return ret;
}

std::vector<std::string> getFilesOnPartition( )
{
	std::vector<std::string> ret;
	std::string tmp( "" );

	std::cout << "Put full path to files in partition.\n";
	std::cout << "See documentation for details\n";
	std::cout << "Put 'q' after all files\n";

	while( tmp != "q" )
	{
		getline( std::cin, tmp );

		if( tmp == "q" )
			break;

		ret.push_back( tmp );
	}

	return ret;
}

std::vector<std::string> getFilesToHide( )
{
	std::vector<std::string> ret;
	std::string tmp( "" );

	std::cout << "Put full path to files to hide.\n";
	std::cout << "Put 'q' after all files\n";

	while( tmp != "q" )
	{
		getline( std::cin, tmp );

		if( tmp == "q" )
			break;

		ret.push_back( tmp );
	}

	return ret;
}

bool hide()
{
	std::vector<std::string> filesOnPartition, filesToHide;
	std::string partitionPath, partitionDevPath;


	partitionPath = getPartitionPath( );
	partitionDevPath = getPartitionDevPath( );
	filesOnPartition = getFilesOnPartition( );
	filesToHide = getFilesToHide( );


}

void execute( int argc, char *argv[] )
{
	


}

int main()
{
	FileHidder fileHidder;

	vector<wstring> filesToHide;
	vector<wstring> filesOnPartition;
	wstring partitionPath = StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc" );

	filesToHide.push_back( StringToWString( "test.txt" ) );
	filesToHide.push_back( StringToWString( "test.png" ) );

	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/linux-2.6.32.64.tar.xz" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/free.jpg" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/[kickass.so]pure.milf.7.xxx.dvdrip.x264.redsection.torrent" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/[kickass.so]the.amazing.spiderman.2012.1080p.brrip.x264.yify.torrent" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/[kickass.so]windows.xp.professional.sp3.nov.2013.sata.drivers.thumperdc.torrent" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/2.rar" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/4689873108.jpg" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/Bespin.zip" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/Diablo II   Pan Zniszczenia [ ISO  BIN] [5CD] [PL][Torrenty.org] (1).torrent" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/main.c" ) );
	filesOnPartition.push_back( StringToWString( "C:/PROGRAMOWANIE/C++/PROJEKTY/MOJE/HIDE_ME_FATTY/files/zawartosc/pliczki/nieznany_kapsel.jpg" ) );
	
	fileHidder.hideFiles( filesOnPartition, partitionPath, filesToHide, StringToWString( "fat32example" ) );

	fileHidder.restoreMyFiles( filesOnPartition, partitionPath, StringToWString( "fat32example" ), StringToWString( "finded" ) );

	return 0;
}