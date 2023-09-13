#ifndef _ISTREAM_H_
#define _ISTREAM_H_

#pragma once

/*!
   API for data streams

   Based on an initial implementation by Loki software
   modified to be abstracted and shared across modules

   NOTE: why IDataStream and not IStream? because IStream is defined in windows IDL headers
 */

class IDataStream
{
public:
    IDataStream(void);
    virtual ~IDataStream();

    virtual void IncRef(void) = 0;      ///< Increment the number of references to this object
    virtual void DecRef(void) = 0;      ///< Decrement the reference count

    virtual uint64_t GetPosition() const = 0;
    virtual uint64_t Seek( uint64_t lOff, int nFrom ) = 0;
    virtual void SetLength( uint64_t nNewLen ) = 0;
    virtual uint64_t GetLength() const = 0;

    virtual char* ReadString( char* pBuf, uint64_t nMax ) = 0;
    virtual uint64_t Read( void* pBuf, uint64_t nCount ) = 0;
    virtual uint64_t Write( const void* pBuf, uint64_t nCount ) = 0;
    virtual int GetChar( void ) = 0;
    virtual int PutChar( int c ) = 0;

    virtual void printf( const char*, ... ) = 0;   ///< completely matches the usual printf behaviour

    virtual void Abort(void) = 0;
    virtual void Flush(void) = 0;
    virtual void Close(void) = 0;
};

#endif // _ISTREAM_H_