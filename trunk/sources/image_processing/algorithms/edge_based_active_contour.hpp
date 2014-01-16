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

#ifndef EDGE_BASED_ACTIVE_CONTOUR_HPP
#define EDGE_BASED_ACTIVE_CONTOUR_HPP

#include "active_contour.hpp"

namespace ofeli
{

class EdgeBasedActiveContour : public ActiveContour
{

public :

    //! Constructor to initialize the active contour with a centered rectangle and the default values of the algorithm parameters.
    EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1);

    //! Constructor to initialize the active contour from geometrical parameters of an unique shape, an ellipse or a rectangle.
    EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1,
               bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
               bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1);

    //! Constructor to initialize the active contour from an initial phi level-set function.
    EdgeBasedActiveContour(const ofeli::Matrix<const unsigned char>& img_gradient1,
               const ofeli::Matrix<const signed char>& phi_init1,
               bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1);

    //! Copy constructor.
    EdgeBasedActiveContour(const EdgeBasedActiveContour& ac);

    //! Initialization for each new frame buffer, used for video tracking.
    virtual void initialize_for_each_frame() override;

    //! Getter function for #otsu_threshold1
    unsigned char get_otsu_threshold() const { return otsu_threshold1; }

private :

    //! Computes external speed \a Fd with a geodesic model for a current point \a (x,y) of #Lout or #Lin.
    virtual int compute_external_speed_Fd(unsigned int offset) override;

    //! Calculates boolean #isInside.
    bool find_direction_evolution();

    //! Otsu's method.
    static unsigned char otsu_method(const ofeli::Matrix<const unsigned char>& img);

    const ofeli::Matrix<const unsigned char> img_gradient;

    //! Otsu's threshod of #img_data.
    const unsigned char otsu_threshold1;
    //! Determines where are the high values of the gradient.
    bool isInside;
};

}

#endif // EDGE_BASED_ACTIVE_CONTOUR_HPP

//! \class ofeli::EdgeBasedActiveContour
//! The child class EdgeBasedActiveContour implements a function to calculate specifically speed \a Fd based on a geodesic model, an edge-based energy functional. An Otsu's method is performed in order to find the high values of the gradient to converge even when the initialization of the curve is far of the solution.

/**
 * \fn EdgeBasedActiveContour::EdgeBasedActiveContour(const unsigned char* img_gradient_data1, int img_width1, int img_height1,
                              bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                              bool hasCycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1)
 * \param img_gradient_data1 Input pointer on the grayscale image data buffer. Usually, this buffer is a smoothed gradient of an original image. This buffer must be row-wise. Passed to #img_data.
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
 */

/**
 * \fn EdgeBasedActiveContour::EdgeBasedActiveContour(const unsigned char* img_gradient_data1, int img_width1, int img_height1,
                              const char* phi_init1,
                              bool hasCycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1)
 * \param img_gradient_data1 Input pointer on the grayscale image data buffer. Usually, this buffer is a smoothed gradient of an original image. This buffer must be row-wise. Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi.
 * \param hasCycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed \a Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of times the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of times the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 */

/**
 * \fn virtual int EdgeBasedActiveContour::compute_external_speed_Fd(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */
