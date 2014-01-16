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

#ifndef REGION_BASED_ACTIVE_CONTOUR_YUV_HPP
#define REGION_BASED_ACTIVE_CONTOUR_YUV_HPP

#include "active_contour.hpp"

namespace ofeli
{

class RegionBasedActiveContourYUV : public ActiveContour
{

public :

    //! Constructor to initialize the active contour with a centered rectangle and the default values of the algorithm parameters.
    RegionBasedActiveContourYUV(const ofeli::Matrix<const unsigned char>& img_rgb1);

    //! Constructor to initialize the active contour from geometrical parameters of an unique shape, an ellipse or a rectangle.
    RegionBasedActiveContourYUV(const ofeli::Matrix<const unsigned char>& img_rgb1,
                      bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                      bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                      int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1);

    //! Constructor to initialize the active contour from an initial phi level-set function.
    RegionBasedActiveContourYUV(const ofeli::Matrix<const unsigned char>& img_rgb1,
                      const ofeli::Matrix<const signed char>& phi_init1,
                      bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                      int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1);

    //! Copy constructor.
    RegionBasedActiveContourYUV(const RegionBasedActiveContourYUV& ac);

    //! Initialization for each new frame buffer, used for video tracking.
    virtual void initialize_for_each_frame() override;

    //! Getter function for #Cout_RGB
    const int* get_Cout_RGB() const { return Cout_RGB; }

    //! Getter function for #Cin_RGB
    const int* get_Cin_RGB() const { return Cin_RGB; }

    //! Getter function for #Cout_YUV
    const int* get_Cout_YUV() const { return Cout_YUV; }

    //! Getter function for #Cin_YUV
    const int* get_Cin_YUV() const { return Cin_YUV; }

private :

    //! Affects the variable R, G, B from the buffer img_rgb_data
    void RGB_from_buffer(unsigned int offset);

    //! Initializes the six sums and #n_in and #n_out with scanning through the image.
    void initialize_sums();

    //! Calculates means #CoutYUV and #CinYUV in \a O(1) or accounting for the previous updates of (#sum_out_R, #sum_out_G, #sum_out_B) and (#sum_in_R, #sum_in_G, #sum_in_B), in \a O(#lists_length) and not in \a O(#img_size).
    virtual void calculate_means() override;

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of #Lout or #Lin.
    virtual int compute_external_speed_Fd(unsigned int offset) override;


    //! Updates the six sums, #n_in and #n_out, before each #switch_in, in the cycle 1, in order to calculate means #CoutYUV and #CinYUV.
    virtual void updates_for_means_in1() override;

    //! Updates the six sums, #n_in and #n_out, before each #switch_out, in the cycle 1, in order to calculate means #CoutYUV and #CinYUV.
    virtual void updates_for_means_out1() override;

    //! Updates the six sums, #n_in and #n_out, before each #switch_in, in the cycle 2, in order to calculate means #CoutYUV and #CinYUV.
    virtual void updates_for_means_in2(unsigned int offset) override;

    //! Updates the six sums, #n_in and #n_out, before each #switch_out, in the cycle 2, in order to calculate means #CoutYUV and #CinYUV.
    virtual void updates_for_means_out2(unsigned int offset) override;

    //! Calculates \a YUV value with a RGB value.
    static void calculate_YUV(const int RGB[], int YUV[]);

    const ofeli::Matrix<const unsigned char> img_rgb;

    //! Weight of the outside homogeneity criterion in the Chan-Vese model.
    const int lambda_out;
    //! Weight of the inside homogeneity criterion in the Chan-Vese model.
    const int lambda_in;

    //! Weight of component \a Y to calculate external speed \a Fd.
    const int alpha;
    //! Weight of component \a U to calculate external speed \a Fd.
    const int beta;
    //! Weight of component \a V to calculate external speed \a Fd.
    const int gamma;


    //! YUV mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int Cout_YUV[3];

    //! YUV mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    int Cin_YUV[3];

    //! RGB mean of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int Cout_RGB[3];

    //! RGB mean of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    int Cin_RGB[3];

    //! Sum of component #R, #G, #B of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int sum_out_RGB[3];
    //! Sum of component #R, #G, #B of the pixels intside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    int sum_in_RGB[3];

    //! Number of pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$ .
    int n_out;
    //! Number of pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$ .
    int n_in;

    //! Red, green and blue component of the current pixel.
    int RGB[3];
};

// Definitions

inline void RegionBasedActiveContourYUV::RGB_from_buffer(unsigned int offset)
{
    #ifdef RGB_PLANAR_IMAGE_DATA_BUFFER
    const unsigned int img_rgb_size = img_rgb.get_width()*img_rgb.get_height();

    // RGB planar image data buffer, with R1 R2 R3 ... G1 G2 G3 ... B1 B2 B3 ... (for Matlab)
    RGB[0] = int(img_rgb[ offset ]);
    RGB[1] = int(img_rgb[ offset+img_rgb_size ]);
    RGB[2] = int(img_rgb[ offset+2*img_rgb_size ]);

    #else

    // RGB interleaved image data buffer, with R1 G1 B1 R2 G2 B2 ...
    RGB[0] = int(img_rgb[ 3*offset ]);
    RGB[1] = int(img_rgb[ 3*offset+1 ]);
    RGB[2] = int(img_rgb[ 3*offset+2 ]);

    #endif
}

inline void RegionBasedActiveContourYUV::calculate_YUV(const int RGB[], int YUV[])
{
    YUV[0] = ( (  66 * RGB[0] + 129 * RGB[1] +  25 * RGB[2] + 128) >> 8) +  16; // Y
    YUV[1] = ( ( -38 * RGB[0] -  74 * RGB[1] + 112 * RGB[2] + 128) >> 8) + 128; // U
    YUV[2] = ( ( 112 * RGB[0] -  94 * RGB[1] -  18 * RGB[2] + 128) >> 8) + 128; // V
}


}

