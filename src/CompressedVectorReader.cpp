/*
 * CompressedVectorReader.cpp - implementation of public functions of the
 *   CompressedVectorReader class.
 *
 * Original work Copyright 2009 - 2010 Kevin Ackley (kackley@gwi.net)
 * Modified work Copyright 2018 - 2020 Andy Maloney <asmaloney@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/// @file CompressedVectorReader.cpp

#include "CompressedVectorReaderImpl.h"

using namespace e57;

// Put this function first so we can reference the code in doxygen using @skip
/*!
@brief Check whether CompressedVectorReader class invariant is true

@details
This function checks at least the assertions in the documented class invariant description (see
class reference page for this object). Other internal invariants that are implementation-dependent
may also be checked. If any invariant clause is violated, an ::ErrorInvarianceViolation E57Exception
is thrown.

@post No visible state is modified.
*/
void CompressedVectorReader::checkInvariant( bool /*doRecurse*/ )
{
   // If this CompressedVectorReader is not open, can't test invariant (almost
   // every call would throw)
   if ( !isOpen() )
   {
      return;
   }

   CompressedVectorNode cv = compressedVectorNode();
   ImageFile imf = cv.destImageFile();

   // If destImageFile not open, can't test invariant (almost every call would
   // throw)
   if ( !imf.isOpen() )
   {
      return;
   }

   // Associated CompressedVectorNode must be attached to ImageFile
   if ( !cv.isAttached() )
   {
      throw E57_EXCEPTION1( ErrorInvarianceViolation );
   }

   // Dest ImageFile must have at least 1 reader (this one)
   if ( imf.readerCount() < 1 )
   {
      throw E57_EXCEPTION1( ErrorInvarianceViolation );
   }

   // Dest ImageFile can't have any writers
   if ( imf.writerCount() != 0 )
   {
      throw E57_EXCEPTION1( ErrorInvarianceViolation );
   }
}

/*!
@class e57::CompressedVectorReader

@brief An iterator object keeping track of a read in progress from a CompressedVectorNode.

@details
A CompressedVectorReader object is a block iterator that reads blocks of records from a
CompressedVectorNode and stores them in memory buffers (SourceDestBuffers). Blocks of records are
processed rather than a single record-at-a-time for efficiency reasons. The CompressedVectorReader
class encapsulates all the state that must be saved in between the processing of one record block
and the next (e.g. partially read disk pages, or data decompression state). New memory buffers can
be used for each record block read, or the previous buffers can be reused.

CompressedVectorReader objects have an open/closed state. Initially a newly created
CompressedVectorReader is in the open state. After the API user calls CompressedVectorReader::close,
the object will be in the closed state and no more data transfers will be possible.

There is no CompressedVectorReader constructor in the API. The function CompressedVectorNode::reader
returns an already constructed CompressedVectorReader object with given memory buffers
(SourceDestBuffers) already associated.

It is recommended to call CompressedVectorReader::close to gracefully end the transfer. Unlike the
CompressedVectorWriter, not all fields in the record of the CompressedVectorNode are required to be
read at one time.

@section CompressedVectorReader_invariant Class Invariant
A class invariant is a list of statements about an object that are always true before and after any
operation on the object. An invariant is useful for testing correct operation of an implementation.
Statements in an invariant can involve only externally visible state, or can refer to internal
implementation-specific state that is not visible to the API user. The following C++ code checks
externally visible state for consistency and throws an exception if the invariant is violated:

@dontinclude CompressedVectorReader.cpp
@skip void CompressedVectorReader::checkInvariant
@until ^}

@see CompressedVectorNode, CompressedVectorWriter
*/

/// @cond documentNonPublic The following isn't part of the API, and isn't documented.
CompressedVectorReader::CompressedVectorReader( std::shared_ptr<CompressedVectorReaderImpl> ni ) :
   impl_( ni )
{
}
/// @endcond

