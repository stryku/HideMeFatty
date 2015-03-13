#ifndef _INCLUDE_PATHOPERATIONS_HPP_
#define _INCLUDE_PATHOPERATIONS_HPP_

namespace pathOperations
{


template <class T>
T removeExtension( const T& filename )
{
	size_t lastdot = filename.find_last_of( '.' );

	if( lastdot == T::npos )
		return filename;

	return filename.substr( 0, lastdot );
}

template <class T>
T getPathFileName( const T &path ) const
{
	size_t pos = path.find_last_of( '/' );

	pos = ( pos == T::npos ) ? 0 : pos + 1; // pos+1 to not copy '/' char

	return path.substr( pos );
}

template <class T>
std::vector<T> getPathFoldersNames( const T &path ) const
{
	std::vector<T> folders;
	size_t posBegin = 0, posEnd, numberToCopy;

	while( ( posEnd = path.find( '/', posBegin ) ) != T::npos )
	{
		numberToCopy = posEnd - posBegin;
		folders.push_back( path.substr( posBegin, numberToCopy ) );
		posBegin = posEnd + 1;
	}

	return folders;
}

} // namespace pathOperations

#endif