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

#ifndef ACTIVE_CONTOUR_HPP
#define ACTIVE_CONTOUR_HPP

// false or true below, respectively,
// if you want the 4-connexity or 8-connexity version of the algorithm
#ifndef HAS_8_CONNEXITY
#define HAS_8_CONNEXITY false
#endif
// functions affected by the define :
// - void switch_in(int offset)
// - void switch_out(int offset)
// - bool isRedundantLinPoint(int offset) const
// - bool isRedundantLoutPoint(int offset) const

#include "list.hpp"
#include "matrix.hpp"
#include <iostream> // for the objects "std::ostream" and "std::cerr"

namespace ofeli
{

class ActiveContour
{

public :

    //! Constructor to initialize the active contour from geometrical parameters of an unique shape, an ellipse or a rectangle.
    ActiveContour(unsigned int phi_width, unsigned int phi_height,
                  bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                  bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1);

    //! Constructor to initialize the active contour from an initial phi level-set function.
    ActiveContour(const ofeli::Matrix<const signed char>& phi_init1,
                  bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1);

    //! Copy constructor.
    ActiveContour(const ActiveContour& ac);

    //! Destructor to delete #phi and #gaussian_kernel.
    virtual ~ActiveContour();

    //! Initialization for each new frame buffer, used for video tracking.
    virtual void initialize_for_each_frame();

    //! Evolves the active contour of one iteration or one step. This function calls the functions #do_one_iteration_in_cycle1 or #do_one_iteration_in_cycle2.
    void evolve_one_iteration();

    //! Evolves directly the active contour to the final state, i.e. it evolves while #isStopped is not \c true.
    void evolve();

    //! Overloading of cout <<. It displays the active contour position.
    friend std::ostream& operator<<(std::ostream& os, const ActiveContour& ac);

    //! Displays the active contour position.
    void display() const;

    //! Getter function for the pointer #phi.
    const ofeli::Matrix<signed char>& get_phi() const { return phi; }
    //! Getter function for the linked list #Lout.
    const ofeli::List<unsigned int>& get_Lout() const { return Lout; }
    //! Getter function for the linked list #Lin.
    const ofeli::List<unsigned int>& get_Lin() const { return Lin; }

    //! Getter function for the boolean #hasListsChanges.
    bool get_hasListsChanges() const { return hasListsChanges; }
    //! Getter function for the boolean #hasOscillation.
    bool get_hasOscillation() const { return hasOscillation; }
    //! Getter function for #iteration.
    unsigned int get_iteration() const { return iteration; }
    //! Getter function for #iteration_max.
    unsigned int get_iteration_max() const { return iteration_max; }
    //! Getter function for the boolean #isStopped.
    bool get_isStopped() const { return isStopped; }

protected :

    //! Initialization of #phi with a shape.
    void initialize_phi_with_a_shape(bool hasEllipse, double shape_width_ratio, double shape_height_ratio,
                                     double center_x_ratio, double center_y_ratio);

    //! Initialization of #Lout and #Lin.
    void initialize_lists();

    //! Level-set function.
    ofeli::Matrix<signed char> phi;

    //! Gives the square of a value.
    template <typename T>
    static T square(const T& value);

    template <typename T>
    static bool isValid_matrix(const ofeli::Matrix<T>& matrix);

    //! Checks #lambda_out1, #lambda_in1, #alpha1, #beta1 or #gamma1.
    static int check_value(int value);

private :

    //! Function called by evolve_one_iteration() for external or data dependant evolution with \a Fd speed.
    void do_one_iteration_in_cycle1();

    //! Function called by evolve_one_iteration() for a curve smoothing or internal evolution with \a Fint speed.
    void do_one_iteration_in_cycle2();



    //! Outward local movement of the curve for a current point (\a x,\a y) of #Lout that is switched in #Lin.
    ofeli::List<unsigned int>::iterator switch_in(ofeli::List<unsigned int>::iterator Lout_point);

    //! Second step of procedure #switch_in.
    void add_Rout_neighbor_to_Lout(unsigned int neighbor_offset);

    //! Inward local movement of the curve for a current point (\a x,\a y) of #Lin that is switched in #Lout.
    ofeli::List<unsigned int>::iterator switch_out(ofeli::List<unsigned int>::iterator Lin_point);

    //! Second step of procedure #switch_out.
    void add_Rin_neighbor_to_Lin(unsigned int neighbor_offset);

    //! Computes the internal speed  Fint for a current point (\a x,\a y) of #Lout or #Lin.
    int compute_internal_speed_Fint(unsigned int offset);

    //! Computes the external speed \a Fd for a current point (\a x,\a y) of #Lout or #Lin.
    virtual int compute_external_speed_Fd(unsigned int offset);