/*!
@brief Request transfer of blocks of data from CompressedVectorNode into
previously designated destination buffers.

@details
The SourceDestBuffers used are previously designated either in
CompressedVectorNode::reader where this object was created, or in the last call
to CompressedVectorReader::read(std::vector<SourceDestBuffer>&) where new
buffers were designated. The function will always return the full number of
records requested (the capacity of the SourceDestBuffers) unless it has reached
the end of the CompressedVectorNode, in which case it will return less than the
capacity of the SourceDestBuffers. Partial reads will store the records at the
beginning of the SourceDestBuffers. It is not an error to call this function
after all records in the CompressedVectorNode have been read (the function
returns 0).

If a conversion or bounds error occurs during the transfer, the
CompressedVectorReader is left in an undocumented state (it can't be used any
further). If a file I/O or checksum error occurs during the transfer, both the
CompressedVectorReader and the associated ImageFile are left in an undocumented
state (they can't be used any further).

The API user is responsible for ensuring that the underlying memory buffers
represented in the SourceDestBuffers still exist when this function is called.
The E57 Foundation Implementation cannot detect that a memory buffer been
destroyed.

@pre The associated ImageFile must be open.
@pre This CompressedVectorReader must be open (i.e isOpen())

@return The number of records read.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorReaderNotOpen
@throw ::ErrorConversionRequired            This CompressedVectorReader
in undocumented state
@throw ::ErrorValueNotRepresentable        This CompressedVectorReader
in undocumented state
@throw ::ErrorScaledValueNotRepresentable This CompressedVectorReader
in undocumented state
@throw ::ErrorReal64TooLarge               This CompressedVectorReader
in undocumented state
@throw ::ErrorExpectingNumeric              This CompressedVectorReader
in undocumented state
@throw ::ErrorExpectingUString              This CompressedVectorReader
in undocumented state
@throw ::ErrorBadCVPacket      This CompressedVectorReader, associated
ImageFile in undocumented state
@throw ::ErrorSeekFailed       This CompressedVectorReader, associated
ImageFile in undocumented state
@throw ::ErrorReadFailed        This CompressedVectorReader, associated
ImageFile in undocumented state
@throw ::ErrorBadChecksum       This CompressedVectorReader, associated
ImageFile in undocumented state
@throw ::ErrorInternal           All objects in undocumented state

@see CompressedVectorReader::read(std::vector<SourceDestBuffer>&),
CompressedVectorNode::reader, SourceDestBuffer,
CompressedVectorReader::read(std::vector<SourceDestBuffer>&)
*/
unsigned CompressedVectorReader::read()
{
   return impl_->read();
}

/*!
@brief Request transfer of block of data from CompressedVectorNode into given destination buffers.

@param [in] dbufs Vector of memory buffers that will receive data read from a CompressedVectorNode.

@details
The @a dbufs must all have the same capacity.

The specified @a dbufs must have same number of elements as previously designated SourceDestBuffer
vector. The each SourceDestBuffer within @a dbufs must be identical to the previously designated
SourceDestBuffer except for capacity and buffer address.

The @a dbufs locations are saved so that a later call to CompressedVectorReader::read() can be used
without having to re-specify the SourceDestBuffers.

The function will always return the full number of records requested (the capacity of the
SourceDestBuffers) unless it has reached the end of the CompressedVectorNode, in which case it will
return less than the capacity of the SourceDestBuffers. Partial reads will store the records at the
beginning of the SourceDestBuffers. It is not an error to call this function after all records in
the CompressedVectorNode have been read (the function returns 0).

If a conversion or bounds error occurs during the transfer, the CompressedVectorReader is left in an
undocumented state (it can't be used any further). If a file I/O or checksum error occurs during the
transfer, both the CompressedVectorReader and the associated ImageFile are left in an undocumented
state (they can't be used any further).

The API user is responsible for ensuring that the underlying memory buffers represented in the
SourceDestBuffers still exist when this function is called. The E57 Foundation Implementation cannot
detect that a memory buffer been destroyed.

@pre The associated ImageFile must be open.
@pre This CompressedVectorReader must be open (i.e isOpen())

@return The number of records read.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorReaderNotOpen
@throw ::ErrorPathUndefined
@throw ::ErrorBufferSizeMismatch
@throw ::ErrorBufferDuplicatePathName
@throw ::ErrorConversionRequired This CompressedVectorReader in undocumented state
@throw ::ErrorValueNotRepresentable This CompressedVectorReader in undocumented state
@throw ::ErrorScaledValueNotRepresentable This CompressedVectorReader in undocumented state
@throw ::ErrorReal64TooLarge This CompressedVectorReader in undocumented state
@throw ::ErrorExpectingNumeric This CompressedVectorReader in undocumented state
@throw ::ErrorExpectingUString  This CompressedVectorReader in undocumented state
@throw ::ErrorBadCVPacket This CompressedVectorReader, associated ImageFile in undocumented state
@throw ::ErrorSeekFailed This CompressedVectorReader, associated ImageFile in undocumented state
@throw ::ErrorReadFailed This CompressedVectorReader, associated ImageFile in undocumented state
@throw ::ErrorBadChecksum This CompressedVectorReader, associated ImageFile in undocumented state
@throw ::ErrorInternal All objects in undocumented state

@see CompressedVectorReader::read(), CompressedVectorNode::reader, SourceDestBuffer
*/
unsigned CompressedVectorReader::read( std::vector<SourceDestBuffer> &dbufs )
{
   return impl_->read( dbufs );
}

