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

#include "filters.hpp"

#include <random> // random generator and normal/uniform distributions
#include <chrono> // to generate a seed for the random generator
#include <functional> // for function "std::bind" to link a generator and a distribution

#include <cmath> // for function "std::exp"
#include <cstring> // for function "std::memcpy"
#include <iostream> // for object "std::cerr"

namespace ofeli
{

Filters::Filters(const unsigned char* img_data1, int img_width1, int img_height1, int byte_per_pixel1) :
    img_data(img_data1), img_width(img_width1), img_height(img_height1), img_size(img_width1*img_height1), byte_per_pixel(byte_per_pixel1),
    filtered( new unsigned char[byte_per_pixel1*img_width1*img_height1] ),
    filtered_modif( new unsigned char[byte_per_pixel1*img_width1*img_height1] ),
    previous_filtered( new unsigned char[byte_per_pixel1*img_width1*img_height1] ),
    diff_img( new double[img_width1*img_height1] ),
    diff_img1( new double[img_width1*img_height1] ),
    columns_histo( new int [256*img_width1] )
{
    if( img_data1 == nullptr )
    {
        std::cerr << std::endl <<
        "The pointer img_data1 must be a non-null pointer, it must be allocated."
        << std::endl;
    }

    if( byte_per_pixel == 3 )
    {
        gradient = new unsigned char[img_size];
    }

    initialyze_filtered();
}

void Filters::initialyze_filtered()
{
    std::memcpy(filtered,img_data,byte_per_pixel*img_size);
}

Filters::~Filters()
{
    // buffer deleted for the Perreault's algorithm
    delete[] columns_histo;

    // buffers deleted for the anisotropic filter
    delete[] diff_img1;
    delete[] diff_img;

    if( byte_per_pixel == 3 )
    {
        delete[] gradient;
    }

    delete[] previous_filtered;
    delete[] filtered_modif;
    delete[] filtered;
}

void Filters::anisotropic_diffusion(int max_itera, double lambda, double kappa, int option)
{
    if( kappa < 0.000000001 )
    {
        kappa = 0.000000001;
    }

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        // initial condition of the PDE
        for( int offset = 0; offset < img_size; offset++ )
        {
            diff_img[offset] = double( filtered[byte_per_pixel*offset+color_channel] );
        }

        // euclidian distance square of the neighbor pixels to the center
        //const double dist_sqr[8] = { dd*dd, dy*dy, dd*dd, dx*dx, dx*dx, dd*dd, dy*dy, dd*dd };
        const double dist_sqr[8] = { 2.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 2.0 };

        // index for 0 to 7 to access of the 8 gradients and the 8 diffusion coefficients
        int index;
        // sum calculated on the 8 directions for the PDE solving
        double sigma;

        // 8 gradients for each direction for the current pixel n
        double nabla[8];

        // intensity of the current central pixel
        double centrI;

        // coefficient of diffusion for each direction
        double c;

        // temporary pointer to swap the pointers of diffused images
        double* ptemp_aniso;

        int x, y; // position of the current pixel


        // choice of the function of diffusion
        switch( option )
        {
        // c(x,y,t) = exp(-(nabla/kappa)^2)
        case 1 :
        {
            // anisotropic diffusion
            for( int itera = 0; itera < max_itera; itera++ )
            {
                // finite differences
                for( int offset = 0; offset < img_size; offset++ )
                {
                    y = offset/img_width;
                    x = offset-y*img_width;

                    centrI = diff_img[offset];

                    index = 0;

                    // if the current pixel is not in the border
                    if( x > 0 && x < img_width-1 && y > 0 && y < img_height-1 )
                    {
                        // no tests of neighbors
                        for( int dy = -1; dy <= 1; dy++ )
                        {
                            for( int dx = -1; dx <= 1; dx++ )
                            {
                                if( !( dx == 0 && dy == 0 ) )
                                {
                                    nabla[ index++ ] = diff_img[ (x+dx)+(y+dy)*img_width ]-centrI;
                                }
                            }
                        }
                    }

                    else
                    {
                        for( int dy = -1; dy <= 1; dy++ )
                        {
                            for( int dx = -1; dx <= 1; dx++ )
                            {
                                if( !( dx == 0 && dy == 0 ) )
                                {
                                    if( x+dx >= 0 && x+dx < img_width && y+dy >= 0 && y+dy < img_height )
                                    {
                                        nabla[ index++ ] = diff_img[ (x+dx)+(y+dy)*img_width ]-centrI;
                                    }
                                    else
                                    {
                                        nabla[ index++ ] = 0.0;
                                    }
                                }
                            }
                        }

                    }

                    sigma = 0.0;
                    for( index = 0; index < 8; index++ )
                    {
                        c = std::exp( -square(nabla[index]/kappa) );

                        sigma += ( 1.0/(dist_sqr[index]) )*c*nabla[index];
                    }

                    // solution of the discret PDE
                    diff_img1[offset] = centrI+lambda*sigma;

                }

                // swap pointers
                ptemp_aniso = diff_img;
                diff_img = diff_img1;
                diff_img1 = ptemp_aniso;
            }

            break;
        }

        // c(x,y,t) = 1/(1+(1+(nabla/kappa)^2)
        case 2 :
        {
            // anisotropic diffusion
            for( int itera = 0; itera < max_itera; itera++ )
            {
                // finite differences
                for( int offset = 0; offset < img_size; offset++ )
                {
                    y = offset/img_width;
                    x = offset-y*img_width;

                    centrI = diff_img[offset];

                    // if the current pixel is not in the border
                    if( x > 0 && x < img_width-1 && y > 0 && y < img_height-1 )
                    {
                        // no tests of neighbors
                        index = 0;
                        for( int dy = -1; dy <= 1; dy++ )
                        {
                            for( int dx = -1; dx <= 1; dx++ )
                            {
                                if( !( dx == 0 && dy == 0 ) )
                                {
                                    nabla[ index++ ] = diff_img[ (x+dx)+(y+dy)*img_width ]-centrI;
                                }
                            }
                        }
                    }

                    else
                    {
                        index = 0;
                        for( int dy = -1; dy <= 1; dy++ )
                        {
                            for( int dx = -1; dx <= 1; dx++ )
                            {
                                if( !( dx == 0 && dy == 0 ) )
                                {
                                    if( x+dx >= 0 && x+dx < img_width && y+dy >= 0 && y+dy < img_height )
                                    {
                                        nabla[ index++ ] = diff_img[ (x+dx)+(y+dy)*img_width ]-centrI;
                                    }
                                    else
                                    {
                                        nabla[ index++ ] = 0.0;
                                    }
                                }
                            }
                        }

                    }

                    sigma = 0.0;
                    for( index = 0; index < 8; index++ )
                    {
                        c = 1.0/( 1.0+square(nabla[index]/kappa) );

                        sigma += ( 1.0/(dist_sqr[index]) )*c*nabla[index];
                    }

                    // solution of the discret PDE
                    diff_img1[offset] = centrI+lambda*sigma;

                }

                // swap pointers
                ptemp_aniso = diff_img;
                diff_img = diff_img1;
                diff_img1 = ptemp_aniso;
            }
        }
        }

        // result
        for( int offset = 0; offset < img_size; offset++ )
        {
            filtered[byte_per_pixel*offset+color_channel] = (unsigned char)(diff_img[offset]);
        }

    }
}