#endif // REGION_BASED_ACTIVE_CONTOUR_YUV_HPP


//! \class ofeli::RegionBasedActiveContourYUV
//! The child class RegionBasedActiveContourYUV implements a function to calculate specifically speed \a Fd based on the Chan-Vese model, a region-based energy functional.
//! The regularization of our active contour is performed by a gaussian smoothing of #phi so we are interested uniquely by the external or data dependant term of this energy functional.\n
//! \f$F_{d}=\lambda _{out}\left[ \alpha \left( Y_{out}-C_{outC}\right) ^{2}+ \beta \left( U_{out}-C_{outU}\right) ^{2}+ \gamma \left( V_{out}-C_{outV}\right) ^{2}\right] + \lambda _{in}\left[ \alpha \left( Y_{in}-C_{inY}\right) ^{2}+ \beta \left( U_{in}-C_{inU}\right) ^{2}+ \gamma \left( V_{in}-C_{inV}\right) ^{2}\right]\f$
//!  - \f$F_{d}\f$ : data dependant evolution speed calculated for each point of the active contour, only it sign is used by the algorithm. \n
//!  - \f$Y\f$ : luminance component Y of the (Y,U,V) color space of the current pixel of the active contour. \n
//!  - \f$U\f$ : chrominance component U of the (Y,U,V) color space of the current pixel of the active contour. \n
//!  - \f$V\f$ : chrominance component V of the (Y,U,V) color space of the current pixel of the active contour. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$C_{out}\f$ : mean of the intensities or grey-levels of the pixels outside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) >0\f$. \n
//!  - \f$C_{in}\f$ : mean of the intensities or grey-levels of the pixels inside the curve, i.e. pixels \f$i\f$ with \f$\phi \left( i\right) <0\f$. \n
//!  - \f$\lambda _{out}\f$ : weight of the outside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\lambda _{in}\f$ : weight of the inside homogeneity criterion in the Chan-Vese model. \n
//!  - \f$\alpha\f$ : weight of the luminance component Y. \n
//!  - \f$\beta\f$ : weight of the chrominance component U. \n
//!  - \f$\gamma\f$ : weight of the chrominance component V.

/**
 * \fn RegionBasedActiveContourYUV::RegionBasedActiveContourYUV(const unsigned char* img_rgb_data1, int img_width1, int img_height1,
                                            bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                                            bool hasCycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                                            int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1)
 * \param img_rgb_data1 Input pointer on the RGB image data buffer. This buffer must be row-wise and interleaved (R1 G1 B1 R2 G2 B2 ...). Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param hasEllipse1 Boolean to choose the shape of the active contour initialization, \c true for an ellipse or \c false for a rectangle.
 * \param shape_width_ratio1 Width of the shape divided by the image #img_width.
 * \param shape_height_ratio1 Height of the shape divided by the image #img_height.
 * \param center_x_ratio1 X-axis position (or column index) of the center of the shape divided by the image #img_width subtracted by 0.5
 * \param center_y_ratio1 Y-axis position (or row index) of the center of the shape divided by the image #img_height subtracted by 0.5\n
          To have the center of the shape in the image : -0.5 < center_x_ratio1 < 0.5 and -0.5 < center_y_ratio1 < 0.5
 * \param hasCycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed \a Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambda_out.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambda_in.
 * \param alpha1 Weight of luminance Y. Passed to #alpha.
 * \param beta1 Weight of chrominance U. Passed to #beta.
 * \param gamma1 Weight of chrominance V. Passed to #gamma.
 */

/**
 * \fn RegionBasedActiveContourYUV::RegionBasedActiveContourYUV(const unsigned char* img_rgb_data1, int img_width1, int img_height1,
                                            const char* phi_init1,
                                            bool hasCycle2_1,
                                            int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1,
                                            int lambda_out1, int lambda_in1, int alpha1, int beta1, int gamma1)
 * \param img_rgb_data1 Input pointer on the RGB data image buffer. This buffer must be row-wise and interleaved (R1 G1 B1 R2 G2 B2 ...). Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi.
 * \param hasCycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed \a Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 * \param lambda_out1 Weight of the outside homogeneity criterion. Passed to #lambda_out.
 * \param lambda_in1 Weight of the inside homogeneity criterion. Passed to #lambda_in.
 * \param alpha1 Weight of luminance Y. Passed to #alpha.
 * \param beta1 Weight of chrominance U. Passed to #beta.
 * \param gamma1 Weight of chrominance V. Passed to #gamma.
 */

/**
 * \fn virtual void RegionBasedActiveContourYUV::compute_external_speed_Fd(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual void RegionBasedActiveContourYUV::updates_for_means_in2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual RegionBasedActiveContourYUV::updates_for_means_out2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */
