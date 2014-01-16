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

#ifndef HAUSDORFF_DISTANCE_HPP
#define HAUSDORFF_DISTANCE_HPP

namespace ofeli
{

class HausdorffDistance
{

public :

    //! Constructor considering lists A and B to compare and their sizes and image dimensions. All calculations are done in the construcor.
    HausdorffDistance(const int* list1_1, int list1_1_length, int img1_1_width, int img1_1_height,
                      const int* list2_1, int list2_1_length, int img2_1_width, int img2_1_height);

    //! Getter function for #hausdorff_dist.
    double get_hausdorff_dist() const { return hausdorff_dist; }

    //! Getter function for #hausdorff_dist.
    double get_modified_hausdorff_dist() const { return modified_hausdorff_dist; }

    //! Getter function for #hausdorff_ratio.
    double get_hausdorff_ratio() const { return hausdorff_ratio; }

    //! Getter function for #modified_hausdorff_dist.
    double get_modified_hausdorff_ratio() const { return modified_hausdorff_ratio; }

    //! Getter function for #centroids_dist.
    double get_centroids_dist() const { return centroids_dist; }

    //! Getter function for #centroids_ratio.
    double get_centroids_ratio() const { return centroids_ratio; }

private :

    //! Computes the distances #hausdorff_dist and #modified_hausdorff_dist between #list1 and #list2.
    void compute_hausdorff_dist(double& hausdorff_dist, double& modified_hausdorff_dist) const;

    //! Calculates the maximum ditance.
    static double calculate_max_dist(int width1, int height1, int width2, int height2);

    //! Swaps lists to compute the backward distances.
    void swap_lists();

    //! Gives the square of a value.
    template <typename T>
    static T square(const T& value);

    //! Pointer on a array containing the pixels' offsets of a boundary segmentation from an image data buffer \a img1.
    const int* list1;
    //! Length of the array pointed by #list1.
    int list1_length;
    //! \a img1 width, i.e. number of columns of \a img1.
    int img1_width;
    //! \a img1 height, i.e. number of rows of \a img1.
    int img1_height;


    //! Pointer on a array containing the pixels' offsets of boundary segmentation from an image data buffer \a img2.
    const int* list2;
    //! Length of the array pointed by #list2.
    int list2_length;
    //! \a img2 width, i.e. number of columns of \a img2.
    int img2_width;
    //! img2 height, i.e. number of rows of img2.
    int img2_height;


    //! Mean position of list1 in the X-axis.
    double mean_x1;
    //! Mean position of list1 in the Y-axis.
    double mean_y1;

    //! Mean position of list2 in the X-axis.
    double mean_x2;
    //! Mean position of list2 in the Y-axis.
    double mean_y2;

    //! Maximum distance or diagonal of the images.
    const double max_dist;

    //! Hausdorff distance, i.e. max( min(distance) ).
    double hausdorff_dist;
    //! Hausdorff distance divides by max_dist.
    double hausdorff_ratio;
    //! Modified Hausdorff distance, i.e mean( min(distance) ).
    double modified_hausdorff_dist;
    //! Modified Hausdorff distance divides by max_dist.
    double modified_hausdorff_ratio;

    //! Distance between the centroids of the lists.
    double centroids_dist;
    //! Distance between the centroids of the lists divides by max_dist.
    double centroids_ratio;

};

template <typename T>
inline T HausdorffDistance::square(const T& value)
{
    return value*value;
}

}

#endif // HAUSDORFF_DISTANCE_HPP

//! \class ofeli::HausdorffDistance
//! The class HausdorffDistance computes the Hausdorff distance and the modified Hausdorff distance from two arrays of points which contain the pixels' offsets of a boundary segmentation.