void Filters::morphological_gradient(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length-1)/2;

    int x, y; // position of the current pixel

    unsigned char max, min;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            // initialization
            max = 0;
            min = 255;

            // if the current pixel is not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // no tests of neighbors
                        if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] > max )
                        {
                            max = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                        }
                        if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] < min )
                        {
                            min = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                        }
                    }
                }
            }
            // if in the border
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // neighbors tests
                        if( x+dx >= 0 && x+dx < img_width && y+dy >= 0 && y+dy < img_height )
                        {
                            if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] > max )
                            {
                                max = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                            }
                            if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] < min )
                            {
                                min = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                            }
                        }
                    }
                }
            }
            filtered_modif[byte_per_pixel*offset+color_channel] = max-min;
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

void Filters::morphological_gradient_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length-1)/2;

    int I;
    unsigned char max, min;

    int x, y; // position of the current pixel

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = 0; y < kernel_radius+1; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the maximum value in the kernel histogram
                    for( max = 255; kernel_histo[max] == 0; max-- )
                    {
                    }

                    // to find the minimum value in the kernel histogram
                    for( min = 0; kernel_histo[min] == 0; min++ )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = max-min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max-min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max-min;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = kernel_radius+1; y <= img_height-kernel_radius-2; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the maximum value in the kernel histogram
                    for( max = 255; kernel_histo[max] == 0; max-- )
                    {
                    }

                    // to find the minimum value in the kernel histogram
                    for( min = 0; kernel_histo[min] == 0; min++ )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = max-min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max-min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max-min;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for( y = img_height-kernel_radius-1; y < img_height; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the maximum value in the kernel histogram
                    for( max = 255; kernel_histo[max] == 0; max-- )
                    {
                    }

                    // to find the minimum value in the kernel histogram
                    for( min = 0; kernel_histo[min] == 0; min++ )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = max-min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max-min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max-min;
            }
        }
    }

    if( kernel_length != 1 )
    {
        // swap pointers
        unsigned char* ptemp = filtered;
        filtered = filtered_modif;
        filtered_modif = ptemp;
    }
}