/*!
@brief Set record number of CompressedVectorNode where next read will start.

@param [in] recordNumber The index of record in CompressedVectorNode where next read using this
CompressedVectorReader will start.

@details
This function may be called at any time (as long as ImageFile and CompressedVectorReader are open).
The next read will start at the given recordNumber. It is not an error to seek to recordNumber =
childCount() (i.e. to one record past end of CompressedVectorNode).

@pre @a recordNumber <= childCount() of CompressedVectorNode.
@pre The associated ImageFile must be open.
@pre This CompressedVectorReader must be open (i.e isOpen())

@throw ::ErrorBadAPIArgument
@throw ::ErrorImageFileNotOpen
@throw ::ErrorReaderNotOpen
@throw ::ErrorBadCVPacket
@throw ::ErrorSeekFailed
@throw ::ErrorReadFailed
@throw ::ErrorBadChecksum
@throw ::ErrorInternal All objects in undocumented state

@see CompressedVectorNode::reader
*/
void CompressedVectorReader::seek( int64_t recordNumber )
{
   impl_->seek( recordNumber );
}

/*!
@brief End the read operation.

@details
It is recommended that this function be called to gracefully end a transfer to a
CompressedVectorNode. It is not an error to call this function if the CompressedVectorReader is
already closed. This function will cause the CompressedVectorReader to enter the closed state, and
any further transfers requests will fail.

@throw ::ErrorInternal All objects in undocumented state

@see CompressedVectorReader::isOpen, CompressedVectorNode::reader
*/
void CompressedVectorReader::close()
{
   impl_->close();
}

/*!
@brief Test whether CompressedVectorReader is still open for reading.

@pre The associated ImageFile must be open.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorInternal           All objects in undocumented state

@see CompressedVectorReader::close, CompressedVectorNode::reader
*/
bool CompressedVectorReader::isOpen()
{
   return impl_->isOpen();
}

/*!
@brief Return the CompressedVectorNode being read.

@details
It is not an error if this CompressedVectorReader is closed.

@pre The associated ImageFile must be open.

@return A smart CompressedVectorNode handle referencing the underlying object being read from.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorInternal           All objects in undocumented state

@see CompressedVectorReader::close, CompressedVectorNode::reader
*/
CompressedVectorNode CompressedVectorReader::compressedVectorNode() const
{
   return impl_->compressedVectorNode();
}

/*!
@brief Diagnostic function to print internal state of object to output stream in an indented format.
@copydetails Node::dump()
*/
#ifdef E57_ENABLE_DIAGNOSTIC_OUTPUT
void CompressedVectorReader::dump( int indent, std::ostream &os ) const
{
   impl_->dump( indent, os );
}
#else
void CompressedVectorReader::dump( int indent, std::ostream &os ) const
{
   E57_UNUSED( indent );
   E57_UNUSED( os );
}
#endif
