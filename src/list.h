/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __UTIL_LIST_H__
#define __UTIL_LIST_H__

#pragma once

#include <stdlib.h>
#include <assert.h>

template< class type >
class idList {
private:
	uint64_t m_num;
	uint64_t m_size;
	uint64_t m_granularity;
	type        *m_list;
public:
	idList( uint64_t granularity = 16 );
	~idList<type>();
	bool operator==(const idList<type>& other) const;
	bool operator!=(const idList<type>& other) const;
	bool		Cmp(const idList<type>& other) const;
	void        Clear( void );
	uint64_t         Num( void ) const;
	void        SetNum( uint64_t num );
	void        SetGranularity( uint64_t granularity );
	void        Condense( void );
	uint64_t         Size( void ) const;
	void        Resize( uint64_t size );
	type operator[]( uint64_t index ) const;
	type        &operator[]( uint64_t index );
	type		*GetBuffer(void);
	const type	*GetBuffer(void) const;
	uint64_t         Append( type const & obj );
	uint64_t         AddUnique( type const & obj );
	type        *Find( type const & obj, uint64_t *index = NULL );
	bool        RemoveIndex( uint64_t index );
	bool        Remove( type const & obj );
	typedef uint64_t cmp_t ( const void *, const void * );
	void        Sort( cmp_t *compare );
};

/*
   ================
   idList<type>::idList( uint64_t )
   ================
 */
template< class type >
inline idList<type>::idList( uint64_t granularity ) {
	assert( granularity > 0 );

	m_list          = NULL;
	m_granularity   = granularity;
	Clear();
}

template< class type >
inline bool idList<type>::Cmp( const idList<type>& other ) const
{
	if (m_size != other.m_size)
		return false;
	
	return !memcmp(m_list, other.m_list, sizeof(*m_list) * m_size);
}

template< class type >
inline bool idList<type>::operator==( const idList<type>& other ) const
{
	return Cmp(other);
}

template< class type >
inline bool idList<type>::operator!=( const idList<type>& other ) const
{
	return !Cmp(other);
}

/*
   ================
   idList<type>::~idList<type>
   ================
 */
template< class type >
inline idList<type>::~idList() {
	Clear();
}

/*
   ================
   idList<type>::Clear
   ================
 */
template< class type >
inline void idList<type>::Clear( void ) {
	if ( m_list ) {
		delete[] m_list;
	}

	m_list  = NULL;
	m_num   = 0;
	m_size  = 0;
}

/*
   ================
   idList<type>::Num
   ================
 */
template< class type >
inline uint64_t idList<type>::Num( void ) const {
	return m_num;
}

/*
   ================
   idList<type>::SetNum
   ================
 */
template< class type >
inline void idList<type>::SetNum( uint64_t num ) {
	assert( num >= 0 );
	if ( num > m_size ) {
		// resize it up to the closest level of granularity
		Resize( ( ( num + m_granularity - 1 ) / m_granularity ) * m_granularity );
	}
	m_num = num;
}

/*
   ================
   idList<type>::SetGranularity
   ================
 */
template< class type >
inline void idList<type>::SetGranularity( uint64_t granularity ) {
	uint64_t newsize;

	assert( granularity > 0 );
	m_granularity = granularity;

	if ( m_list ) {
		// resize it to the closest level of granularity
		newsize = ( ( m_num + m_granularity - 1 ) / m_granularity ) * m_granularity;
		if ( newsize != m_size ) {
			Resize( newsize );
		}
	}
}

/*
   ================
   idList<type>::Condense

   Resizes the array to exactly the number of elements it contains
   ================
 */
template< class type >
inline void idList<type>::Condense( void ) {
	if ( m_list ) {
		if ( m_num ) {
			Resize( m_num );
		}
		else {
			Clear();
		}
	}
}

/*
   ================
   idList<type>::Size
   ================
 */
template< class type >
inline uint64_t idList<type>::Size( void ) const {
	return m_size;
}

/*
   ================
   idList<type>::Resize
   ================
 */
template< class type >
inline void idList<type>::Resize( uint64_t size ) {
	type    *temp;
	uint64_t i;

	assert( size > 0 );

	if ( size <= 0 ) {
		Clear();
		return;
	}

	temp    = m_list;
	m_size  = size;
	if ( m_size < m_num ) {
		m_num = m_size;
	}

	m_list = new type[ m_size ];
	for ( i = 0; i < m_num; i++ ) {
		m_list[ i ] = temp[ i ];
	}

	if ( temp ) {
		delete[] temp;
	}
}

/*
   ================
   idList<type>::operator[] const
   ================
 */
template< class type >
inline type idList<type>::operator[]( uint64_t index ) const {
	assert( index >= 0 );
	assert( index < m_num );

	return m_list[ index ];
}

/*
   ================
   idList<type>::operator[]
   ================
 */
template< class type >
inline type &idList<type>::operator[]( uint64_t index ) {
	assert( index >= 0 );
	assert( index < m_num );

	return m_list[ index ];
}

template< class type >
inline type *idList<type>::GetBuffer( void )
{
	return m_list;
}

template< class type >
inline const type *idList<type>::GetBuffer( void ) const
{
	return m_list;
}

/*
   ================
   idList<type>::Append
   ================
 */
template< class type >
inline uint64_t idList<type>::Append( type const & obj ) {
	if ( !m_list ) {
		Resize( m_granularity );
	}

	if ( m_num == m_size ) {
		Resize( m_size + m_granularity );
	}

	m_list[ m_num ] = obj;
	m_num++;

	return m_num - 1;
}

/*
   ================
   idList<type>::AddUnique
   ================
 */
template< class type >
inline uint64_t idList<type>::AddUnique( type const & obj ) {
	uint64_t index;

	if ( !Find( obj, &index ) ) {
		index = Append( obj );
	}

	return index;
}

/*
   ================
   idList<type>::Find
   ================
 */
template< class type >
inline type *idList<type>::Find( type const & obj, uint64_t *index ) {
	uint64_t i;

	for ( i = 0; i < m_num; i++ ) {
		if ( m_list[ i ] == obj ) {
			if ( index ) {
				*index = i;
			}
			return &m_list[ i ];
		}
	}

	return NULL;
}

/*
   ================
   idList<type>::RemoveIndex
   ================
 */
template< class type >
inline bool idList<type>::RemoveIndex( uint64_t index ) {
	uint64_t i;

	if ( !m_list || !m_num ) {
		return false;
	}

	assert( index >= 0 );
	assert( index < m_num );

	if ( ( index < 0 ) || ( index >= m_num ) ) {
		return false;
	}

	m_num--;
	for ( i = index; i < m_num; i++ ) {
		m_list[ i ] = m_list[ i + 1 ];
	}

	return true;
}

/*
   ================
   idList<type>::Remove
   ================
 */
template< class type >
inline bool idList<type>::Remove( type const & obj ) {
	uint64_t index;

	if ( Find( obj, &index ) ) {
		return RemoveIndex( index );
	}

	return false;
}

/*
   ================
   idList<type>::Sort
   ================
 */
template< class type >
inline void idList<type>::Sort( cmp_t *compare ) {
	if ( !m_list ) {
		return;
	}

	qsort( ( void * )m_list, ( size_t )m_num, sizeof( type ), compare );
}

#endif /* !__UTIL_LIST_H__ */