void Filters::dilation(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length-1)/2;

    int x, y; // position of the current pixel

    unsigned char max;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            // initialisation
            max = 0;

            // if the current pixel is not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // no neighbors tests
                        if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] > max )
                        {
                            max = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                        }
                    }
                }
            }
            // if in the border
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // neighbors tests
                        if( x+dx >= 0 && x+dx < img_width && y+dy >= 0 && y+dy < img_height )
                        {
                            if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] > max )
                            {
                                max = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                            }
                        }
                    }
                }

            }
            filtered_modif[byte_per_pixel*offset+color_channel] = max;
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

void Filters::dilation_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length-1)/2;

    int I;
    unsigned char max;

    int x, y; // position of the current pixel

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = 0; y < kernel_radius+1; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the maximum value in the kernel histogram
                    for( max = 255; kernel_histo[max] == 0; max-- )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = max;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = kernel_radius+1; y <= img_height-kernel_radius-2; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the maximum value in the kernel histogram
                    for( max = 255; kernel_histo[max] == 0; max-- )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = max;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for( y = img_height-kernel_radius-1; y < img_height; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the maximum value in the kernel histogram
                    for( max = 255; kernel_histo[max] == 0; max-- )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = max;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the maximum value in the kernel histogram
                for( max = 255; kernel_histo[max] == 0; max-- )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = max;
            }
        }

    }

    if( kernel_length != 1 )
    {
        // swap pointers
        unsigned char* ptemp = filtered;
        filtered = filtered_modif;
        filtered_modif = ptemp;
    }
}

void Filters::erosion(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length-1)/2;

    int x, y; // position of the current pixel

    unsigned char min;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            // initialization
            min = 255;

            // if the current pixel is not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // no neighbors tests
                        if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] < min )
                        {
                            min = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                        }
                    }
                }
            }
            // if in the border
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // neighbors tests
                        if( x+dx >= 0 && x+dx < img_width && y+dy >= 0 && y+dy < img_height )
                        {
                            if( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] < min )
                            {
                                min = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                            }
                        }
                    }
                }
            }
            filtered_modif[byte_per_pixel*offset+color_channel] = min;
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

void Filters::erosion_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length-1)/2;

    int I;
    unsigned char min;

    int x, y; // position of the current pixel

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = 0; y < kernel_radius+1; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the minimum value in the kernel histogram
                    for( min = 0; kernel_histo[min] == 0; min++ )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I]-columns_histo[256*(x-kernel_radius-1)+I ];
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = min;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = kernel_radius+1; y <= img_height-kernel_radius-2; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the minimum value in the kernel histogram
                    for( min = 0; kernel_histo[min] == 0; min++ )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = min;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for( y = img_height-kernel_radius-1; y < img_height; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the minimum value in the kernel histogram
                    for( min = 0; kernel_histo[min] == 0; min++ )
                    {
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = min;
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = min;
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the minimum value in the kernel histogram
                for( min = 0; kernel_histo[min] == 0; min++ )
                {
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = min;
            }
        }
    }

    if( kernel_length != 1 )
    {
        // swap pointers
        unsigned char* ptemp = filtered;
        filtered = filtered_modif;
        filtered_modif = ptemp;
    }
}

void Filters::closing(int kernel_length)
{
    dilation(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif;
    filtered_modif = previous_filtered;
    previous_filtered = ptemp;

    erosion(kernel_length);
}

void Filters::closing_o1(int kernel_length)
{
    dilation_o1(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif;
    filtered_modif = previous_filtered;
    previous_filtered = ptemp;

    erosion_o1(kernel_length);
}

void Filters::opening(int kernel_length)
{
    erosion(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif;
    filtered_modif = previous_filtered;
    previous_filtered = ptemp;

    dilation(kernel_length);
}

void Filters::opening_o1(int kernel_length)
{
    erosion_o1(kernel_length);

    // for the top-hat function eventually
    unsigned char* ptemp = filtered_modif;
    filtered_modif = previous_filtered;
    previous_filtered = ptemp;

    dilation_o1(kernel_length);
}

void Filters::black_top_hat(int kernel_length)
{
    closing(kernel_length);

    for( int offset = 0; offset < byte_per_pixel*img_size; offset++ )
    {
        if( filtered[offset] >= previous_filtered[offset] )
        {
            filtered[offset] -= previous_filtered[offset];
        }
        else
        {
            filtered[offset] = 0;
        }
    }
}

void Filters::black_top_hat_o1(int kernel_length)
{
    closing_o1(kernel_length);

    for( int offset = 0; offset < byte_per_pixel*img_size; offset++ )
    {
        if( filtered[offset] >= previous_filtered[offset] )
        {
            filtered[offset] -= previous_filtered[offset];
        }
        else
        {
            filtered[offset] = 0;
        }
    }
}

void Filters::white_top_hat(int kernel_length)
{
    opening(kernel_length);

    for( int offset = 0; offset < byte_per_pixel*img_size; offset++ )
    {
        if( previous_filtered[offset] >= filtered[offset] )
        {
            filtered[offset] = previous_filtered[offset]-filtered[offset];
        }
        else
        {
            filtered[offset] = 0;
        }
    }
}

void Filters::white_top_hat_o1(int kernel_length)
{
    opening_o1(kernel_length);

    for( int offset = 0; offset < byte_per_pixel*img_size; offset++ )
    {
        if( previous_filtered[offset] >= filtered[offset] )
        {
            filtered[offset] = previous_filtered[offset]-filtered[offset];
        }
        else
        {
            filtered[offset] = 0;
        }
    }
}

void Filters::mean_filtering(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length-1)/2;
    int sum;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            sum = 0;

            // if the current pixel is not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        sum += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
            }
            // if in the border
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        if( (x+dx >= 0 && x+dx < img_width) && (y+dy >= 0 && y+dy < img_height) )
                        {
                            sum += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                        }
                        if( (x+dx >= 0 && x+dx < img_width) && ( !(y+dy >= 0 && y+dy < img_height) ) )
                        {
                            sum += int( filtered[ byte_per_pixel*((x+dx)+(y-dy)*img_width)+color_channel ] );
                        }
                        if( ( !(x+dx >= 0 && x+dx < img_width) ) && (y+dy >= 0 && y+dy < img_height) )
                        {
                            sum += int( filtered[ byte_per_pixel*((x-dx)+(y+dy)*img_width)+color_channel ] );
                        }
                        if( ( !(x+dx >= 0 && x+dx < img_width) ) && ( !(y+dy >= 0 && y+dy < img_height) ) )
                        {
                            sum += int( filtered[ byte_per_pixel*((x-dx)+(y-dy)*img_width)+color_channel ] );
                        }
                    }
                }
            }
            filtered_modif[byte_per_pixel*offset+color_channel] = (unsigned char)(sum/(kernel_length*kernel_length));
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

const double* Filters::gaussian_kernel(int kernel_length, double sigma)
{
    // to protect the input : kernel_length impair et strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    // to protect against /0
    if( sigma < 0.000000001 )
    {
        sigma = 0.000000001;
    }

    const int kernel_size = kernel_length*kernel_length;

    double* kernel = new double[kernel_size];

    int x;
    int y;
    const int k = (kernel_length-1)/2;

    double sum = 0.0;

    for( int offset = 0; offset < kernel_size; offset++ )
    {
        y = offset/kernel_length;
        x = offset-y*kernel_length;

        kernel[offset] = std::exp( - ( (double(y)-double(k))*(double(y)-double(k)) + (double(x)-double(k))*(double(x)-double(k)) ) / (2*sigma*sigma)   );

        sum += kernel[offset];
    }

    // to normalize
    for( int offset = 0; offset < kernel_size; offset++ )
    {
        kernel[offset] = kernel[offset]/sum;
    }

    return kernel;
}

void Filters::gaussian_filtering(int kernel_length, double sigma)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    // to protect against /0
    if( sigma < 0.000000001 )
    {
        sigma = 0.000000001;
    }

    const double* mask = gaussian_kernel(kernel_length, sigma);

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length-1)/2;
    double sum;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            sum = 0.0;

            // if the current pixel is not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        sum += mask[ (kernel_radius+dx)+(kernel_radius+dy)*kernel_length ]*double( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
            }
            // if is in the border
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        if( (x+dx >= 0 && x+dx < img_width) && (y+dy >= 0 && y+dy < img_height) )
                        {
                            sum += mask[ (kernel_radius+dx)+(kernel_radius+dy)*kernel_length ]*double( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                        }
                        if( (x+dx >= 0 && x+dx < img_width) && ( !(y+dy >= 0 && y+dy < img_height) ) )
                        {
                            sum += mask[ (kernel_radius+dx)+(kernel_radius+dy)*kernel_length ]*double( filtered[ byte_per_pixel*((x+dx)+(y-dy)*img_width)+color_channel ] );
                        }
                        if( ( !(x+dx >= 0 && x+dx < img_width) ) && (y+dy >= 0 && y+dy < img_height) )
                        {
                            sum += mask[ (kernel_radius+dx)+(kernel_radius+dy)*kernel_length ]*double( filtered[ byte_per_pixel*((x-dx)+(y+dy)*img_width)+color_channel ] );
                        }
                        if( ( !(x+dx >= 0 && x+dx < img_width) ) && ( !(y+dy >= 0 && y+dy < img_height) ) )
                        {
                            sum += mask[ (kernel_radius+dx)+(kernel_radius+dy)*kernel_length ]*double( filtered[ byte_per_pixel*((x-dx)+(y-dy)*img_width)+color_channel ] );
                        }
                    }
                }
            }
            filtered_modif[byte_per_pixel*offset+color_channel] = (unsigned char)(sum);
        }
    }

    delete[] mask;

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

