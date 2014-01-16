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

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream> // for operator<< overloading


// false or true below, respectively,
// if you want to use a matrix with a row/column wise data buffer
#ifndef IS_COLUMN_WISE
#define IS_COLUMN_WISE false
#endif
// functions affected by the define :
// - int get_offset(unsigned int x, unsigned int y) const
// - void get_position(unsigned int offset, unsigned int& x, unsigned int& y) const

namespace ofeli
{

template <typename T>
class Matrix
{

public :

    //! Constructor.
    Matrix(T* matrix_data1, unsigned int width1, unsigned int height1);

    //! Constructor.
    Matrix(unsigned int width1, unsigned int height1);

    //! Constructor.
    Matrix(unsigned int width1, unsigned int height1, const T& value);

    //! Creates a normalised gaussian kernel without to divide by \a π.
    Matrix(unsigned int kernel_length, double standard_deviation);

    //! Copy constructor.
    template <typename U>
    Matrix(const Matrix<U>& copied);

    //! Assignment operator overloading.
    Matrix& operator=(const Matrix& rhs);

    //! Destructor.
    ~Matrix();

    bool isNull() const;

    unsigned int get_offset(unsigned int x, unsigned int y) const;
    void get_position(unsigned int offset, unsigned int& x, unsigned int& y) const;

    const T& get_element(unsigned int offset) const;
    const T& get_element(unsigned int x, unsigned int y) const;

    void set_element(unsigned int offset, T element);
    void set_element(unsigned int x, unsigned int y, T element);

    //! \a Equal \a to operator overloading.
    template <typename U>
    friend bool operator==(const Matrix<U>& lhs, const Matrix<U>& rhs);

    //! \a Not \a equal \a to operator overloading.
    template <typename U>
    friend bool operator!=(const Matrix<U>& lhs, const Matrix<U>& rhs);

    T& operator[](unsigned int offset);
    const T& operator[](unsigned int offset) const;
    T& operator()(unsigned int x, unsigned int y);
    const T& operator()(unsigned int x, unsigned int y) const;

    //! Overloading of cout <<. It displays a linked list in the same way as integral-type variable.
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const Matrix<U>& displayed);

    //! A second way to display a linked list.
    void display() const;

    const T* get_matrix_data() const { return matrix_data; }
    unsigned int get_width() const { return width; }
    unsigned int get_height() const { return height; }

private :

    //! Matrix data.
    T* matrix_data;

    //! Width of the matrix, i.e. number of columns.
    const unsigned int width;

    //! Height of the matrix, i.e. number of rows.
    const unsigned int height;

    const bool isAllocatedHere;

    //! Gives the square of a value.
    template <typename U>
    static U square(const U& value);

    static unsigned int check_kernel_length(unsigned int value);
};

template <typename T>
template <typename U>
inline U Matrix<T>::square(const U& value)
{
    return value*value;
}

}

// list definitions
#include "matrix.tpp"

#endif // MATRIX_HPP
