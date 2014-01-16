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

#include "hausdorff_distance.hpp"
#include <cmath> // for the function "std::sqrt"
#include <algorithm> // for the functions "std::max" and "std::swap"
#include <iostream> // for the object "std::cerr"

namespace ofeli
{

HausdorffDistance::HausdorffDistance(const int* list1_1, int list1_1_length, int img1_1_width, int img1_1_height,
                                     const int* list2_1, int list2_1_length, int img2_1_width, int img2_1_height) :
    list1(list1_1), list1_length(list1_1_length), img1_width(img1_1_width), img1_height(img1_1_height),
    list2(list2_1), list2_length(list2_1_length), img2_width(img2_1_width), img2_height(img2_1_height),
    max_dist( calculate_max_dist(img1_1_width,img1_1_height,img2_1_width,img2_1_height) )
{
    if( list1_1 == nullptr )
    {
        std::cerr << std::endl << "The pointer list1_1 must be a non-null pointer, it must be allocated." << std::endl;
    }
    if( list1_1_length == 0 )
    {
        std::cerr << std::endl << "list1_1 must not be empty." << std::endl;
    }

    if( list2_1 == nullptr )
    {
        std::cerr << std::endl << "The pointer list2_1 must be a non-null pointer, it must be allocated." << std::endl;
    }
    if( list2_1_length == 0 )
    {
        std::cerr << std::endl << "list2_1 must not be empty." << std::endl;
    }

    ////////////////  it calculates the centroids  ////////////////////

    int x1, y1;
    int sum_x1 = 0;
    int sum_y1 = 0;

    for( int index = 0; index < list1_length; index++ )
    {
        y1 = list1[index]/img1_width;
        x1 = list1[index]-y1*img1_width;

        sum_x1 += x1;
        sum_y1 += y1;
    }

    // centroid of list 1
    if( list1_length != 0 )
    {
        mean_x1 = double(sum_x1)/double(list1_length);
        mean_y1 = double(sum_y1)/double(list1_length);
    }

    int x2, y2;
    int sum_x2 = 0;
    int sum_y2 = 0;

    for( int index = 0; index < list2_length; index++ )
    {
        y2 = list2[index]/img2_width;
        x2 = list2[index]-y2*img2_width;

        sum_x2 += x2;
        sum_y2 += y2;
    }

    // centroid of list 2
    if( list2_length != 0 )
    {
        mean_x2 = double(sum_x2)/double(list2_length);
        mean_y2 = double(sum_y2)/double(list2_length);
    }

    ////////////////////////////////////////////////////////////////////

    // hd, Hausdorff distance
    // mhd, modified Hausdorff distance
    double hd, mhd, backward_hd, backward_mhd;

    compute_hausdorff_dist(hd,mhd); // passed by reference (output arguments)
    swap_lists(); // list1 <--> list2
    compute_hausdorff_dist(backward_hd, backward_mhd); // passed by reference (output arguments)

    hausdorff_dist = std::max(hd,backward_hd);
    modified_hausdorff_dist = std::max(mhd,backward_mhd);
    centroids_dist = std::sqrt( square(mean_x2-mean_x1)
                                + square(mean_y2-mean_y1) );

    hausdorff_ratio = hausdorff_dist/max_dist;
    modified_hausdorff_ratio = modified_hausdorff_dist/max_dist;
    centroids_ratio = centroids_dist/max_dist;
}

void HausdorffDistance::compute_hausdorff_dist(double& hausdorff_dist,
                                               double& modified_hausdorff_dist) const
{
    int x1, y1, x2, y2;
    double relative_x1, relative_y1, relative_x2, relative_y2;
    double euclidean_dist, sum_min_dist, min_dist;

    sum_min_dist = 0.0;

    // initialization in order to maximize
    hausdorff_dist = 0.0;

    // for each point of list 1
    for( int index1 = 0; index1 < list1_length; index1++ )
    {
        y1 = list1[index1]/img1_width;
        x1 = list1[index1]-y1*img1_width;

        // position in the centroid frame
        relative_x1 = double(x1)-mean_x1;
        relative_y1 = double(y1)-mean_y1;

        // initialization in order to minimize
        min_dist = 999999.999;

        // computes the distance of all points of list 2
        for( int index2 = 0; index2 < list2_length; index2++ )
        {
            y2 = list2[index2]/img2_width;
            x2 = list2[index2]-y2*img2_width;

            // position in the centroid frame
            relative_x2 = double(x2)-mean_x2;
            relative_y2 = double(y2)-mean_y2;


            // computes the euclidean distance
            euclidean_dist = std::sqrt( square(relative_x2-relative_x1)
                                        + square(relative_y2-relative_y1) );

            // it minimizes min_dist
            if( euclidean_dist < min_dist )
            {
                min_dist = euclidean_dist;
            }
        }

        // it maximizes hausdorff_dist
        if( min_dist > hausdorff_dist )
        {
            hausdorff_dist = min_dist;
        }

        sum_min_dist += min_dist;
    }

    // modified hausdorff distance = mean of min_dist
    modified_hausdorff_dist = sum_min_dist/double(list1_length);
}

void HausdorffDistance::swap_lists()
{
    std::swap(list1,list2);
    std::swap(list1_length,list2_length);
    std::swap(img1_width,img2_width);
    std::swap(img1_height,img2_height);
    std::swap(mean_x1,mean_x2);
    std::swap(mean_y1,mean_y2);
}

double HausdorffDistance::calculate_max_dist(int width1, int height1,
                                             int width2, int height2)
{
    int max_width = std::max(width1,width2);
    int max_height = std::max(height1,height2);

    return std::sqrt( double( square(max_width) + square(max_height) ) );
}

}