void Filters::quick_sort(unsigned char* const array, int begin, int end)
{
    int left = begin-1;
    int right = end+1;
    const unsigned char pivot = array[begin];

    // if the length of the array is 0, to do nothing
    if( begin >= end )
    {
        return;
    }

    while( 1 )
    {
        do
        {
            right--;
        }
        while( array[right] > pivot );

        do
        {
            left++;
        }
        while( array[left] < pivot );

        if( left < right )
        {
            swap(array, left, right);
        }
        else
        {
            break;
        }
    }

    quick_sort(array, begin, right);
    quick_sort(array, right+1, end);
}

void Filters::median_filtering_oNlogN(int kernel_length) {

    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0)
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length-1)/2;
    int m;

    const int length = kernel_length*kernel_length;
    unsigned char* const median_kernel = new unsigned char[length];

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            // m : index of the kernel
            m = 0;

            // if the current pixel is not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        median_kernel[m++] = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                    }
                }
            }
            // if in the border
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        if( (x+dx >= 0 && x+dx < img_width) && (y+dy >= 0 && y+dy < img_height) )
                        {
                            median_kernel[m++] = filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ];
                        }
                        if( (x+dx >= 0 && x+dx < img_width) && ( !(y+dy >= 0 && y+dy < img_height) ) )
                        {
                            median_kernel[m++] = filtered[ byte_per_pixel*((x+dx)+(y-dy)*img_width)+color_channel ];
                        }
                        if( ( !(x+dx >= 0 && x+dx < img_width) ) && (y+dy >= 0 && y+dy < img_height) )
                        {
                            median_kernel[m++] = filtered[ byte_per_pixel*((x-dx)+(y+dy)*img_width)+color_channel ];
                        }
                        if( ( !(x+dx >= 0 && x+dx < img_width) ) && ( !(y+dy >= 0 && y+dy < img_height) ) )
                        {
                            median_kernel[m++] = filtered[ byte_per_pixel*((x-dx)+(y-dy)*img_width)+color_channel ];
                        }
                    }
                }
            }

            quick_sort(median_kernel,0,length-1);

            filtered_modif[byte_per_pixel*offset+color_channel] = median_kernel[(length-1)/2];
        }
    }
    delete[] median_kernel;

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