    //! Finds if a current point (\a x,\a y) of #Lin is redundant.
    bool isRedundantLinPoint(unsigned int offset) const;

    //! Eliminates redundant points in #Lin. Each point of a list must be connected at least by one point of the other list.
    void clean_Lin();

    //! Finds if a current point (\a x,\a y) of #Lout is redundant.
    bool isRedundantLoutPoint(unsigned int offset) const;

    //! Eliminates redundant points in #Lout. Each point of a list must be connected at least by one point of the other list.
    void clean_Lout();



    //! Specific step reimplemented in the children active contours ACwithoutEdges and ACwithoutEdgesYUV to calculate the means \a Cout and \a Cin in \a O(1) or \a O(#lists_length) with updates counting and not in \a O(#img_size).
    virtual void calculate_means();

    //! Specific step reimplemented in the children active contours ACwithoutEdges and ACwithoutEdgesYUV to update the variables to calculate the means \a Cout and \a Cin before each #switch_in, in the cycle 1.
    virtual void updates_for_means_in1();

    //! Specific step reimplemented in the children active contours ACwithoutEdges and ACwithoutEdgesYUV to update the variables to calculate the means \a Cout and \a Cin before each #switch_in, in the cycle 2.
    virtual void updates_for_means_in2(unsigned int offset);

    //! Specific step reimplemented in the children active contours ACwithoutEdges and ACwithoutEdgesYUV to update the variables to calculate the means \a Cout and \a Cin before each #switch_out, in the cycle 1.
    virtual void updates_for_means_out1();

    //! Specific step reimplemented in the children active contours ACwithoutEdges and ACwithoutEdgesYUV to update the variables to calculate the means \a Cout and \a Cin before each #switch_out, in the cycle 2.
    virtual void updates_for_means_out2(unsigned int offset);



    //! First stopping condition called at the end of each iteration of the cycle 1. The boolean #hasLastCycle2 is calculated.
    void calculate_stopping_conditions1();

    //! Second stopping condition called at the end of the cycle 2. The boolean #isStopped is calculated.
    void calculate_stopping_conditions2();



    //! Gives the sign of a value. Return the integer -1 or 1.
    static int signum_function(signed char value);


    //! Checks #shape_width_ratio1 or shape_height_ratio1.
    static double check_shape_ratio1(double value);

    //! Checks #kernel_length1.
    static unsigned int check_kernel_length1(unsigned int value);

    //! Checks #sigma1.
    static double check_sigma1(double value);

    //! Checks #Na1 or #Ns1.
    static unsigned int check_Na1_Ns1(unsigned int value);



    //! List of points belong to the outside boundary.
    ofeli::List<unsigned int> Lout;
    //! List of points belong to the inside boundary.
    ofeli::List<unsigned int> Lin;

    //! Number of times the active contour has evolved in the current cycle 1 with \a Fd speed.
    unsigned int Na;
    //! Number of times the active contour can evolve in a cycle 1 with \a Fd speed.
    const unsigned int Na_max;
    //! Number of times the active contour has evolved in the current cycle 2 with \a Fint speed.
    unsigned int Ns;
    //! Number of times the active contour can evolve in a cycle 2 with \a Fint speed.
    const unsigned int Ns_max;

    //! Boolean egals to \c true to have the curve smoothing, evolutions in the cycle 2 with the internal speed Fint.
    const bool hasCycle2;
    //! Gaussian kernel matrix used to calculate Fint.
    const ofeli::Matrix<unsigned int> gaussian_kernel;

    //! Number of times the active contour has evolved from the start.
    unsigned int iteration;
    //! Maximum number of times the active contour can evolve.
    const unsigned int iteration_max;

    //! Boolean egals to \c true if one point of #Lout at least is switched in during the scan through the #Lout list.
    bool hasOutwardEvolution;
    //! Boolean egals to \c true if one point of #Lin at least is switched out during the scan through the #Lin list.
    bool hasInwardEvolution;
    //! Boolean egals to true if #hasOutwardEvolution egals to \c true or #hasInwardEvolution egals to \c true.
    bool hasListsChanges;

    //! Length of #Lin + length of #Lout.
    unsigned int lists_length;
    //! Length of #Lin + length of #Lout of the previous iteration.
    unsigned int previous_lists_length;
    //! Boolean calculated at the end of the cycle 2 with function \a calculate_stopping_conditions2().
    bool hasOscillation;
    //! To count the number of times the relative gap of the lists length are less of a constant, fixed at 1%, i.e. the number of consecutive oscillations.
    unsigned int oscillations_in_a_row;

