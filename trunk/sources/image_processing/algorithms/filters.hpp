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

#ifndef FILTERS_HPP
#define FILTERS_HPP

namespace ofeli
{

class Filters
{

public :

    //! Constructor with an input pointer on a row-wise image data buffer and the dimensions of the image.
    Filters(const unsigned char* img_data1, int img_width1, int img_height1, int byte_per_pixel1);

    //! Desctructor.
    ~Filters();

    //! Copies \a img buffer to \a filtered buffer.
    void initialyze_filtered();

    //! Performs a Perona-Malik anisotropic diffusion with a 8-connected neighborhood.
    void anisotropic_diffusion(int max_itera, double lambda, double kappa, int option);

    //! Performs a morphological_gradient with a square structuring element with a naïve algorithm.
    void morphological_gradient(int kernel_length);
    //! Performs a morphological_gradient with a square structuring element with the Perreault's algorithm.
    void morphological_gradient_o1(int kernel_length);

    //! Performs a dilation with a square structuring element with a naïve algorithm.
    void dilation(int kernel_length);
    //! Performs a dilation with a square structuring element with the Perreault's algorithm.
    void dilation_o1(int kernel_length);

    //! Performs an erosion with a square structuring element with a naïve algorithm.
    void erosion(int kernel_length);
    //! Performs an erosion with a square structuring element with the Perreault's algorithm.
    void erosion_o1(int kernel_length);

    //! Performs a closing with a square structuring element with a naïve algorithm.
    void closing(int kernel_length);
    //! Performs a closing with a square structuring element with the Perreault's algorithm.
    void closing_o1(int kernel_length);

    //! Performs a closing with a square structuring element with a naïve algorithm.
    void opening(int kernel_length);
    //! Performs an opening with a square structuring element with the Perreault's algorithm.
    void opening_o1(int kernel_length);

    //! Performs a black top hat transform with a square structuring element with a naïve algorithm.
    void black_top_hat(int kernel_length);
    //! Performs a black top hat transform with a square structuring element with the Perreault's algorithm.
    void black_top_hat_o1(int kernel_length);

    //! Performs a white top hat transform with a square structuring element with a naïve algorithm.
    void white_top_hat(int kernel_length);
    //! Performs a white top hat transform with a square structuring element with the Perreault's algorithm.
    void white_top_hat_o1(int kernel_length);

    //! Performs a filter alternate sequential.
    void fas(int kernel_length);

    //! Performs a mean filtering.
    void mean_filtering(int kernel_length);

    //! Performs a Nagao filtering.
    void nagao_filtering(int kernel_length);

    //! Performs a gaussian filtering with the kernel length and the standard deviation \c sigma.
    void gaussian_filtering(int kernel_length, double sigma);

    //! Performs a median filtering based on the quick sort algorithm. Complexity is in \c O(kernel_length×log(kernel_length)).
    void median_filtering_oNlogN(int kernel_length);

    //! Performs a median filtering based on the Perreault's algorithm. Complexity is in \c O(1).
    void median_filtering_o1(int kernel_length);

    //! Adds a gaussian white noise with the standard deviation \c sigma.
    void gaussian_white_noise(double sigma);

    //! Does an impulsional noise with a probabity of noise for each pixel, i.e the density.
    void salt_pepper_noise(double proba);

    //! Performs a speckle noise, a multiplicative noise into the image.
    void speckle(double sigma);

    //! Computes the local binary pattern.
    void local_binary_pattern(int kernel_length);

    //! Computes a morphological gradient #gradient.
    void morphological_gradient_yuv(int kernel_length, int alpha, int beta, int gamma);

    //! Getter function for #filtered.
    const unsigned char* get_filtered() const { return filtered; }

    //! Getter function for #gradient.
    const unsigned char* get_gradient() const { return gradient; }

private :

    //! Input pointer on the grayscale or grey-level image data buffer. This buffer must be row-wise.
    const unsigned char* const img_data;
    //! Image width, i.e. number of columns.
    const int img_width;
    //! Image height, i.e. number of rows.
    const int img_height;
    //! Image size, i.e. number of pixels. Obviously, it egals to #img_width × #img_height.
    const int img_size;
    //! Number of byte or octet per pixel, i.e number of channels. It is equal 1 for a grayscale or grey-level image and 3 for a color image.
    const int byte_per_pixel;

    //! Pointer on the current result image.
    unsigned char* filtered;
    //! Pointer on the temporary result because the filters are not in place.
    unsigned char* filtered_modif;
    //! Pointer used by top-hat functions to keep in memory the previous result before opening or closing to subtract the both images then.
    unsigned char* previous_filtered;
    //! Pointer on the gradient used by the geodesic model in the case of an RGB image.
    unsigned char* gradient;

    // pour le filtre anisotrope
    //! Pointer on the diffused image buffer for the anisotropic diffusion filter.
    double* diff_img;
    //! Pointer on the temporary diffused image buffer for the anisotropic diffusion filter because this filter is not in place
    double* diff_img1;

    //! Columns histogram used by the Perreault's algorithm.
    int* const columns_histo;
    //! Kernel histogram used by the Perreault's algorithm.
    int kernel_histo[256];

    //! Calculates the offset the image data buffer with the position (\a x,\a y, \a color_channel).
    int find_offset(int x, int y, int color_channel) const;

    //! Creates a gaussian kernel with the kernel length and the standard deviation sigma.
    static const double* gaussian_kernel(int kernel_length, double sigma);

    //! Swaps two values of an array.
    static void swap(unsigned char* const array, int a, int b);

    //! Sorts an array with the quick sort algorithm.
    static void quick_sort(unsigned char* const array, int begin, int end);

    //! Gives the square of a value.
    template <typename T>
    static T square(const T& value);

};

inline int Filters::find_offset(int x, int y, int color_channel) const
{
    return byte_per_pixel*(x+y*img_width)+color_channel;
}

inline void Filters::swap(unsigned char* const array, int a, int b)
{
    unsigned char temp = array[a];
    array[a] = array[b];
    array[b] = temp;
}

template <typename T>
inline T Filters::square(const T& value)
{
    return value*value;
}


}

#endif // FILTERS_HPP

//! \class ofeli::Filters
//! This class implements filters and noise functions for RGB images. \n
//! There are two linear filters : a mean filter and a gaussian filter. \n
//! Moreover, there are seven operators of mathematical morphology (erosion, dilation, morphological gradient, opening, closing and the both top-hat transforms) with a square for a structuring element. \n
//! Furthermore, there are two edge preserving filters : a median filter and a Perona-Malik anisotropic diffusion. \n
//! The order filters (i.e. the median filter and the morphological filters) are implemented in a naïve version and in an optimized version (using Simon Perreault's algorithm). \n
//! In addition, there are three functions to add noise, implemented with the random variables of Boost library : gaussian white noise, impulsional noise or salt and pepper and speckle.

/**
 * \fn void Filters::anisotropic_diffusion(int max_itera, double lambda, double kappa, int option)
 *
 * \param max_itera maximum number of iterations
 * \param lambda constant of integration ( \f$0\leq \lambda \leq \frac {1} {4}\f$ ), max value for the numerical stability
 * \param kappa constant of diffusion
 * \param option choice of the function of conduction coefficient : \n
 * 1 - \f$g\left( \nabla I\right) =e^{\left( -\left( \left\| \nabla I\right\| / \kappa\right) ^{2}\right) }\f$ \n
 * 2 - \f$g\left( \nabla I\right) =\frac {1} {1+\left( \frac {\left\| \nabla I\right\| } {\kappa}\right) ^{2}}\f$
 */
