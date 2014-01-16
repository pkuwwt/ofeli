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

#include "region_based_active_contour.hpp"

namespace ofeli
{

RegionBasedActiveContour::RegionBasedActiveContour(const ofeli::Matrix<const unsigned char>& img1) :
    ActiveContour(img1.get_width(), img1.get_height(),
                  true, 0.65, 0.65, 0.0, 0.0,
                  true, 5, 2.0, 30, 3),
    img(img1), lambda_out(1), lambda_in(1)
{
    if( !isValid_matrix(img1) )
    {
        std::cerr << std::endl <<
        "img1 is not valid."
        << std::endl;
    }

    initialize_sums();
    calculate_means();
}

// delegating constructor in C++11, instead of the previous constructor but VS 2012 doesn't implement this feature
/*RegionBasedActiveContour::RegionBasedActiveContour(const ofeli::Matrix<const unsigned char>& img1) :
    RegionBasedActiveContour(const ofeli::Matrix<const unsigned char>& img1,
                   true, 0.65, 0.65, 0.0, 0.0,
                   true, 5, 2.0, 30, 3,
                   1, 1)
{
}*/

RegionBasedActiveContour::RegionBasedActiveContour(const ofeli::Matrix<const unsigned char>& img1,
                               bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                               bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                               int lambda_out1, int lambda_in1) :
    ActiveContour(img1.get_width(), img1.get_height(),
                  hasEllipse1, shape_width_ratio1, shape_height_ratio1, center_x_ratio1, center_y_ratio1,
                  hasCycle2_1, kernel_length1, sigma1, Na1, Ns1),
    img(img1), lambda_out(check_value(lambda_out1)), lambda_in(check_value(lambda_in1))
{
    if( !isValid_matrix(img1) )
    {
        std::cerr << std::endl <<
        "img1 is not valid."
        << std::endl;
    }

    initialize_sums();
    calculate_means();
}

RegionBasedActiveContour::RegionBasedActiveContour(const ofeli::Matrix<const unsigned char>& img1,
                               const ofeli::Matrix<const signed char>& phi_init1,
                               bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                               int lambda_out1, int lambda_in1) :
    ActiveContour(phi_init1,
                  hasCycle2_1, kernel_length1, sigma1, Na1, Ns1),
    img(img1), lambda_out(check_value(lambda_out1)), lambda_in(check_value(lambda_in1))
{
    if( !isValid_matrix(img1) )
    {
        std::cerr << std::endl <<
        "img1 is not valid."
        << std::endl;
    }
    if( !isValid_matrix(phi_init1) )
    {
        std::cerr << std::endl <<
        "phi_init1 is not valid."
        << std::endl;
    }

    if( phi_init1.get_width() != img1.get_width() )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 must have the same width of the input image img1."
        << std::endl;
    }
    if( phi_init1.get_height() != img1.get_height() )
    {
        std::cerr << std::endl <<
        "Precondition, phi_init1 must have the same height of the input image img1."
        << std::endl;
    }

    initialize_sums();
    calculate_means();
}

RegionBasedActiveContour::RegionBasedActiveContour(const RegionBasedActiveContour& ac) :
    ActiveContour(ac),
    img(ac.img), lambda_out(ac.lambda_out), lambda_in(ac.lambda_in),
    Cout(ac.Cout), Cin(ac.Cin),
    sum_out(ac.sum_out), sum_in(ac.sum_in), n_out(ac.n_out), n_in(ac.n_in),
    I(ac.I)
{
}

void RegionBasedActiveContour::initialize_sums()
{
    sum_in = 0;
    sum_out = 0;
    n_in = 0;
    n_out = 0;

    const unsigned int img_size = img.get_width()*img.get_height();

    for( unsigned int offset = 0; offset < img_size; offset++ )
    {
        if( phi[offset] > 0 )
        {
            sum_out += int(img[offset]);
            n_out++;
        }
        else
        {
            sum_in += int(img[offset]);
            n_in++;
        }
    }
}

void RegionBasedActiveContour::calculate_means()
{
    if( n_out != 0 )
    {
        Cout = sum_out/n_out;
    }

    if( n_in != 0 )
    {
        Cin = sum_in/n_in;
    }
}

int RegionBasedActiveContour::compute_external_speed_Fd(unsigned int offset)
{
    I = int(img[offset]); // intensity of the current pixel

    return lambda_out*square(I-Cout) - lambda_in*square(I-Cin);
}

void RegionBasedActiveContour::updates_for_means_in1()
{
    sum_out -= I;
    n_out--;

    sum_in += I;
    n_in++;
}

void RegionBasedActiveContour::updates_for_means_out1()
{
    sum_in -= I;
    n_in--;

    sum_out += I;
    n_out++;
}

void RegionBasedActiveContour::updates_for_means_in2(unsigned int offset)
{
    I = int(img[offset]); // intensity of the current pixel

    updates_for_means_in1();
}

void RegionBasedActiveContour::updates_for_means_out2(unsigned int offset)
{
    I = int(img[offset]); // intensity of the current pixel

    updates_for_means_out1();
}

void RegionBasedActiveContour::initialize_for_each_frame()
{
    ActiveContour::initialize_for_each_frame();

    initialize_sums();
    calculate_means();
}

}
