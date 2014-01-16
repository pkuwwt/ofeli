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

#include "edge_based_active_contour.hpp"

namespace ofeli
{

EdgeBasedActiveContour::EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1) :
    ActiveContour(img_gradient1.get_width(), img_gradient1.get_height(),
                  true, 1.25, 1.25, 0.0, 0.0,
                  true, 5, 2.0, 30, 3),
    img_gradient(img_gradient1),
    otsu_threshold1( otsu_method(img_gradient1) ),
    isInside( find_direction_evolution() )
{
    if( !isValid_matrix(img_gradient1) )
    {
        std::cerr << std::endl <<
        "img_gradient1 is not valid."
        << std::endl;
    }
}

// delegating constructor in C++11, instead of the previous constructor but VS 2012 doesn't implement this feature
/*EdgeBasedActiveContour::EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1) :
    EdgeBasedActiveContour(img_gradient1,
               true, 1.25, 1.25, 0.0, 0.0,
               true, 5, 2.0, 30, 3)
{
}*/

EdgeBasedActiveContour::EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1,
                       bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                       bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1) :
    ActiveContour(img_gradient1.get_width(), img_gradient1.get_height(),
                  hasEllipse1, shape_width_ratio1, shape_height_ratio1, center_x_ratio1, center_y_ratio1,
                  hasCycle2_1, kernel_length1, sigma1, Na1, Ns1),
    img_gradient(img_gradient1),
    otsu_threshold1( otsu_method(img_gradient1) ),
    isInside( find_direction_evolution() )
{
    if( !isValid_matrix(img_gradient1) )
    {
        std::cerr << std::endl <<
        "img_gradient1 is not valid."
        << std::endl;
    }
}

EdgeBasedActiveContour::EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1,
                       const ofeli::Matrix<const signed char>& phi_init1,
                       bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1) :
    ActiveContour(phi_init1,
                  hasCycle2_1, kernel_length1, sigma1, Na1, Ns1),
    img_gradient(img_gradient1),
    otsu_threshold1( otsu_method(img_gradient1) ),
    isInside( find_direction_evolution() )
{
    if( !isValid_matrix(img_gradient1) )
    {
        std::cerr << std::endl <<
        "img_gradient1 is not valid."
        << std::endl;
    }
    if( !isValid_matrix(phi_init1) )
    {
        std::cerr << std::endl <<
        "phi_init1 is not valid."
        << std::endl;
    }

    if( phi_init1.get_width() != img_gradient1.get_width() )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 must have the same width of the input image img_gradient1."
        << std::endl;
    }
    if( phi_init1.get_height() != img_gradient1.get_height() )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 must have the same height of the input image img_gradient1."
        << std::endl;
    }
}

EdgeBasedActiveContour::EdgeBasedActiveContour(const EdgeBasedActiveContour& ac) :
    ActiveContour(ac),
    img_gradient(ac.img_gradient), otsu_threshold1(ac.otsu_threshold1), isInside(ac.isInside)
{
}

int EdgeBasedActiveContour::compute_external_speed_Fd(unsigned int offset)
{
    unsigned int x, y;
    phi.get_position(offset,x,y); // x and y passed by reference

    int sum_local_out, n_local_out, sum_local_in, n_local_in;

    // if high gradient value
    if( double(img_gradient[offset]) > 0.7*double(otsu_threshold1) )
    {
        sum_local_out = 0;
        n_local_out = 0;
        sum_local_in = 0;
        n_local_in = 0;

        if( y > 0 )
        {
            if( phi(x,y-1) < 0 )
            {
                sum_local_in += (unsigned int)( img_gradient(x,y-1) );
                n_local_in++;
            }
            else
            {
                sum_local_out += (unsigned int)( img_gradient(x,y-1) );
                n_local_out++;
            }
        }
        if( x > 0 )
        {
            if( phi(x-1,y) < 0 )
            {
                sum_local_in += (unsigned int)( img_gradient(x-1,y) );
                n_local_in++;
            }
            else
            {
                sum_local_out += (unsigned int)( img_gradient(x-1,y) );
                n_local_out++;
            }
        }
        if( x < img_gradient.get_width()-1 )
        {
            if( phi(x+1,y) < 0 )
            {
                sum_local_in += (unsigned int)( img_gradient(x+1,y) );
                n_local_in++;
            }
            else
            {
                sum_local_out += (unsigned int)( img_gradient(x+1,y) );
                n_local_out++;
            }
        }
        if( y < img_gradient.get_height()-1 )
        {
            if( phi(x,y+1) < 0 )
            {
                sum_local_in += (unsigned int)( img_gradient(x,y+1) );
                n_local_in++;
            }
            else
            {
                sum_local_out += (unsigned int)( img_gradient(x,y+1) );
                n_local_out++;
            }
        }

        return sum_local_out*n_local_in - sum_local_in*n_local_out;
    }
    else // if low gradient value
    {
        if( isInside ) // if high gradient values are inside the curve
        {
            return -1; // inward movement
        }
        else
        {
            return 1; // outward movement
        }
    }
}

// to find the high gradient values and to converge
unsigned char EdgeBasedActiveContour::otsu_method(const ofeli::Matrix<const unsigned char>& img)
{
    unsigned int histogram[256];

    for( unsigned char I = 0; I <= 255; I++ )
    {
        histogram[I] = 0;
    }

    const unsigned int img_size = img.get_width()*img.get_height();

    for( unsigned int offset = 0; offset < img_size; offset++ )
    {
        histogram[ img[offset] ]++;
    }

    unsigned int sum = 0;
    for( unsigned char I = 0; I <= 255; I++ )
    {
        sum += I*histogram[I];
    }

    unsigned int weight1, weight2, sum1;
    double mean1, mean2, var_t, var_max;

    unsigned char threshold = 127; // value returned in the case of an totally homogeneous image

    weight1 = 0;
    sum1 = 0;
    var_max = -1.0;

    // 256 values ==> 255 thresholds t evaluated
    // class1 <= t and class2 > t
    for( unsigned char t = 0; t < 255; t++ )
    {
        weight1 += histogram[t];
        if( weight1 == 0 )
        {
            continue;
        }

        weight2 = img_size-weight1;
        if( weight2 == 0 )
        {
            break;
        }

        sum1 += t*histogram[t];

        mean1 = double(sum1/weight1);
        mean2 = double( (sum-sum1)/weight2 ); // sum2 = sum-sum1

        var_t = double(weight1)*double(weight2)*(mean1-mean2)*(mean1-mean2);

        if( var_t > var_max )
        {
            var_max = var_t;
            threshold = t;
        }
    }

    return threshold;
}

bool EdgeBasedActiveContour::find_direction_evolution()
{
    unsigned int sum_in = 0;
    unsigned int sum_out = 0;

    const unsigned int img_size = img_gradient.get_width()*img_gradient.get_height();

    for( unsigned int offset = 0; offset < img_size; offset++ )
    {
        if( phi[offset] < 0 )
        {
            sum_in += (unsigned int)(img_gradient[offset]);
        }
        else
        {
            sum_out += (unsigned int)(img_gradient[offset]);
        }
    }

    return sum_in > sum_out;
}

void EdgeBasedActiveContour::initialize_for_each_frame()
{
    ActiveContour::initialize_for_each_frame();

    isInside = find_direction_evolution();
}

}
