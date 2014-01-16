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

#include <cstring> // for the function "std::memcpy"
#include <cmath>

namespace ofeli
{

template <typename T>
Matrix<T>::Matrix(T* matrix_data1, unsigned int width1, unsigned int height1) : matrix_data(matrix_data1),
    width(width1), height(height1), isAllocatedHere(false)
{
    if( matrix_data1 == nullptr )
    {
        std::cerr << std::endl <<
        "Precondition, the pointer matrix_data1 must be a non-null pointer, it must be allocated."
        << std::endl;
    }
    if( width1 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, width1 must be strictly positive."
        << std::endl;
    }
    if( height1 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, height1 must be strictly positive."
        << std::endl;
    }
}

template <typename T>
Matrix<T>::Matrix(unsigned int width1, unsigned int height1) : matrix_data(new T[width1*height1]),
    width(width1), height(height1), isAllocatedHere(true)
{
    if( width1 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, width1 must be strictly positive."
        << std::endl;
    }
    if( height1 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, height1 must be strictly positive."
        << std::endl;
    }
}

template <typename T>
Matrix<T>::Matrix(unsigned int width1, unsigned int height1, const T& value) : matrix_data(new T[width1*height1]),
    width(width1), height(height1), isAllocatedHere(true)
{
    if( width1 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, width1 must be strictly positive."
        << std::endl;
    }
    if( height1 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, height1 must be strictly positive."
        << std::endl;
    }

    for( unsigned int offset = 0; offset < width*height; offset++ )
    {
        (*this)[offset] = value;
    }
}

template <typename T>
template <typename U>
Matrix<T>::Matrix(const Matrix<U>& copied) : matrix_data(new T[copied.get_width()*copied.get_height()]),
    width(copied.get_width()), height(copied.get_height()), isAllocatedHere(true)
{
    if( width == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, width must be strictly positive."
        << std::endl;
    }
    if( height == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, height must be strictly positive."
        << std::endl;
    }


    if( width  == copied.get_width() &&
        height == copied.get_height() )
    {
        if( !copied.isNull() )
        {
            std::memcpy( matrix_data,
                         copied.get_matrix_data(),
                         width*height*sizeof(T) );
        }
        else
        {
            std::cerr << std::endl <<
            "Precondition, the pointer encapsulated by the copied matrix must be a non-null pointer, it must be allocated."
            << std::endl;
        }
    }
    else
    {
        std::cerr << std::endl <<
        "Precondition, matrices *this and copied need to have the same size."
        << std::endl;
    }
}

template <typename T>
unsigned int Matrix<T>::check_kernel_length(unsigned int value)
{
    if( value < 3 )
    {
        std::cerr << std::endl <<
        "Precondition, kernel_length1 must not be less than 3. It is set to 3."
        << std::endl;
        value = 3;
    }
    else if( value % 2 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, kernel_length1 must be odd. It is decremented."
        << std::endl;
        value--;
    }

    return value;
}

template <typename T>
Matrix<T>::Matrix(unsigned int kernel_length, double standard_deviation) :
    width( check_kernel_length(kernel_length) ),
    height( check_kernel_length(kernel_length) ),
    isAllocatedHere(true)
{
    matrix_data = new T[width*height];

    if( standard_deviation < 0.000000001 )
    {
        std::cerr << std::endl <<
        "Precondition, standard_deviation is positive and must not equal to zero. It is set to 0.000000001."
        << std::endl;

        standard_deviation = 0.000000001;
    }

    const int kernel_radius = (int(width)- 1) / 2;

    for( unsigned int y = 0; y < height; y++ )
    {
        for( unsigned int x = 0; x < width; x++ )
        {
            (*this)(x,y) = T(  0.5
                                 + 100000.0 *
                                 std::exp( -( double( square(int(y)-kernel_radius)+square(int(x)-kernel_radius) ) )
                                           / (2.0*square(standard_deviation)) )
                                 );
        }
    }
}

template <typename T>
Matrix<T>& Matrix<T>::operator=(const Matrix<T>& rhs)
{
    if( this != &rhs ) // no auto-affectation
    {
        if( this->width == rhs.width && this->height == rhs.height )
        {
            if( this->matrix_data != nullptr && rhs.matrix_data != nullptr )
            {
                std::memcpy( this->matrix_data,
                             rhs.matrix_data,
                             width*height*sizeof(T) );
            }
            else
            {
                std::cerr << std::endl <<
                "Precondition, this->matrix_data or rhs.matrix_data must be a non-null pointer. It must be allocated."
                << std::endl;
            }
        }
        else
        {
            std::cerr << std::endl <<
            "Precondition, buffers pointed by this->matrix_data and rhs.matrix_data need to have the same size."
            << std::endl;
        }
    }
}

template <typename T>
Matrix<T>::~Matrix()
{
    if( isAllocatedHere )
    {
        if( matrix_data != nullptr )
        {
            delete[] matrix_data;
        }
    }
}

template <typename T>
inline bool Matrix<T>::isNull() const
{
    return !matrix_data;
}

template <typename T>
inline unsigned int Matrix<T>::get_offset(unsigned int x, unsigned int y) const
{
    if( IS_COLUMN_WISE )
    {
        return x*height+y;
    }
    else
    {
        return x+y*width;
    }
}

template <typename T>
inline void Matrix<T>::get_position(unsigned int offset, unsigned int& x, unsigned int& y) const
{
    if( IS_COLUMN_WISE )
    {
        x = offset/height;
        y = offset-x*height;
    }
    else
    {
        y = offset/width;
        x = offset-y*width;
    }
}

template <typename T>
inline const T& Matrix<T>::get_element(unsigned int offset) const
{
    return matrix_data[offset];
}

template <typename T>
inline const T& Matrix<T>::get_element(unsigned int x, unsigned int y) const
{
    return get_element( get_offset(x,y) );
}

template <typename T>
inline void Matrix<T>::set_element(unsigned int offset, T element)
{
    matrix_data[offset] = element;
}

template <typename T>
inline void Matrix<T>::set_element(unsigned int x, unsigned int y, T element)
{
    set_element( element, get_offset(x,y) );
}

template <typename U>
bool operator==(const Matrix<U>& lhs, const Matrix<U>& rhs)
{
    if( lhs.width == rhs.width && lhs.height == rhs.height )
    {
        return !( std::memcmp( lhs.matrix_data,
                               rhs.matrix_data,
                               rhs.width*rhs.height*sizeof(U) )
                );
    }
    else
    {
        return false;
    }
}

template <typename U>
bool operator!=(const Matrix<U>& lhs, const Matrix<U>& rhs)
{
    return !( lhs == rhs );
}

template <typename T>
inline T& Matrix<T>::operator[](unsigned int offset)
{
    return matrix_data[offset];
}

template <typename T>
inline const T& Matrix<T>::operator[](unsigned int offset) const
{
    return matrix_data[offset];
}

template <typename T>
inline T& Matrix<T>::operator()(unsigned int x, unsigned int y)
{
    return matrix_data[ get_offset(x,y) ];
}

template <typename T>
inline const T& Matrix<T>::operator()(unsigned int x, unsigned int y) const
{
    return matrix_data[ get_offset(x,y) ];
}

template <typename U>
std::ostream& operator<<(std::ostream& os, const Matrix<U>& displayed)
{
    os << std::endl;
    for( unsigned int y = 0; y < displayed.get_height(); y++ )
    {
        for( unsigned int x = 0; x < displayed.get_width(); x++ )
        {
            os << "[" << x << "," << y << "] = " << displayed(x,y) << "   ";
        }
        os << std::endl;
    }

    return os;
}

template <typename T>
void Matrix<T>::display() const
{
    std::cout << *this;
}

}