// Simon Perreault, Patrick Hébert: Median Filtering in Constant Time. IEEE Transactions on Image Processing 16(9): 2389-2394 (2007)
void Filters::median_filtering_o1(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    const int median_rank = 1+kernel_length*kernel_length/2;

    // variable called radius r in the Perreault and Hébert's article
    // kernel_radius = r
    // kernel_length = 2*r+1
    const int kernel_radius = (kernel_length-1)/2;

    int I; // pixel intensity, grey-level or channel value for rgb image

    int x, y; // position of the current pixel

    int rank, m;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        /////////////////////////////////////////
        // processing of the top of the image //
        ////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = 0; y < kernel_radius+1; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while( rank < 1+kernel_length*(x+1)/2 )
                    {
                        rank += kernel_histo[++I];
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius+1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while( rank < median_rank )
                {
                    rank += kernel_histo[++I];
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while( rank < 1+kernel_length*(kernel_length-m)/2 )
                {
                    rank += kernel_histo[++I];
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = (unsigned char)(I);
                m++;
            }
        }

        //////////////////////////////////////////////////////////////
        // processing of the image without top and bottom stripes  ///
        //////////////////////////////////////////////////////////////

        // clear
        for( I = 0; I < 256*img_width; I++ )
        {
            columns_histo[I] = 0;
        }

        // initialization
        for( x = 0; x < img_width; x++ )
        {
            for( y = 0; y < kernel_length; y++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y,color_channel)] ]++;
            }
        }

        // downward moving in the image
        for( y = kernel_radius+1; y <= img_height-kernel_radius-2; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y+kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while( rank < 1+kernel_length*(x+1)/2 )
                    {
                        rank += kernel_histo[++I];
                    }

                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y+kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while( rank < median_rank )
                {
                    rank += kernel_histo[++I];
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while( rank < 1+kernel_length*(kernel_length-m)/2 )
                {
                    rank += kernel_histo[++I];
                }
                filtered_modif[ find_offset(x,y,color_channel) ] = (unsigned char)(I);
                m++;
            }
        }

        ///////////////////////////////////////////
        // processing of the bottom of the image //
        ///////////////////////////////////////////

        // no need to clear and to initialize columns_histo

        for( y = img_height-kernel_radius-1; y < img_height; y++ )
        {
            // clear
            for( I = 0; I < 256; I++ )
            {
                kernel_histo[I] = 0;
            }

            // initialization for each new current row
            for( x = 0; x < kernel_length; x++ )
            {
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*x+filtered[find_offset(x,y-kernel_radius,color_channel)] ]++;
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*x+I ];
                }

                ///////////////////////////
                // left border processed //
                ///////////////////////////

                if( x >= kernel_radius )
                {
                    // to find the value I of the median in the kernel histogram
                    rank = 0;
                    I = -1;
                    while( rank < 1+kernel_length*(x+1)/2 )
                    {
                        rank += kernel_histo[++I];
                    }
                    filtered_modif[ find_offset(x-kernel_radius,y,color_channel) ] = (unsigned char)(I);
                }
            }

            //////////////////////
            // center processed //
            //////////////////////

            // rightward moving in the image
            for( x = kernel_radius+1; x < img_width-kernel_radius-1; x++ )
            {
                // to update the column histogram H(x+kernel_radius)
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius-1,color_channel)] ]--;
                columns_histo[ 256*(x+kernel_radius)+filtered[find_offset(x+kernel_radius,y-kernel_radius,color_channel)] ]++;

                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] += columns_histo[ 256*(x+kernel_radius)+I ]-columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while( rank < median_rank )
                {
                    rank += kernel_histo[++I];
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = (unsigned char)(I);
            }

            ////////////////////////////
            // right border processed //
            ////////////////////////////

            // rightward moving in the image
            m = 1;
            for(  ; x < img_width; x++ )
            {
                // to update the mask histogram from the column histograms H(x+kernel_radius) and H(x-kernel_radius-1)
                for( I = 0; I < 256; I++ )
                {
                    kernel_histo[I] -= columns_histo[ 256*(x-kernel_radius-1)+I ];
                }

                // to find the value I of the median in the kernel histogram
                rank = 0;
                I = -1;
                while( rank < 1+kernel_length*(kernel_length-m)/2 )
                {
                    rank += kernel_histo[++I];
                }

                filtered_modif[ find_offset(x,y,color_channel) ] = (unsigned char)(I);
                m++;
            }
        }
    }

    if( kernel_length != 1 )
    {
        // swap pointers
        unsigned char* ptemp = filtered;
        filtered = filtered_modif;
        filtered_modif = ptemp;
    }
}

void Filters::gaussian_white_noise(double sigma)
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 engine( static_cast<std::mt19937::result_type>(seed) );

    // normal/gaussian distribution with mean = 0 and standard deviation = sigma
    std::normal_distribution<double> distribution(0.0,sigma);

    auto generator = std::bind(distribution,engine);


    int filtered_int;

    for( int offset = 0; offset < byte_per_pixel*img_size; offset++ )
    {
        filtered_int = int(filtered[offset]) + (int)( generator() );

        if( filtered_int < 0 )
        {
            filtered_int = 0;
        }
        if( filtered_int > 255 )
        {
            filtered_int = 255;
        }
        filtered[offset] = (unsigned char)(filtered_int);
    }
}

void Filters::salt_pepper_noise(double proba)
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 engine( static_cast<std::mt19937::result_type>(seed) );

    std::uniform_real_distribution<double> distribution(0.0,1.0);

    auto generator = std::bind(distribution,engine);

    for( int offset = 0; offset < img_size; offset++ )
    {
        if( generator() < proba )
        {
            for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
            {
                if( generator() < 0.5 )
                {
                    filtered[byte_per_pixel*offset+color_channel] = 0;
                }
                else
                {
                    filtered[byte_per_pixel*offset+color_channel] = 255;
                }
            }
        }
    }
}

void Filters::speckle(double sigma)
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 engine( static_cast<std::mt19937::result_type>(seed) );

    std::uniform_real_distribution<double> distribution(0.0,1.0);

    auto generator = std::bind(distribution,engine);


    int filtered_int;

    for( int offset = 0; offset < byte_per_pixel*img_size; offset++ )
    {
        // img = img + X * img
        // with X a random variable from a uniform distribution centered at 0
        // and of a standard deviation sigma
        // sigma * sqrt(12) is the interval of the uniform distribution
        filtered_int = int(filtered[offset]) + int (double(filtered[offset]) * sigma * sqrt(12.0) * (generator()-0.5));

        if( filtered_int < 0 )
        {
            filtered_int = 0;
        }
        if(filtered_int > 255 )
        {
            filtered_int = 255;
        }
        filtered[offset] = (unsigned char)(filtered_int);
    }
}

