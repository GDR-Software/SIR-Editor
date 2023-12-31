//----------------------------------------------------------------------------
//
// DESCRIPTION:
// map format interface (.map and .xmap, Q3 and other games)
//

#ifndef __IMAP_H__
#define __IMAP_H__

/*! IMap depends on IDataStream, including the header there for now */
#include "idatastream.h"

/*! header for CPtrArray */
#include "missing.h"

#define MAP_MAJOR "map"
/*!
   define a GUID for this interface so everyone can acces and reference it
   {75076973-3414-49c9-be5b-2378ec5601af}
 */
static const GUID ERPlugMapTable_GUID =
{ 0x75076973, 0x3414, 0x49c9, { 0xbe, 0x5b, 0x23, 0x78, 0xec, 0x56, 0x01, 0xaf } };

/*!
   read from a stream into a list of entities
   \param in the input stream. For regular map file parsing it's possible to copy the content in a text buffer
   and use the old school parser
   \param ents the list of entities read from the stream. They are not linked to the world, and their brushes
   are not either.
 */
typedef void ( *PFN_MAP_READ )( IDataStream *in, CPtrArray *ents ); ///< read from a stream into a list of entities
typedef void ( *PFN_MAP_WRITE )( CPtrArray *ents, IDataStream *out ); ///< save a list of entities into a stream

struct _ERPlugMapTable
{
	int m_nSize;
	PFN_MAP_READ m_pfnMap_Read;
	PFN_MAP_WRITE m_pfnMap_Write;
};

/*!
   this set of macros will define the functions to map on a given table
   it should be used in the headers (see modules source, plugin.h)
   we don't want those defines in the part where WE implement the Map_LoadFile
   so we're using a define to disable .. should find a standard define name
   (for instance QCOM_CLIENT / QCOM_SERVER ?)
   or the name should be specific to any interface .. it's not a client/server thing here anyway
 */
#ifdef USE_MAPTABLE_DEFINE
#ifndef __MAPTABLENAME
/*!
   TTimo NOTE: this is the default table name we map to
   if you are using a different table name, just define __MAPTABLENAME before you include the imap.h header
 */
#define __MAPTABLENAME g_MapTable
#endif
#define Map_Read __MAPTABLENAME.m_pfnMap_Read
#define Map_Write __MAPTABLENAME.m_pfnMap_Write
#endif

#endif