    //! Boolean calculated at the end of each iteration of the cycle 1 with function \a calculate_stopping_conditions1().
    bool hasLastCycle2;

    //! Boolean egals to \c true to stop the algorithm.
    bool isStopped;
};

// Definitions

template <typename T>
inline T ActiveContour::square(const T& value)
{
    return value*value;
}

inline int ActiveContour::signum_function(signed char value)
{
    return ( value < 0 ) ? -1 : 1;
}


template <typename T>
inline bool ActiveContour::isValid_matrix(const ofeli::Matrix<T>& matrix)
{
    bool isValid = true;

    if( matrix.isNull() )
    {
        isValid = false;

        std::cerr << std::endl <<
        "Precondition, the pointer encapsulated by matrix must be a non-null pointer, it must be allocated."
        << std::endl;
    }

    if( matrix.get_width() == 0 )
    {
        isValid = false;

        std::cerr << std::endl <<
        "Precondition, matrix width must be strictly positive."
        << std::endl;
    }
    if( matrix.get_height() == 0 )
    {
        isValid = false;

        std::cerr << std::endl <<
        "Precondition, matrix height must be strictly positive."
        << std::endl;
    }

    return isValid;
}


}

#endif // ACTIVE_CONTOUR_HPP



//! \class ofeli::ActiveContour
//! The class ActiveContour contains the implementation of the Fast-Two-Cycle (FTC) algorithm of Shi and Karl as it describes in their article "A real-time algorithm for the approximation of level-set based curve evolution" published in IEEE Transactions on Image Processing in may 2008.
//! This class should be never instantiated because the function to calculate the external speed \a Fd is too general. The 3 classes #ofeli::ACwithoutEdges, #ofeli::ACwithoutEdgesYUV and #ofeli::GeodesicAC, inherit of the class ActiveContour, with for each class it own implementation of this function.

/**
 * \fn ActiveContour::ActiveContour(const unsigned char* img_data1, int img_width1, int img_height1,
                                    bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                                    bool hasCycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1)
 * \param img_data1 Input pointer on the image data buffer. This buffer must be row-wise, except if you define the macro COLUMN_WISE_IMAGE_DATA_BUFFER. Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param hasEllipse1 Boolean to choose the shape of the active contour initialization, \c true for an ellipse or \c false for a rectangle.
 * \param shape_width_ratio1 Width of the shape divided by the image #img_width.
 * \param shape_height_ratio1 Height of the shape divided by the image #img_height.
 * \param center_x_ratio1 X-axis position or column index of the center of the shape divided by the image #img_width subtracted by 0.5.
 * \param center_y_ratio1 Y-axis position or row index of the center of the shape divided by the image #img_height subtracted by 0.5.\n
          To have the center of the shape into the image : -0.5 < center_x1 < 0.5 and -0.5 < center_y_ratio1 < 0.5.
 * \param hasCycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of maximum iterations the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of maximum iterations the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 */

/**
 * \fn ActiveContour::ActiveContour(const unsigned char* img_data1, int img_width1, int img_height1,
                                    const char* phi_init1,
                                    bool hasCycle2_1, int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1)
 * \param img_data1 Input pointer on the image data buffer. This buffer must be row-wise, except if you define the macro COLUMN_WISE_IMAGE_DATA_BUFFER. Passed to #img_data.
 * \param img_width1 Image width, i.e. number of columns. Passed to #img_width.
 * \param img_height1 Image height, i.e. number of rows. Passed to #img_height.
 * \param phi_init1 Pointer on the initialized level-set function buffer. Copied to #phi. #phi is checked and cleaned if needed after so \a phi_init1 can be a binarized buffer with 1 for outside region and -1 for inside region to simplify your task.
 * \param hasCycle2_1 Boolean to have or not the curve smoothing, evolutions in the cycle 2 with an internal speed Fint. Passed to #hasCycle2.
 * \param kernel_length1 Kernel length of the gaussian filter for the curve smoothing. Passed to #kernel_length.
 * \param sigma1 Standard deviation of the gaussian kernel for the curve smoothing. Passed to #sigma.
 * \param Na1 Number of maximum iterations the active contour evolves in the cycle 1, external or data dependant evolutions with \a Fd speed. Passed to #Na_max.
 * \param Ns1 Number of maximum iterations the active contour evolves in the cycle 2, curve smoothing or internal evolutions with \a Fint speed. Passed to #Ns_max.
 */

/**
 * \fn void ActiveContour::switch_in(ofeli::list<int>::iterator Lout_point)
 * \param Lout_point iterator which contains offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn void ActiveContour::switch_out(ofeli::list<int>::iterator Lin_point)
 * \param Lin_point which contains offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn int ActiveContour::compute_internal_speed_Fint(int offset)
 * \param offset offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 * \return Fint, the internal speed for the regularization of the active contour used in the cycle 2
 */

/**
 * \fn virtual int ActiveContour::compute_external_speed_Fd(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 * \return Fd, the external speed for the data dependant evolution of the active contour used in the cycle 1
 */

/**
 * \fn bool ActiveContour::isRedundantLinPoint(int offset) const
 * \param offset offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 * \return \c true if the point (\a x,\a y) of #Lin is redundant, otherwise, \c false
 */

/**
 * \fn bool ActiveContour::isRedundantLoutPoint(int offset) const
 * \param offset offset of the buffer pointed by #phi with \a offset = \a x + \a y × #img_width
 * \return \c true if the point (\a x,\a y) of #Lout is redundant, otherwise, \c false
 */

/**
 * \fn virtual void ActiveContour::updates_for_means_in2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \fn virtual void ActiveContour::updates_for_means_out2(int offset)
 * \param offset offset of the image data buffer with \a offset = \a x + \a y × #img_width
 */

/**
 * \example interface
 * \code
 * // It interfaces the algorithm in order to display each iteration
 *
 * #include "ac_withoutedges.hpp"
 * #include "List.hpp"
 * #include <iostream>
 *
 * int offset // offset of the current pixel
 * unsigned char I // intensity of the current pixel
 *
 * // Lout displayed in blue
 * unsigned char Rout = 0;
 * unsigned char Gout = 0;
 * unsigned char Bout = 255;
 *
 * // Lin displayed in red
 * unsigned char Rin = 255;
 * unsigned char Gin = 0;
 * unsigned char Bin = 0;
 *
 *
 * ofeli::ACwithoutEdges ac(img1, img_width1, img_height1);
 *
 *
 * -----------------------------   Display the initial active contour   -----------------------------
 *
 * const ofeli::list<int>* Lout = &ac.get_Lout();
 * const ofeli::list<int>* Lin = &ac.get_Lin();
 *
 * // put the color of lists into the displayed buffer
 * for( auto it = Lout->get_begin(); !it.end(); ++it )
 * {
 *     offset = *it*3;
 *
 *     img_displayed[offset] = Rout;
 *     img_displayed[offset+1] = Gout;
 *     img_displayed[offset+2] = Bout;
 * }
 *
 * for( auto it = Lin->get_begin(); !it.end(); ++it )
 * {
 *     offset = *it*3;
 *
 *     img_displayed[offset] = Rin;
 *     img_displayed[offset+1] = Gin;
 *     img_displayed[offset+2] = Bin;
 * }
 *
 * // paint event, refresh the widget witch display the buffer img_displayed
 * update();
 *
 * // display the position of the active contour in the standard output
 * std::cout << ac << std::endl;
 *
 * --------------------------------------------------------------------------------------------------
 *
 *
 * // Loop for the evolution of the active contour
 * while( !ac.get_isStopped() )
 * {
 *     // erase the previous lists Lout1 and Lin1 of the displayed buffer
 *     for( auto it = Lout->get_begin(); !it.end(); ++it )
 *     {
 *         offset = *it;
 *
 *         I = img1[offset];
 *
 *         img_displayed[3*offset] = I;
 *         img_displayed[3*offset+1] = I;
 *         img_displayed[3*offset+2] = I;
 *     }
 *
 *     for( auto it = Lin->get_begin(); !it.end(); ++it )
 *     {
 *         offset = *it;
 *
 *         I = img1[offset];
 *
 *         img_displayed[3*offset] = I;
 *         img_displayed[3*offset+1] = I;
 *         img_displayed[3*offset+2] = I;
 *     }
 *
 *     // to evolve the active contour of one iteration or one step
 *     ++ac;
 *
 *     // to get the temporary result
 *     Lout = &ac.get_Lout();
 *     Lin = &ac.get_Lin();
 *
 *     // put the color of lists into the displayed buffer
 *     for( auto it = Lout->get_begin(); !it.end(); ++it )
 *
 *         offset = *it*3;
 *
 *         img_displayed[offset] = Rout;
 *         img_displayed[offset+1] = Gout;
 *         img_displayed[offset+2] = Bout;
 *     }
 *
 *     for( auto it = Lin->get_begin(); !it.end(); ++it )
 *
 *         offset = *it*3;
 *
 *         img_displayed[offset] = Rin;
 *         img_displayed[offset+1] = Gin;
 *         img_displayed[offset+2] = Bin;
 *     }
 *
 *     // paint event, refresh the widget witch display the buffer img_displayed
 *     update();
 * }
 *
 * \endcode
 */
