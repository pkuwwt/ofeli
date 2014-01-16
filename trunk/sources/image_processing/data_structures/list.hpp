/****************************************************************************
**
** Copyright (C) 2010-2013 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef LIST_HPP
#define LIST_HPP

#include <iostream> // for operator<< overloading

namespace ofeli
{

template <typename T = int>
class List
{

    //////////////////////   Nested structure and class   /////////////////////



        //////////////////////////   Declarations   ///////////////////////////

public :

    class const_iterator;

    class iterator;

private :

    //! Element of the list.
    struct Node;
    //! Link is a pointer to a Node.
    typedef Node* Link;

        ///////////////////////////////////////////////////////////////////////


        //////////////////////////   Definitions   ///////////////////////////

public :

    //! const iterator to read a const list.
    class const_iterator
    {

        //! list has access to the private members of \a const_iterator.
        friend class List;

    public :

        //! Gets the node data. \a const_iterator protects the data against writing.
        const T& operator*() const
        {
            return node->data;
        }

        //! Checks if the \a const_iterator is at the end of the list, i.e. if the node is the sentinel node.
        bool end() const
        {
            return node->end();
        }

        //! Increment \a const_iterator.
        const_iterator& operator++()
        {
            node = node->next;
            return *this;
        }

    private :

        //! Constructor with a node.
        const_iterator(Link node1) : node(node1)
        {
        }

        //! \a const_iterator encapsulates a pointer to a node.
        Link node;
    };

    //! Iterator to modify a list.
    class iterator
    {

        //! \a list has access to the private members of iterator.
        friend class List;

    public :

        //! Gets the node data. \a iterator does not protect the data against writing.
        T& operator*() const
        {
            return node->data;
        }

        //! Checks if the iterator is at the end of the list, i.e. if the node is the sentinel node.
        bool end() const
        {
            return node->end();
        }

        //! Increment the iterator.
        iterator& operator++()
        {
            node = node->next;
            return *this;
        }

    private :

        //! Constructor with a node.
        iterator(Link node1) : node(node1)
        {
        }

        //! \a const_iterator encapsulates a pointer to a node.
        Link node;
    };

private :

    //! This structure implements a node of class \a list. It is composed by an element of type \a T and a pointer on the next node.
    struct Node
    {
        //! Constructor.
        Node(const T& data1, Link next1) : data(data1), next(next1)
        {
        }

        //! Checks if the node is at the end of the list, i.e. if the node is the sentinel node.
        bool end() const
        {
            return next == nullptr;
        }

        //! Element storage.
        T data;
        //! Pointer to the next node.
        Link next;

        //! Collects a node from the linked list memory pool and initializes it. It mimes operator \c new.
        static Link new_(const T& data1, Link next1)
        {
            // take a node from the linked list memory pool
            Link node = mem_head;
            mem_head = mem_head->next;

            // initialization
            node->data = data1;
            node->next = next1;

            return node;
        }

        //! Collects a node from the linked list memory pool and initializes it. It mimes operator \c new.
        static Link new_(const Node& copied)
        {
            // take a node from the linked list memory pool
            Link node = mem_head;
            mem_head = mem_head->next;

            // initialization
            *node = copied;

            return node;
        }

        //! Moves a node to the linked list memory pool. It mimes operator \c delete.
        static void delete_(Link node)
        {
            // put a node to the linked list memory pool
            node->next = mem_head;
            mem_head = node;
        }
    };

        ///////////////////////////////////////////////////////////////////////




    ///////////////////////////////////////////////////////////////////////////

public :

    ///////////////////// list initialization/destruction /////////////////////

    //! Constructor.
    List(unsigned int mem_pool_size1);

    //! Constructor to create a list with \a n value.
    List(unsigned int mem_pool_size1, unsigned int n, const T& value);

    //! Constructor to create a list from an array.
    List(unsigned int mem_pool_size1, const T array[], unsigned int array_length);

    //! Copy constructor.
    List(unsigned int mem_pool_size1, const List& copied);

    //! Assignment operator overloading.
    List& operator=(const List& rhs);

    //! Destructor. All the elements in the list container are dropped (including the sentinel node) : their destructors are called, and then they are removed from the list container, leaving it with a size of 0.
    ~List();

    //! All the elements in the list container are dropped (except the sentinel node) : their destructors are called, and then they are removed from the list container, leaving it with a size of 0.
    void clear();


    ///////////////////////// list moving and reading /////////////////////////

    //! Returns the head link.
    iterator begin();

    //! Returns the head link.
    const_iterator begin() const; // const overloading


    ////////////////////////////// list changes ///////////////////////////////

    //! Assigns new content to the container, dropping all the elements contained in the container object before the call and replacing them by those specified by the parameters:
    //! - the new content is the repetition \a n times of copies of element value.
    void assign(unsigned int n, const T& value);

    //! Assigns new content to the container, dropping all the elements contained in the container object before the call and replacing them by those specified by the parameters:
    //! - the new content is a copy of an array.
    void assign(const T array[], unsigned int array_length);

    //! Removes the first element in the \a list container, effectively reducing its size by one.
    void pop_front();

    //! Inserts a new element at the beginning of the list.
    void push_front(const T& value);

    //! Inserts copies of list \a copied elements at the beginning of the list \a *this.
    void push_front(const List& copied);

    //! Puts away a new element to maintain sorted list into the specified order.
    template <typename BinaryPredicate>
    void put_away(const T& value, BinaryPredicate compare);

    //! Puts away a new element to maintain sorted list into ascending order.
    void put_away(const T& value);

    //! Inserts a new element before \a position. Returns the position of the inserted element.
    iterator insert_before(iterator position, const T& value);

    //! Inserts a new element after \a position. Returns the position of the inserted element.
    iterator insert_after(iterator position, const T& value);

    //! Removes from the list container the element at \a position and returns a valid iterator, i.e. the position of the next element.
    iterator erase(iterator position);

    //! Removes from the list container the element at \a ++position and returns the position.
    iterator erase_after(iterator position);

    //! Removes an element \a moved (of another list or list \a *this) and inserts at the beginning of list \a *this. Equivalent to push_front(*it) and it = erase(it) without use of the operators \a new and \a delete.
    //! Returns a valid iterator, i.e the position of the next element.
    iterator splice_front(iterator moved);

    //! Moves all elements of list \a moved to the beginnning of list \a *this.
    void splice_front(List& moved);

    //! Removes elements by their value.
    void remove(const T& value);

    //! Removes from the container all the elements for which \a Predicate \a predicate returns \c true. This calls the destructor of these objects and reduces the container size by the number of elements removed.
    //! UnaryPredicate \a predicate can be implemented as any typed expression taking one argument of the same type as the elements container in the \a list and returning a bool (this may either be a function pointer or an object whose class implements \c operator()).
    template <typename UnaryPredicate>
    void remove_if(UnaryPredicate predicate);

    //! This version accepting a binary predicate, a specific comparison function to determine the "uniqueness" of an element can be specified.
    //! In fact, any behavior can be implemented (and not only an equality comparison), but notice that the function will call binary_pred(*i,*(i-1)) for all pairs of elements (where i is an iterator to an element, starting from the second) and remove i from the list if the predicate returns \c true.
    template <typename BinaryPredicate>
    void unique(BinaryPredicate compare);

    //! Remove consecutive duplicate values.
    void unique();

    //! Reverses the order of the elements in the \a list container.
    void reverse();

    //! Sorts the elements with a specified order. The order of equal elements is guaranteed to be preserved (stability). The sorting is performed by merge sort algorithm in \a O(n log n) in worst-case, where \a n is the size of the container. It does not require \a O(n) extra space.
    template <typename BinaryPredicate>
    void sort(BinaryPredicate compare);

    //! Sorts the elements into ascending order. Calls the function void sort(BinaryPredicate compare) with an object of the class \a less because parameter by default for a template function is not allowed.
    void sort();

    //! Exchanges the content of container \a list1 by the content of \a list2, which is another list object containing elements of the same type. Sizes may differ. Performs it in a constant time without copying.
    //! Use this function if you don't have accesss to C++11 (formerly known as C++0x) "void std::swap(T& a, T& b)" version with semantic move.
    static void swap(List& list1, List& list2);

    //////////////////////////// list information /////////////////////////////

    //! Returns whether the list container is empty, i.e. whether its size is 0.
    bool empty() const;

    //! Returns the number of elements in the list without counting the sentinel node.
    //! The complexity of this function is in \a O(n) so if you want to get often the size of the list, you should update the size in a variable after each modification.
    unsigned int size() const;

    //! \a Equal \a to operator overloading.
    template <typename U>
    friend bool operator==(const List<U>& lhs, const List<U>& rhs);

    //! \a Not \a equal \a to operator overloading.
    template <typename U>
    friend bool operator!=(const List<U>& lhs, const List<U>& rhs);

    //! Overloading of cout <<. It displays a linked list in the same way as integral-type variable.
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const List<U>& displayed);

    //! A second way to display a linked list.
    void display() const;

private :

    //! Head of the list.
    Link head;

    //! Head of the memory pool list which is shared for all lists.
    static Link mem_head;

    //! Number of nodes of the memory pool list created by list *this.
    const unsigned int mem_pool_size;

    //! Creates a memory pool with a shared linked list.
    void alloc_mem_pool();

    static unsigned int check_mem_pool_size1(unsigned int value);
};

// function object definitions for the template function parameter BinaryPredicate
template <typename T = int>
struct equal_to { bool operator()(const T& x, const T& y) const { return x == y; } };

template <typename T = int>
struct not_equal_to { bool operator()(const T& x, const T& y) const { return x != y; } };

template <typename T = int>
struct greater { bool operator()(const T& x, const T& y) const { return x > y; } };

template <typename T = int>
struct less { bool operator()(const T& x, const T& y) const { return x < y; } };

template <typename T = int>
struct greater_equal { bool operator()(const T& x, const T& y) const { return x >= y; } };

template <typename T = int>
struct less_equal { bool operator()(const T& x, const T& y) const { return x <= y; } };

// function object definition for the template function parameter UnaryPredicate
template <typename T = int>
struct predicate_example
{
    bool operator()(const T& value) const
    {
        return value > T(10) && value < T(20) && value != T(13);
    }
};

}

// list definitions
#include "list.tpp"

#endif // LIST_HPP

//! \class ofeli::list
//! This class implements a singly linked list used by the implementation of Shi and Karl' algorithm. The elements stored are integers by default that correspond of the offset of the level set function buffer.

/**
 * \example linked_list
 * \code
 * // How to use the linked list ?
 *
 * #include <iostream>
 *
 * ofeli::list<int> list; // list is empty
 * list.push_front(99); // list = 99
 * list.push_front(99); // list = 99|99
 * list.push_front(2);  // list = 2|99|99
 * list.push_front(5);  // list = 5|2|99|99
 * list.push_front(99); // list = 99|5|2|99|99
 * std::cout << list << std::endl;
 * \endcode
 * \code
 * Standard output result :
 *
 *      Linked list
 * ---------------------
 *  Position | Value
 * ---------------------
 *     1     |   99
 *     2     |   5
 *     3     |   2
 *     4     |   99
 *     5     |   99
 * ---------------------
 * \endcode
 * \code
 * list.push_front(1);  // list = 1|99|5|2|99|99
 * list.push_front(99); // list = 99|1|99|5|2|99|99
 * std::cout << list << std::endl;
 * \endcode
 * \code
 * Standard output result :
 *
 *      Linked list
 * ---------------------
 *  Position | Value
 * ---------------------
 *     1     |   99
 *     2     |   1
 *     3     |   99
 *     4     |   5
 *     5     |   2
 *     6     |   99
 *     7     |   99
 * ---------------------
 * \endcode
 * \code
 * // Removes all values of the list strictly greater than 50.
 *
 * // void begin() : the current position is the head of the list
 * // bool end() : true if the current position is the tail of the list
 *
 * for( list.begin(); !list.end(); )
 * {
 *     if( list.get_current() > 50 ) // if the value at the current position is greater than 50
 *     {
 *         list.erase(); // erase the value at the current position, the current position is the next element, now
 *     }
 *     else
 *     {
 *         ++list; // similar to "++iterator;" for the STL list, moves to the next element in the list.
 *     }
 * }
 * std::cout << list << std::endl;
 * \endcode
 * \code
 * Standard output result :
 *
 *      Linked list
 * ---------------------
 *  Position | Value
 * ---------------------
 *     1     |   1
 *     2     |   5
 *     3     |   2
 * ---------------------
 * \endcode
 * \code
 * list.clear(); // list is empty
 * list.push_front(63);
 * std::cout << list << std::endl;
 * \endcode
 * \code
 * Standard output result :
 *
 *      Linked list
 * ---------------------
 *  Position | Value
 * ---------------------
 *     1     |   63
 * ---------------------
 *
 * \endcode
 * \code
 * // If you want to read a list without modifying it. You should use const_iterator.
 * for( auto it = list.get_begin(); !it.end(); ++it ) // scan through the list
   {
       std::cout << "value = " << *it << std::endl;
   }
 * \endcode
 */

//! \struct ofeli::equal_to
//! This class defines function objects for the equality comparison operation. Generically, function objects are instances of a class with member function \c operator() defined. This member function allows the object to be used with the same syntax as a regular function call, and therefore it can be used in templates instead of a pointer to a function. \c equal_to has its \c operator() member defined such that it returns \c true if its two arguments compare equal to each other using \c operator==, and \c false otherwise. This class can be used with the template functions \c list<T>::sort(BinaryPredicate compare) and list<T>::put_away(BinaryPredicate compare).

//! \struct ofeli::not_equal_to
//! This class defines function objects for the non-equality comparison operation. Generically, function objects are instances of a class with member function \c operator() defined. This member function allows the object to be used with the same syntax as a regular function call, and therefore it can be used in templates instead of a pointer to a function. \c not_equal_to has its \c operator() member defined such that it returns \c true if its two arguments do not compare equal to each other using \c operator!=, and \c false otherwise. This class can be used with the template function \c list<T>::sort(BinaryPredicate compare).

//! \struct ofeli::greater
//! This class defines function objects for the "greater than" inequality comparison operation. Generically, function objects are instances of a class with member function \c operator() defined. This member function allows the object to be used with the same syntax as a regular function call, and therefore it can be used in templates instead of a pointer to a function. \c greater has its \c operator() member defined such that it returns \c true if its first argument compares greater than the second one using \c operator>, and \c false otherwise. This class can be used with the template function \clist<T>::sort(BinaryPredicate compare).

