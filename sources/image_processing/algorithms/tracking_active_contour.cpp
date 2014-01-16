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

#include "tracking_active_contour.hpp"

namespace ofeli
{

ACwithoutEdgesYUVcamera::ACwithoutEdgesYUVcamera(const QImage& img_rgb1) :
    ActiveContour(img_rgb1.width(), img_rgb1.height(),
                  true, 0.65, 0.65, 0.0, 0.0,
                  true, 5, 2.0, 30, 3),
    img_rgb(img_rgb1),
    lambda_out(1), lambda_in(1), alpha(1), beta(10), gamma(10)
{
    if( img_rgb1.isNull() )
    {
        std::cerr << std::endl <<
        "Precondition, the pointer encapsulated by img_rgb1 must be a non-null pointer, it must be allocated."
        << std::endl;
    }

    if( img_rgb1.width() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, img_rgb1 width must be strictly positive."
        << std::endl;
    }
    if( img_rgb1.height() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, img_rgb1 height must be strictly positive."
        << std::endl;
    }

    initialize_sums();
    calculate_means();
}

// delegating constructor in C++11, instead of the previous constructor but VS 2012 doesn't implement this feature
/*ACwithoutEdgesYUVcamera::ACwithoutEdgesYUVcamera(const QImage& img_rgb1) :
    ACwithoutEdgesYUVcamera(img_rgb1,
                      true, 0.65, 0.65, 0.0, 0.0,
                      true, 5, 2.0, 30, 3,
                      1, 1, 1, 10, 10)
{
}*/

ACwithoutEdgesYUVcamera::ACwithoutEdgesYUVcamera(const QImage& img_rgb1,
                                     bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                                     bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                                     int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1) :
    ActiveContour(img_rgb1.width(), img_rgb1.height(),
                  hasEllipse1, shape_width_ratio1, shape_height_ratio1, center_x_ratio1, center_y_ratio1,
                  hasCycle2_1, kernel_length1, sigma1, Na1, Ns1),
    img_rgb(img_rgb1),
    lambda_out(check_value(lambda_out1)), lambda_in(check_value(lambda_in1)), alpha(check_value(alpha1)), beta(check_value(beta1)), gamma(check_value(gamma1))
{
    if( img_rgb1.isNull() )
    {
        std::cerr << std::endl <<
        "Precondition, the pointer encapsulated by img_rgb1 must be a non-null pointer, it must be allocated."
        << std::endl;
    }

    if( img_rgb1.width() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, img_rgb1 width must be strictly positive."
        << std::endl;
    }
    if( img_rgb1.height() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, img_rgb1 height must be strictly positive."
        << std::endl;
    }

    initialize_sums();
    calculate_means();
}

ACwithoutEdgesYUVcamera::ACwithoutEdgesYUVcamera(const QImage& img_rgb1,
                                     const ofeli::Matrix<const signed char>& phi_init1,
                                     bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                                     int lambda_out1, int lambda_in1, int alpha1,int beta1, int gamma1) :
    ActiveContour(phi_init1,
                  hasCycle2_1, kernel_length1, sigma1, Na1, Ns1),
    img_rgb(img_rgb1),
    lambda_out(check_value(lambda_out1)), lambda_in(check_value(lambda_in1)), alpha(check_value(alpha1)), beta(check_value(beta1)), gamma(check_value(gamma1))
{
    if( img_rgb1.isNull() )
    {
        std::cerr << std::endl <<
        "Precondition, the pointer encapsulated by img_rgb1 must be a non-null pointer, it must be allocated."
        << std::endl;
    }

    if( img_rgb1.width() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, img_rgb1 width must be strictly positive."
        << std::endl;
    }
    if( img_rgb1.height() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, img_rgb1 height must be strictly positive."
        << std::endl;
    }

    if( phi_init1.isNull() )
    {
        std::cerr << std::endl <<
        "Precondition, the pointer encapsulated by phi_init1 must be a non-null pointer, it must be allocated."
        << std::endl;
    }

    if( phi_init1.get_width() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 width must be strictly positive."
        << std::endl;
    }
    if( phi_init1.get_height() == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 height must be strictly positive."
        << std::endl;
    }

    if( phi_init1.get_width() != (unsigned int)(img_rgb1.width()) )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 must have the same width of the input image img_rgb1."
        << std::endl;
    }
    if( phi_init1.get_height() != (unsigned int)(img_rgb1.height()) )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 must have the same height of the input image img_rgb1."
        << std::endl;
    }

    initialize_sums();
    calculate_means();
}

ACwithoutEdgesYUVcamera::ACwithoutEdgesYUVcamera(const ACwithoutEdgesYUVcamera& ac) :
    ActiveContour(ac),
    img_rgb(ac.img_rgb),
    lambda_out(ac.lambda_out), lambda_in(ac.lambda_in), alpha(ac.alpha), beta(ac.beta), gamma(ac.gamma),
    n_out(ac.n_out), n_in(ac.n_in)
{
    for( int index = 0; index < 3; index++ )
    {
        Cout_YUV[index] = ac.Cout_YUV[index];
        Cin_YUV[index] = ac.Cin_YUV[index];

        Cout_RGB[index] = ac.Cout_RGB[index];
        Cin_RGB[index] = ac.Cin_RGB[index];

        sum_out_RGB[index] = ac.sum_out_RGB[index];
        sum_in_RGB[index] = ac.sum_in_RGB[index];

        RGB[index] = ac.RGB[index];
    }
}

void ACwithoutEdgesYUVcamera::initialize_sums()
{
    for( int index = 0; index < 3; index++ )
    {
        sum_out_RGB[index] = 0;
        sum_in_RGB[index] = 0;
    }

    n_in = 0;
    n_out = 0;

    const unsigned int phi_size = phi.get_width()*phi.get_height();

    for( unsigned int offset = 0; offset < phi_size; offset++ )
    {
        RGB_from_buffer(offset);

        if( phi[offset] > 0 )
        {
            for( int index = 0; index < 3; index++ )
            {
                sum_out_RGB[index] += RGB[index];
            }

            n_out++;
        }
        else
        {
            for( int index = 0; index < 3; index++ )
            {
                sum_in_RGB[index] += RGB[index];
            }

            n_in++;
        }
    }
}

void ACwithoutEdgesYUVcamera::calculate_means()
{
    if( n_out != 0 )
    {
        for( int index = 0; index < 3; index++ )
        {
            Cout_RGB[index] = sum_out_RGB[index]/n_out;
        }
    }

    if( n_in != 0 )
    {
        for( int index = 0; index < 3; index++ )
        {
            Cin_RGB[index] = sum_in_RGB[index]/n_in;
        }
    }

    calculate_YUV(Cout_RGB, Cout_YUV);
    calculate_YUV(Cin_RGB, Cin_YUV);
}

int ACwithoutEdgesYUVcamera::compute_external_speed_Fd(unsigned int offset)
{
    RGB_from_buffer(offset);

    int YUV[3];
    calculate_YUV(RGB, YUV);

    return    lambda_out*(   alpha*square(YUV[0]-Cout_YUV[0])
                           + beta*square(YUV[1]-Cout_YUV[1])
                           + gamma*square(YUV[2]-Cout_YUV[2]) )

            - lambda_in*(   alpha*square(YUV[0]-Cin_YUV[0])
                          + beta*square(YUV[1]-Cin_YUV[1])
                          + gamma*square(YUV[2]-Cin_YUV[2]) );
}

void ACwithoutEdgesYUVcamera::updates_for_means_in1()
{
    for( int index = 0; index < 3; index++ )
    {
        sum_out_RGB[index] -= RGB[index];
        sum_in_RGB[index] += RGB[index];
    }

    n_out--;
    n_in++;
}

void ACwithoutEdgesYUVcamera::updates_for_means_out1()
{
    for( int index = 0; index < 3; index++ )
    {
        sum_in_RGB[index] -= RGB[index];
        sum_out_RGB[index] += RGB[index];
    }

    n_in--;
    n_out++;
}

void ACwithoutEdgesYUVcamera::updates_for_means_in2(unsigned int offset)
{
    RGB_from_buffer(offset);

    updates_for_means_in1();
}

void ACwithoutEdgesYUVcamera::updates_for_means_out2(unsigned int offset)
{
    RGB_from_buffer(offset);

    updates_for_means_out1();
}

void ACwithoutEdgesYUVcamera::initialize_for_each_frame()
{
    ActiveContour::initialize_for_each_frame();

    initialize_sums();
    calculate_means();
}

void ACwithoutEdgesYUVcamera::initialize_for_each_frame_set_buffer(const QImage& img_rgb1)
{
    img_rgb = img_rgb1;
    initialize_for_each_frame();
}

}