void Filters::local_binary_pattern(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    const int kernel_radius = (kernel_length-1)/2;

    int x, y; // position of the current pixel

    unsigned char Icentr;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            Icentr = filtered[byte_per_pixel*offset+color_channel];

            filtered_modif[byte_per_pixel*offset+color_channel] = 0;

            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                if( filtered[ byte_per_pixel*((x-kernel_radius)+(y-kernel_radius)*img_width)+color_channel ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 1;
                }
                if( filtered[ find_offset(x,y-kernel_radius,color_channel) ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 2;
                }
                if( filtered[ find_offset(x+kernel_radius,y-kernel_radius,color_channel)] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 4;
                }
                if( filtered[ find_offset(x-kernel_radius,y,color_channel) ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 8;
                }
                if( filtered[ byte_per_pixel*((x+kernel_radius)+y*img_width)+color_channel ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 16;
                }
                if( filtered[ byte_per_pixel*((x-kernel_radius)+(y+kernel_radius)*img_width)+color_channel ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 32;
                }
                if( filtered[ find_offset(x,y+kernel_radius,color_channel) ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 64;
                }
                if( filtered[ find_offset(x+kernel_radius,y+kernel_radius,color_channel) ] >= Icentr )
                {
                    filtered_modif[ byte_per_pixel*offset+color_channel ] += 128;
                }
            }
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}


void Filters::nagao_filtering(int kernel_length)
{
    // to protect the input : kernel_length impair and strictly positive
    if( kernel_length % 2 == 0 )
    {
        kernel_length--;
    }
    if( kernel_length < 1 )
    {
        kernel_length = 1;
    }

    int x, y; // position of the current pixel

    const int kernel_radius = (kernel_length-1)/2;

    int var[9];
    int sum[9];

    int min_var;

    int nagao_size_init = square(kernel_radius+1);
    if( nagao_size_init <= 0 )
    {
        nagao_size_init = 1;
    }
    const int nagao_size = nagao_size_init;

    for( int color_channel = 0; color_channel < byte_per_pixel; color_channel++ )
    {
        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            for( int n = 0; n < 9; n++ )
            {
                sum[n] = 0;
                var[n] = 0;
            }

            // if not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius)
            {
                for( int dy = 0; dy >= -kernel_radius; dy-- )
                {
                    for( int dx = dy; dx <= -dy; dx++ )
                    {
                        sum[0] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
                for( int dx = 0; dx <= kernel_radius; dx++ )
                {
                    for( int dy = -dx; dy <= dx; dy++ )
                    {
                        sum[1] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
                for( int dy = 0; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -dy; dx <= dy; dx++ )
                    {
                        sum[2] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
                for( int dx = 0; dx >= -kernel_radius; dx-- )
                {
                    for( int dy = dx; dy <= -dx; dy++ )
                    {
                        sum[3] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }

                for( int dy = -kernel_radius; dy <= 0; dy++ )
                {
                    for( int dx = 0; dx <= kernel_radius; dx++ )
                    {
                        sum[4] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
                for( int dy = 0; dy <= kernel_radius; dy++ )
                {
                    for( int dx = 0; dx <= kernel_radius; dx++ )
                    {
                        sum[5] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
                for( int dy = 0; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= 0; dx++ )
                    {
                        sum[6] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
                for( int dy = -kernel_radius; dy <= 0; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= 0; dx++ )
                    {
                        sum[7] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }

                for( int dy = -kernel_radius+1; dy <= kernel_radius-1; dy++ )
                {
                    for( int dx = -kernel_radius+1; dx <= kernel_radius-1; dx++ )
                    {
                        sum[8] += int( filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] );
                    }
                }
            }

            // if not in the border
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for(int dy = 0; dy >= -kernel_radius; dy-- )
                {
                    for( int dx = dy; dx <= -dy; dx++ )
                    {
                        var[0] += square( sum[0]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]) );
                    }
                }
                for( int dx = 0; dx <= kernel_radius; dx++ )
                {
                    for( int dy = -dx; dy <= dx; dy++ )
                    {
                        var[1] += square( sum[1]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]) );
                    }
                }
                for( int dy = 0; dy <= kernel_radius; dy++ )
                {
                    for(int dx = -dy; dx <= dy; dx++ )
                    {
                        var[2] += square( sum[2]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] ) );
                    }
                }
                for( int dx = 0; dx >= -kernel_radius; dx-- )
                {
                    for( int dy = dx; dy <= -dx; dy++ )
                    {
                        var[3] += square( sum[3]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ] ) );
                    }
                }

                for( int dy = -kernel_radius; dy <= 0; dy++ )
                {
                    for( int dx = 0; dx <= kernel_radius; dx++ )
                    {
                        var[4] += square( sum[4]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]) );
                    }
                }
                for( int dy = 0; dy <= kernel_radius; dy++ )
                {
                    for( int dx = 0; dx <= kernel_radius; dx++ )
                    {
                        var[5] += square( sum[5]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]) );
                    }
                }
                for( int dy = 0; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= 0; dx++ )
                    {
                        var[6] += square( sum[6]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]) );
                    }
                }
                for( int dy = -kernel_radius; dy <= 0; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= 0; dx++ )
                    {
                        var[7] += square( sum[7]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]) );
                    }
                }

                for( int dy = -kernel_radius+1; dy <= kernel_radius-1; dy++ )
                {
                    for( int dx = -kernel_radius+1; dx <= kernel_radius-1; dx++ )
                    {
                        var[8] += square(sum[8]-int(filtered[ byte_per_pixel*((x+dx)+(y+dy)*img_width)+color_channel ]));
                    }
                }
            }

            min_var = 999999999;
            for( int n = 0; n < 9; n++ )
            {
                if( var[n] < min_var )
                {
                    min_var = var[n];
                }
            }

            for( int n = 0; n < 9; n++ )
            {
                if( var[n] == min_var )
                {
                    filtered_modif[byte_per_pixel*offset+color_channel] = (unsigned char)(sum[n]/nagao_size);
                }
            }
        }
    }

    // swap pointers
    unsigned char* ptemp = filtered;
    filtered = filtered_modif;
    filtered_modif = ptemp;
}