//! \struct ofeli::less
//! This class defines function objects for the "less than" inequality comparison operation. Generically, function objects are instances of a class with member function \c operator() defined. This member function allows the object to be used with the same syntax as a regular function call, and therefore it can be used in templates instead of a pointer to a function. \c less has its \c operator() member defined such that it returns \c true if its first argument compares lower than the second one using \c operator<, and \c false otherwise. This class can be used with the template function \c list<T>::sort(BinaryPredicate compare).

//! \struct ofeli::greater_equal
//! This class defines function objects for the "greater than or equal to" comparison operation (>=). Generically, function objects are instances of a class with member function \c operator() defined. This member function allows the object to be used with the same syntax as a regular function call, and therefore it can be used in templates instead of a pointer to a function. \c greater_equal has its \c operator() member defined such that it returns \c true if its first argument compares greater than or equal to the second one using \c operator>=, and \c false otherwise. This class can be used with the template function \c list<T>::sort(BinaryPredicate compare).

//! \struct ofeli::less_equal
//! This class defines function objects for the "less than or equal to" comparison operation (<=). Generically, function objects are instances of a class with member function \c operator() defined. This member function allows the object to be used with the same syntax as a regular function call, and therefore it can be used in templates instead of a pointer to a function. \c less_equal has its \c operator() member defined such that it returns \c true if its first argument compares lower than or equal to the second one using \c operator<=, and \c false otherwise. This class can be used with the template function \c list<T>::sort(BinaryPredicate compare).

//! \struct ofeli::predicate_example
//! This class defines function object for an example of an unary predicate. The function returns \c true if a value is between 10 and 20 or is not equal to 13 and \c false otherwise. The user can implement his own unary predicate in order to use it with the template function \c list<T>::remove_if(UnaryPredicate predicate).
