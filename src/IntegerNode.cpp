/*
 * IntegerNode.cpp - implementation of public functions of the IntegerNode class.
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

/// @file IntegerNode.cpp

#include "IntegerNodeImpl.h"
#include "StringFunctions.h"

using namespace e57;

// Put this function first so we can reference the code in doxygen using @skip
/*!
@brief Check whether IntegerNode class invariant is true

@param [in] doRecurse If true, also check invariants of all children or sub-objects recursively.
@param [in] doUpcast If true, also check invariants of the generic Node class.

@details
This function checks at least the assertions in the documented class invariant description (see
class reference page for this object). Other internal invariants that are implementation-dependent
may also be checked. If any invariant clause is violated, an ::ErrorInvarianceViolation E57Exception
is thrown.

Checking the invariant recursively may be expensive if the tree is large, so should be used
judiciously, in debug versions of the application.

@post No visible state is modified.

@throw ::ErrorInvarianceViolation or any other E57 ErrorCode
*/
void IntegerNode::checkInvariant( bool doRecurse, bool doUpcast ) const
{
   E57_UNUSED( doRecurse );

   // If destImageFile not open, can't test invariant (almost every call would throw)
   if ( !destImageFile().isOpen() )
   {
      return;
   }

   // If requested, call Node::checkInvariant
   if ( doUpcast )
   {
      static_cast<Node>( *this ).checkInvariant( false, false );
   }

   if ( value() < minimum() || value() > maximum() )
   {
      throw E57_EXCEPTION1( ErrorInvarianceViolation );
   }
}

/*!
@class e57::IntegerNode

@brief An E57 element encoding an integer value.

@details
An IntegerNode is a terminal node (i.e. having no children) that holds an integer value, and
minimum/maximum bounds. Once the IntegerNode value and attributes are set at creation, they may not
be modified.

The minimum attribute may be a number in the interval [-2^63, 2^63).
The maximum attribute may be a number in the interval [minimum, 2^63).
The value may be a number in the interval [minimum, maximum].

See Node class discussion for discussion of the common functions that StructureNode supports.

@section IntegerNode_invariant Class Invariant
A class invariant is a list of statements about an object that are always true before and after any
operation on the object. An invariant is useful for testing correct operation of an implementation.
Statements in an invariant can involve only externally visible state, or can refer to internal
implementation-specific state that is not visible to the API user. The following C++ code checks
externally visible state for consistency and throws an exception if the invariant is violated:

@dontinclude IntegerNode.cpp
@skip void IntegerNode::checkInvariant
@until ^}

@see Node, CompressedVector
*/

/*!
@brief Create an E57 element for storing a integer value.

@param [in] destImageFile The ImageFile where the new node will eventually be stored.
@param [in] value The integer value of the element.
@param [in] minimum The smallest value that the element may take.
@param [in] maximum The largest value that the element may take.

@details
An IntegerNode stores an integer value, and a lower and upper bound.
The IntegerNode class corresponds to the ASTM E57 standard Integer element.
See the class discussion at bottom of IntegerNode page for more details.

The @a destImageFile indicates which ImageFile the IntegerNode will eventually be attached to. A
node is attached to an ImageFile by adding it underneath the predefined root of the ImageFile
(gotten from ImageFile::root). It is not an error to fail to attach the IntegerNode to the @a
destImageFile. It is an error to attempt to attach the IntegerNode to a different ImageFile.

@warning It is an error to give an @a value outside the @a minimum / @a maximum bounds, even if the
IntegerNode is destined to be used in a CompressedVectorNode prototype (where the @a value will be
ignored). If the IntegerNode is to be used in a prototype, it is recommended to specify a @a value =
0 if 0 is within bounds, or a @a value = @a minimum if 0 is not within bounds.

@pre The @a destImageFile must be open (i.e. destImageFile.isOpen() must be true).
@pre The @a destImageFile must have been opened in write mode (i.e. destImageFile.isWritable() must
be true).
@pre minimum <= value <= maximum

@throw ::ErrorBadAPIArgument
@throw ::ErrorImageFileNotOpen
@throw ::ErrorFileReadOnly
@throw ::ErrorValueOutOfBounds
@throw ::ErrorInternal All objects in undocumented state

@see IntegerNode::value, Node, CompressedVectorNode, CompressedVectorNode::prototype
*/
IntegerNode::IntegerNode( const ImageFile &destImageFile, int64_t value, int64_t minimum,
                          int64_t maximum ) :
   impl_( new IntegerNodeImpl( destImageFile.impl(), value, minimum, maximum ) )
{
   impl_->validateValue();
}

/*!
@brief Is this a root node.
@copydetails Node::isRoot()
*/
bool IntegerNode::isRoot() const
{
   return impl_->isRoot();
}

/*!
@brief Return parent of node, or self if a root node.
@copydetails Node::parent()
*/
Node IntegerNode::parent() const
{
   return Node( impl_->parent() );
}

/*!
@brief Get absolute pathname of node.
@copydetails Node::pathName()
*/
ustring IntegerNode::pathName() const
{
   return impl_->pathName();
}

/*!
@brief Get elementName string, that identifies the node in its parent.
@copydetails Node::elementName()
*/
ustring IntegerNode::elementName() const
{
   return impl_->elementName();
}

/*!
@brief Get the ImageFile that was declared as the destination for the node when it was created.
@copydetails Node::destImageFile()
*/
ImageFile IntegerNode::destImageFile() const
{
   return ImageFile( impl_->destImageFile() );
}

/*!
@brief Has node been attached into the tree of an ImageFile.
@copydetails Node::isAttached()
*/
bool IntegerNode::isAttached() const
{
   return impl_->isAttached();
}

/*!
@brief Get integer value stored.

@pre The destination ImageFile must be open (i.e. destImageFile().isOpen()).
@post No visible state is modified.

@return integer value stored.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorInternal  All objects in undocumented state

@see IntegerNode::minimum, IntegerNode::maximum
*/
int64_t IntegerNode::value() const
{
   return impl_->value();
}

/*!
@brief Get the declared minimum that the value may take.

@pre The destination ImageFile must be open (i.e. destImageFile().isOpen()).
@post No visible state is modified.

@return The declared minimum that the value may take.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorInternal All objects in undocumented state

@see IntegerNode::value
*/
int64_t IntegerNode::minimum() const
{
   return impl_->minimum();
}

/*!
@brief Get the declared maximum that the value may take.

@pre The destination ImageFile must be open (i.e. destImageFile().isOpen()).
@post No visible state is modified.

@return The declared maximum that the value may take.

@throw ::ErrorImageFileNotOpen
@throw ::ErrorInternal All objects in undocumented state

@see IntegerNode::value
*/
int64_t IntegerNode::maximum() const
{
   return impl_->maximum();
}

/*!
@brief Diagnostic function to print internal state of object to output stream in an indented format.
@copydetails Node::dump()
*/
#ifdef E57_ENABLE_DIAGNOSTIC_OUTPUT
void IntegerNode::dump( int indent, std::ostream &os ) const
{
   impl_->dump( indent, os );
}
#else
void IntegerNode::dump( int indent, std::ostream &os ) const
{
   E57_UNUSED( indent );
   E57_UNUSED( os );
}
#endif

/*!
@brief Upcast a IntegerNode handle to a generic Node handle.

@details
An upcast is always safe, and the compiler can automatically insert it for initializations of Node
variables and Node function arguments.

@return A smart Node handle referencing the underlying object.

@throw No E57Exceptions.

@see explanation in Node, Node::type(), IntegerNode(const Node&)
*/
IntegerNode::operator Node() const
{
   // Upcast from shared_ptr<IntegerNodeImpl> to SharedNodeImplPtr and construct a Node object
   return Node( impl_ );
}

/*!
@brief Downcast a generic Node handle to a IntegerNode handle.

@param [in] n The generic handle to downcast.

@details
The handle @a n must be for an underlying IntegerNode, otherwise an exception is thrown. In designs
that need to avoid the exception, use Node::type() to determine the actual type of the @a n before
downcasting. This function must be explicitly called (c++ compiler cannot insert it automatically).

@throw ::ErrorBadNodeDowncast

@see Node::type(), IntegerNode::operator Node()
*/
IntegerNode::IntegerNode( const Node &n )
{
   if ( n.type() != TypeInteger )
   {
      throw E57_EXCEPTION2( ErrorBadNodeDowncast, "nodeType=" + toString( n.type() ) );
   }

   // Set our shared_ptr to the downcast shared_ptr
   impl_ = std::static_pointer_cast<IntegerNodeImpl>( n.impl() );
}

/// @cond documentNonPublic The following isn't part of the API, and isn't documented.
IntegerNode::IntegerNode( std::shared_ptr<IntegerNodeImpl> ni ) : impl_( ni )
{
}
/// @endcond