//! Gradient morphologique avec un élement structurant carré de taille kernel_length
void Filters::morphological_gradient_yuv(int kernel_length, int alpha, int beta, int gamma)
{
    if( byte_per_pixel == 3 )
    {

        // pour blinder l'entrée : kernel_length impair et strictement positif
        if( kernel_length % 2 == 0 )
        {
            kernel_length--;
        }
        if( kernel_length < 1 )
        {
            kernel_length = 1;
        }

        const int kernel_radius = (kernel_length-1)/2;

        int R, G, B;
        int Y, U, V;

        int x, y; // position of the current pixel

        int maxY, minY, maxU, minU, maxV, minV;

        for( int offset = 0; offset < img_size; offset++ )
        {
            y = offset/img_width;
            x = offset-y*img_width;

            // initialisation
            maxY = 0;
            minY = 255;
            maxU = 0;
            minU = 255;
            maxV = 0;
            minV = 255;

            // si pas sur le bord
            if( x > kernel_radius-1 && x < img_width-kernel_radius && y > kernel_radius-1 && y < img_height-kernel_radius )
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        R = int(filtered[3*((x+dx)+(y+dy)*img_width)]);
                        G = int(filtered[3*((x+dx)+(y+dy)*img_width)+1]);
                        B = int(filtered[3*((x+dx)+(y+dy)*img_width)+2]);

                        Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
                        U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
                        V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

                        // pas de tests des voisins
                        if( Y > maxY )
                        {
                            maxY = Y;
                        }
                        if( Y < minY )
                        {
                            minY = Y;
                        }

                        if( U > maxU )
                        {
                            maxU = U;
                        }
                        if( U < minU )
                        {
                            minU = U;
                        }

                        if( V > maxV )
                        {
                            maxV = V;
                        }
                        if( V < minV )
                        {
                            minV = V;
                        }
                    }
                }
            }
            // si sur le bord
            else
            {
                for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
                {
                    for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
                    {
                        // tests des voisins
                        if( x+dx >= 0 && x+dx < img_width && y+dy >= 0 && y+dy < img_height )
                        {
                            R = int(filtered[3*((x+dx)+(y+dy)*img_width)]);
                            G = int(filtered[3*((x+dx)+(y+dy)*img_width)+1]);
                            B = int(filtered[3*((x+dx)+(y+dy)*img_width)+2]);

                            Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
                            U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
                            V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

                            // pas de tests des voisins
                            if( Y > maxY )
                            {
                                maxY = Y;
                            }
                            if( Y < minY )
                            {
                                minY = Y;
                            }

                            if( U > maxU )
                            {
                                maxU = U;
                            }
                            if( U < minU )
                            {
                                minU = U;
                            }

                            if( V > maxV )
                            {
                                maxV = V;
                            }
                            if( V < minV )
                            {
                                minV = V;
                            }
                        }
                    }
                }
            }
            gradient[offset] = (unsigned char)((alpha*(maxY-minY)+beta*(maxU-minU)+gamma*(maxV-minV))/(alpha+beta+gamma));
        }

    }
}

void Filters::fas(int kernel_length)
{
    for( int k = 3; k < kernel_length; k += 2 )
    {
        opening(k);
        closing(k);
    }
}

}
