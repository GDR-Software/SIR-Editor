/*

file.h: slightly modified version from gtkradiant
*/

#ifndef __FILE__
#define __FILE__

#pragma once

class MemFile : public IDataStream
{
public:
    MemFile(void);
    MemFile(uint64_t nLen);
    virtual ~MemFile();

    uint32_t mRefCount;

    void IncRef(void) { mRefCount++; }
    void DecRef(void)
    {
        mRefCount--;
        if ( !mRefCount )
            delete this; // FIXME...?
    }
protected:
    uint64_t m_nGrowBytes;
    uint64_t m_nPosition;
    uint64_t m_nBufferSize;
    uint64_t m_nFileSize;
    byte* m_pBuffer;
    bool m_bAutoDelete;
    void GrowFile( uint64_t nNewLen );
public:
    uint64_t GetPosition( void ) const;
    uint64_t Seek( uint64_t lOff, int nFrom );
    void SetLength( uint64_t nNewLen );
    uint64_t GetLength( void ) const;

    const byte* GetBuffer( void ) const
    { return m_pBuffer; }
    byte* GetBuffer( void )
    { return m_pBuffer; }

    char* ReadString( char* pBuf, uint64_t nMax );
    uint64_t Read( void* pBuf, uint64_t nCount );
    uint64_t Write( const void* pBuf, uint64_t nCount );
    int GetChar( void );
    int PutChar( int c );

    void printf( const char*, ... ); ///< \todo implement on MemStream

    void Abort( void );
    void Flush( void );
    void Close( void );
    bool Open( const char *filename, const char *mode );
};

class FileStream : public IDataStream
{
public:
    FileStream(void);
    virtual ~FileStream();

    uint32_t mRefCount;
    void IncRef(void) { mRefCount++; }
    void DecRef(void)
    {
    	mRefCount--;
        if ( !mRefCount )
    		delete this; // FIXME...?
    }

protected:
// DiscFile specific:
    FILE* m_hFile;
    bool m_bCloseOnDelete;

public:
    uint64_t GetPosition( void ) const;
    uint64_t Seek( uint64_t lOff, int nFrom );
    void SetLength( uint64_t nNewLen );
    uint64_t GetLength( void ) const;

    char* ReadString( char* pBuf, uint64_t nMax );
    uint64_t Read( void* pBuf, uint64_t nCount );
    uint64_t Write( const void* pBuf, uint64_t nCount );
    int GetChar( void );
    int PutChar( int c );

    void printf( const char*, ... ); ///< completely matches the usual printf behaviour

    void Abort( void );
    void Flush( void );
    void Close( void );
    bool Open( const char *filename, const char *mode );
};

#endif