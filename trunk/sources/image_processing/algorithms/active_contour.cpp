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

#include "active_contour.hpp"
#include <cstring> // for the function "std::memcpy"
#include <cmath> // for the function "std::exp"
#include <cstdlib> // for the function "std::abs"
#include <algorithm> // for the function "std::max"

namespace ofeli
{

ActiveContour::ActiveContour(unsigned int phi_width, unsigned int phi_height,
                             bool hasEllipse1, double shape_width_ratio1, double shape_height_ratio1, double center_x_ratio1, double center_y_ratio1,
                             bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1) :
    phi(phi_width,phi_height), Lout( std::max(10000u,std::min(100000u,phi_width*phi_height/5u)) ), Lin( std::max(10000u,std::min(100000u,phi_width*phi_height/5u)) ),
    Na(0u), Na_max(check_Na1_Ns1(Na1)), Ns(0u), Ns_max(check_Na1_Ns1(Ns1)),
    hasCycle2(hasCycle2_1), gaussian_kernel(check_kernel_length1(kernel_length1),check_sigma1(sigma1)),
    iteration(0u), iteration_max( 5u*std::max(phi_width,phi_height) ),
    lists_length(0u), previous_lists_length(99999999u), oscillations_in_a_row(0u)
{
    if( phi_width == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, phi_width must be strictly positive."
        << std::endl;
    }
    if( phi_height == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, phi_height must be strictly positive."
        << std::endl;
    }

    initialize_phi_with_a_shape(hasEllipse1,
                                check_shape_ratio1(shape_width_ratio1),
                                check_shape_ratio1(shape_height_ratio1),
                                center_x_ratio1,
                                center_y_ratio1);
    initialize_lists();
    ActiveContour::initialize_for_each_frame();
}

ActiveContour::ActiveContour(const ofeli::Matrix<const signed char>& phi_init1,
                             bool hasCycle2_1, unsigned int kernel_length1, double sigma1, unsigned int Na1, unsigned int Ns1) :
    phi(phi_init1), Lout( std::max(10000u,std::min(100000u,phi_init1.get_width()*phi_init1.get_height()/5u)) ), Lin( std::max(10000u,std::min(100000u,phi_init1.get_width()*phi_init1.get_height()/5u)) ),
    Na(0u), Na_max(check_Na1_Ns1(Na1)), Ns(0u), Ns_max(check_Na1_Ns1(Ns1)),
    hasCycle2(hasCycle2_1), gaussian_kernel(check_kernel_length1(kernel_length1),check_sigma1(sigma1)),
    iteration(0u), iteration_max( 5u*std::max(phi_init1.get_width(),phi_init1.get_height()) ),
    lists_length(0u), previous_lists_length(99999999u), oscillations_in_a_row(0u)
{
    if( !isValid_matrix(phi_init1) )
    {
        std::cerr << std::endl <<
        "phi_init1 is not valid."
        << std::endl;
    }

    initialize_lists();
    ActiveContour::initialize_for_each_frame();
}

ActiveContour::ActiveContour(const ActiveContour& ac) :
    phi(ac.phi), Lout(ac.Lout), Lin(ac.Lin), // Matrix and List have an implemented copy constructor
    Na(ac.Na), Na_max(ac.Na_max), Ns(ac.Ns), Ns_max(ac.Ns_max),
    hasCycle2(ac.hasCycle2), gaussian_kernel(ac.gaussian_kernel),
    iteration(ac.iteration), iteration_max(ac.iteration_max),
    hasOutwardEvolution(ac.hasOutwardEvolution), hasInwardEvolution(ac.hasInwardEvolution), hasListsChanges(ac.hasListsChanges),
    lists_length(ac.lists_length), previous_lists_length(ac.previous_lists_length), hasOscillation(ac.hasOscillation), oscillations_in_a_row(ac.oscillations_in_a_row),
    hasLastCycle2(ac.hasLastCycle2), isStopped(ac.isStopped)
{
}

double ActiveContour::check_shape_ratio1(double value)
{
    if( value <= 0.0 )
    {
        std::cerr << std::endl <<
        "Precondition, shape_width_ratio1 or shape_height_ratio1 must be strictly positive. It is set to 0.0001."
        << std::endl;

        value = 0.0001;
    }
    return value;
}

unsigned int ActiveContour::check_kernel_length1(unsigned int value)
{
    if( value < 3 )
    {
        std::cerr << std::endl <<
        "Precondition, kernel_length1 must not be less than 3. It is set to 3."
        << std::endl;
        value = 3;
    }
    else if( value % 2 == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, kernel_length1 must be odd. It is decremented."
        << std::endl;
        value--;
    }

    return value;
}

double ActiveContour::check_sigma1(double value)
{
    if( value < 0.000000001 )
    {
        std::cerr << std::endl <<
        "Precondition, sigma1 is positive and must not equal to zero. It is set to 0.000000001."
        << std::endl;

        value = 0.000000001;
    }
    return value;
}

unsigned int ActiveContour::check_Na1_Ns1(unsigned int value)
{
    if( value == 0 )
    {
        std::cerr << std::endl <<
        "Precondition, Na1 or Ns1 is positive and must not equal to zero. It is set to 1."
        << std::endl;

        value = 1;
    }
    return value;
}

int ActiveContour::check_value(int value)
{
    if( value < 1 )
    {
        std::cerr << std::endl <<
        "Precondition, lambda_out1, lambda_in1, alpha1, beta1 or gamma1 is positive and must not equal to zero. It is set to 1."
        << std::endl;

        value = 1;
    }
    return value;
}

void ActiveContour::initialize_phi_with_a_shape(bool hasEllipse, double shape_width_ratio, double shape_height_ratio,
                                                double center_x_ratio, double center_y_ratio)
{
    const unsigned int phi_width = phi.get_width();
    const unsigned int phi_height = phi.get_height();
    const double shape_width = shape_width_ratio*double(phi_width);
    const double shape_height = shape_height_ratio*double(phi_height);

    if( hasEllipse ) // performs an ellipse
    {
        const double center_x = (center_x_ratio+1.0)*double(phi_width);
        const double center_y = (center_y_ratio+1.0)*double(phi_height);

        for( unsigned int y = 0; y < phi_height; y++ )
        {
            for( unsigned int x = 0; x < phi_width; x++ )
            {
                // ellipse inequation
                if(   square( double(x)-center_x/2.0 ) / square( shape_width/2.0 )
                    + square( double(y)-center_y/2.0 ) / square( shape_height/2.0 )
                    > 1.0
                  )
                {
                    phi(x,y) = 1; // outside boundary value
                }
                else
                {
                    phi(x,y) = -1; // inside boundary value
                }
            }
        }
    }
    else // performs a rectangle
    {
        const double center_x = (center_x_ratio+0.5)*double(phi_width);
        const double center_y = (center_y_ratio+0.5)*double(phi_height);

        // bounding box with point(x1,y1) and point(x2,y2)
        const unsigned int x1 = (unsigned int)(center_x-shape_width/2.0);
        const unsigned int x2 = (unsigned int)(center_x+shape_width/2.0);
        const unsigned int y1 = (unsigned int)(center_y-shape_height/2.0);
        const unsigned int y2 = (unsigned int)(center_y+shape_height/2.0);

        for( unsigned int y = 0; y < phi_height; y++ )
        {
            for( unsigned int x = 0; x < phi_width; x++ )
            {
                if( x > x1 && x < x2 && y > y1 && y < y2 )
                {
                    phi(x,y) = -1; // inside boundary value
                }
                else
                {
                    phi(x,y) = 1; // outside boundary value
                }
            }
        }
    }
}

void ActiveContour::initialize_lists()
{
    // each point of a list must be connected at least by one point of the other list
    // eliminate redundant points in phi if needed, and initialize Lout and Lin
    // so you can pass to the constructor phi_init1, a binarized buffer
    // with 1 for outside region and -1 for inside region to simplify your task
    const unsigned int phi_size = phi.get_width()*phi.get_height();

    for( unsigned int offset = 0; offset < phi_size; offset++ )
    {
        if( phi[offset] == 1 ) // outside boundary value
        {
            if( isRedundantLoutPoint(offset) )
            {
                phi[offset] = 3; // exterior value
            }
            else
            {
                Lout.push_front(offset);
            }
        }

        if( phi[offset] == -1 ) // inside boundary value
        {
            if( isRedundantLinPoint(offset) )
            {
                phi[offset] = -3; // interior value
            }
            else
            {
                Lin.push_front(offset);
            }
        }
    }
}

void ActiveContour::initialize_for_each_frame()
{
    if( Lout.empty() && Lin.empty() )
    {
        std::cerr << std::endl <<
        "The both lists Lout and Lin are empty so the algorithm could not converge. The active contour is initialized with an ellipse."
        << std::endl;

        initialize_phi_with_a_shape(true, 0.95, 0.95, 0.0, 0.0);
        initialize_lists();
    }

    // 3 stopping conditions (re)initialized
    iteration = 0;

    hasListsChanges = true;

    hasOscillation = false;
    oscillations_in_a_row = 0;



    hasLastCycle2 = false;

    isStopped = false;
}

ActiveContour::~ActiveContour()
{
    //delete[] gaussian_kernel;
    //delete[] phi;
}

void ActiveContour::do_one_iteration_in_cycle1()
{
    // means of the Chan-Vese model for children classes ACwithoutEdges and ACwithoutEdgesYUV
    calculate_means(); // virtual function for region-based models


    hasOutwardEvolution = false;

    for( auto Lout_point = Lout.begin(); !Lout_point.end();   )
    {
        if( compute_external_speed_Fd(*Lout_point) > 0 )
        {
            hasOutwardEvolution = true;

            // updates of the variables to calculate the means Cout and Cin
            updates_for_means_in1(); // virtual function for region-based models

            Lout_point = switch_in(Lout_point); // outward local movement
            // switch_in function returns a new Lout_point
            // which is the next point of the former Lout_point
        }
        else
        {
            ++Lout_point;
        }
    }


    clean_Lin(); // eliminate Lin redundant points


    hasInwardEvolution = false;

    for( auto Lin_point = Lin.begin(); !Lin_point.end();   )
    {
        if( compute_external_speed_Fd(*Lin_point) < 0 )
        {
            hasInwardEvolution = true;

            // updates of the variables to calculate the means Cout and Cin
            updates_for_means_out1(); // virtual function for region-based models

            Lin_point = switch_out(Lin_point); // inward local movement
            // switch_out function returns a new Lin_point
            // which is the next point of the former Lin_point
        }
        else
        {
            ++Lin_point;
        }
    }


    clean_Lout(); // eliminate Lout redundant points


    iteration++;
}

void ActiveContour::do_one_iteration_in_cycle2()
{
    unsigned int offset;

    lists_length = 0;


    for( auto Lout_point = Lout.begin(); !Lout_point.end();   )
    {
        offset = *Lout_point;

        if( compute_internal_speed_Fint(offset) > 0 )
        {
            // updates of the variables to calculate the means Cout and Cin
            updates_for_means_in2(offset); // virtual function for region-based models

            Lout_point = switch_in(Lout_point); // outward local movement
            // switch_in function returns a new Lout_point
            // which is the next point of the former Lout_point
        }
        else
        {
            lists_length++;
            ++Lout_point;
        }
    }


    clean_Lin(); // eliminate Lin redundant points


    for( auto Lin_point = Lin.begin(); !Lin_point.end();   )
    {
        offset = *Lin_point;

        if( compute_internal_speed_Fint(offset) < 0 )
        {
            // updates of the variables to calculate the means Cout and Cin
            updates_for_means_out2(offset); // virtual function for region-based models

            Lin_point = switch_out(Lin_point); // inward local movement
            // switch_out function returns a new Lin_point
            // which is the next point of the former Lin_point
        }
        else
        {
            lists_length++;
            ++Lin_point;
        }
    }


    clean_Lout(); // eliminate Lout redundant points


    iteration++;
}

void ActiveContour::evolve_one_iteration()
{

    // Fast Two Cycle algorithm

    while( !isStopped )
    {

        ////////   cycle 1 : Na_max times, data dependant evolution   ////////
        while( Na < Na_max && !hasLastCycle2 )
        {
            do_one_iteration_in_cycle1();
            calculate_stopping_conditions1(); // it computes hasLastCycle2
            Na++;
            return; // just one iteration is performed
        }
        //////////////////////////////////////////////////////////////////////

        if( hasCycle2 )
        {
            ////   cycle 2 : Ns_max times, regularization of the active contour   ////
            while( Ns < Ns_max )
            {
                do_one_iteration_in_cycle2();
                Ns++;
                return; // just one iteration is performed
            }
            //////////////////////////////////////////////////////////////////////////

            if( hasLastCycle2 ) // a last cycle 2 has been performed before
            {
                isStopped = true;
            }
            else
            {
                calculate_stopping_conditions2(); // it computes isStopped
            }
        }
        else
        {
            if( hasLastCycle2 )
            {
                isStopped = true;
            }
        }

        Na = 0;
        Ns = 0;
    }
}

void ActiveContour::evolve()
{

    // Fast Two Cycle algorithm

    while( !isStopped )
    {

        ////////   cycle 1 : Na_max times, data dependant evolution   ////////
        while( Na < Na_max && !hasLastCycle2 )
        {
            do_one_iteration_in_cycle1();
            calculate_stopping_conditions1(); // it computes hasLastCycle2
            Na++;
        }
        //////////////////////////////////////////////////////////////////////

        if( hasCycle2 )
        {
            ////   cycle 2 : Ns_max times, regularization of the active contour   ////
            while( Ns < Ns_max )
            {
                do_one_iteration_in_cycle2();
                Ns++;
            }
            //////////////////////////////////////////////////////////////////////////

            if( hasLastCycle2 ) // a last cycle 2 has been performed before
            {
                isStopped = true;
            }
            else
            {
                calculate_stopping_conditions2(); // it computes isStopped
            }
        }
        else
        {
            if( hasLastCycle2 )
            {
                isStopped = true;
            }
        }

        Na = 0;
        Ns = 0;
    }
}

void ActiveContour::add_Rout_neighbor_to_Lout(unsigned int neighbor_offset)
{
    // if a neighbor ∈ Rout
    if( phi[neighbor_offset] == 3 ) // exterior value
    {
        phi[neighbor_offset] = 1; // outside boundary value

        // neighbor ∈ Rout ==> ∈ neighbor Lout
        Lout.push_front(neighbor_offset);
        // due to the linked list implementation
        // with a sentinel/dummy node after the last node and not before the first node ;
        // 'push_front' never invalidates iterator 'Lout_point', even if 'Lout_point' points to the first node.
    }
}

void ActiveContour::add_Rin_neighbor_to_Lin(unsigned int neighbor_offset)
{
    // if a neighbor ∈ Rin
    if( phi[neighbor_offset] == -3 ) // interior value
    {
        phi[neighbor_offset] = -1; // inside boundary value

        // neighbor ∈ Rin ==> ∈ neighbor Lin
        Lin.push_front(neighbor_offset);
        // due to the linked list implementation
        // with a sentinel/dummy node after the last node and not before the first node ;
        // 'push_front' never invalidates iterator 'Lin_point', even if 'Lin_point' points to the first node.
    }
}

ofeli::List<unsigned int>::iterator ActiveContour::switch_in(ofeli::List<unsigned int>::iterator Lout_point)
{
    unsigned int offset, x, y;
    offset = *Lout_point;
    phi.get_position(offset,x,y); // x and y passed by reference

    // Outward local movement

    if( x != 0 )
    {
        add_Rout_neighbor_to_Lout( phi.get_offset(x-1,y) );
    }
    if( x != phi.get_width()-1 )
    {
        add_Rout_neighbor_to_Lout( phi.get_offset(x+1,y) );
    }

    if( y != 0 )
    {
        add_Rout_neighbor_to_Lout( phi.get_offset(x,y-1) );

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                add_Rout_neighbor_to_Lout( phi.get_offset(x-1,y-1) );
            }
            if( x != phi.get_width()-1 )
            {
                add_Rout_neighbor_to_Lout( phi.get_offset(x+1,y-1) );
            }
        }
    }

    if( y != phi.get_height()-1 )
    {
        add_Rout_neighbor_to_Lout( phi.get_offset(x,y+1) );

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                add_Rout_neighbor_to_Lout( phi.get_offset(x-1,y+1) );
            }
            if( x != phi.get_width()-1 )
            {
                add_Rout_neighbor_to_Lout( phi.get_offset(x+1,y+1) );
            }
        }
    }

    phi[offset] = -1; // 1 ==> -1
    return Lin.splice_front(Lout_point); // Lout_point ∈ Lout ==> Lout_point ∈ Lin
    // return a new Lout_point which is the next point of the former Lout_point
    // obviously, this new Lout_point ∈ Lout
}

ofeli::List<unsigned int>::iterator ActiveContour::switch_out(ofeli::List<unsigned int>::iterator Lin_point)
{
    unsigned int offset, x, y;
    offset = *Lin_point;
    phi.get_position(offset,x,y); // x and y passed by reference

    // Inward local movement

    if( x != 0 )
    {
        add_Rin_neighbor_to_Lin( phi.get_offset(x-1,y) );
    }
    if( x != phi.get_width()-1 )
    {
        add_Rin_neighbor_to_Lin( phi.get_offset(x+1,y) );
    }

    if( y != 0 )
    {
        add_Rin_neighbor_to_Lin( phi.get_offset(x,y-1) );

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                add_Rin_neighbor_to_Lin( phi.get_offset(x-1,y-1) );
            }
            if( x != phi.get_width()-1 )
            {
                add_Rin_neighbor_to_Lin( phi.get_offset(x+1,y-1) );
            }
        }
    }

    if( y != phi.get_height()-1 )
    {
        add_Rin_neighbor_to_Lin( phi.get_offset(x,y+1) );

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                add_Rin_neighbor_to_Lin( phi.get_offset(x-1,y+1) );
            }
            if( x != phi.get_width()-1 )
            {
                add_Rin_neighbor_to_Lin( phi.get_offset(x+1,y+1) );
            }
        }
    }

    phi[offset] = 1; // -1 ==> 1
    return Lout.splice_front(Lin_point); // Lin_point ∈ Lin ==> Lin_point ∈ Lout
    // return a new Lin_point which is the next point of the former Lin_point
    // obviously, this new Lin_point ∈ Lin
}

int ActiveContour::compute_internal_speed_Fint(unsigned int offset)
{
    unsigned int x, y;
    phi.get_position(offset,x,y); // x and y passed by reference

    const int kernel_radius = (gaussian_kernel.get_width() - 1) / 2;
    int Fint = 0;

    // if not in the image's border, no neighbors' tests
    if(    int(x) > kernel_radius-1 && int(x) < int(phi.get_width())-kernel_radius
        && int(y) > kernel_radius-1 && int(y) < int(phi.get_height())-kernel_radius
      )
    {
        for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
        {
            for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
            {
                Fint += gaussian_kernel(kernel_radius+dx,kernel_radius+dy)
                        * signum_function( -phi(int(x)+dx,int(y)+dy) );
            }
        }
    }
    // if in the border of the image, tests of neighbors
    else
    {
        for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
        {
            for(int dx = -kernel_radius; dx <= kernel_radius; dx++ )
            {
                if(    int(x)+dx >= 0 && int(x)+dx < int(phi.get_width())
                    && int(y)+dy >= 0 && int(y)+dy < int(phi.get_height())
                  )
                {
                    Fint += gaussian_kernel(kernel_radius+dx,kernel_radius+dy)
                            * signum_function( -phi(int(x)+dx,int(y)+dy) );
                }
                else
                {
                    Fint += gaussian_kernel(kernel_radius+dx,kernel_radius+dy)
                            * signum_function( -phi[offset] );
                }
            }
        }
    }

    return Fint;
}

bool ActiveContour::isRedundantLinPoint(unsigned int offset) const
{
    unsigned int x, y;
    phi.get_position(offset,x,y); // x and y passed by reference

    if( x != 0 )
    {
        if( phi(x-1,y) >= 0 )
        {
            return false;
        }
    }
    if( x != phi.get_width()-1 )
    {
        if( phi(x+1,y) >= 0 )
        {
            return false;
        }
    }

    if( y != 0 )
    {
        if( phi(x,y-1) >= 0 )
        {
            return false;
        }

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                if( phi(x-1,y-1) >= 0 )
                {
                    return false;
                }
            }
            if( x != phi.get_width()-1 )
            {
                if( phi(x+1,y-1) >= 0 )
                {
                    return false;
                }
            }
        }
    }

    if( y != phi.get_height()-1 )
    {
        if( phi(x,y+1) >= 0 )
        {
            return false;
        }

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                if( phi(x-1,y+1) >= 0 )
                {
                    return false;
                }
            }
            if( x != phi.get_width()-1 )
            {
                if( phi(x+1,y+1) >= 0 )
                {
                    return false;
                }
            }
        }
    }

    // ==> ∀ neighbors ∈ Lin | ∈ Rin
    return true;
}

bool ActiveContour::isRedundantLoutPoint(unsigned int offset) const
{
    unsigned int x, y;
    phi.get_position(offset,x,y); // x and y passed by reference

    if( x != 0 )
    {
        if( phi(x-1,y) < 0 )
        {
            return false;
        }
    }
    if( x != phi.get_width()-1 )
    {
        if( phi(x+1,y) < 0 )
        {
            return false;
        }
    }

    if( y != 0 )
    {
        if( phi(x,y-1) < 0 )
        {
            return false;
        }

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                if( phi(x-1,y-1) < 0 )
                {
                    return false;
                }
            }
            if( x != phi.get_width()-1 )
            {
                if( phi(x+1,y-1) < 0 )
                {
                    return false;
                }
            }
        }
    }

    if( y != phi.get_height()-1 )
    {
        if( phi(x,y+1) < 0 )
        {
            return false;
        }

        if( HAS_8_CONNEXITY )
        {
            if( x != 0 )
            {
                if( phi(x-1,y+1) < 0 )
                {
                    return false;
                }
            }
            if( x != phi.get_width()-1 )
            {
                if( phi(x+1,y+1) < 0 )
                {
                    return false;
                }
            }
        }
    }

    // ==> ∀ neighbors ∈ Lout | ∈ Rout
    return true;
}

void ActiveContour::clean_Lin()
{
    unsigned int offset;

    for( auto Lin_point = Lin.begin(); !Lin_point.end();   )
    {
        offset = *Lin_point;

        // if ∀ neighbors ∈ Lin | ∈ Rin
        if( isRedundantLinPoint(offset) )
        {
            phi[offset] = -3; // -1 ==> -3
            Lin_point = Lin.erase(Lin_point); // Lin_point ∈ Lin ==> Lin_point ∈ Rin
            // erase function returns a new Lin_point
            // which is the next point of the former Lin_point
        }
        else
        {
            ++Lin_point;
        }
    }
}

void ActiveContour::clean_Lout()
{
    unsigned int offset;

    for( auto Lout_point = Lout.begin(); !Lout_point.end();   )
    {
        offset = *Lout_point;

        // if ∀ neighbors ∈ Lout | ∈ Rout
        if( isRedundantLoutPoint(offset) )
        {
            phi[offset] = 3; // 1 ==> 3
            Lout_point = Lout.erase(Lout_point); // Lout_point ∈ Lout ==> Lout_point ∈ Rout
            // erase function returns a new Lout_point
            // which is the next point of the former Lout_point
        }
        else
        {
            ++Lout_point;
        }
    }
}

// input integer is an offset
int ActiveContour::compute_external_speed_Fd(unsigned int)
{
    // this class should never be instantiated

    return -1;
    // always an inward movement in each point of the active contour,
    // this speed is not very discriminant...

    // reimplement a better and data-dependent speed function in a child class
}

// at the end of each iteration in the cycle 1
void ActiveContour::calculate_stopping_conditions1()
{
    if( !hasInwardEvolution && !hasOutwardEvolution )
    {
        hasListsChanges = false;
    }


    if( !hasListsChanges || iteration >= iteration_max )
    {
        hasLastCycle2 = true;
    }
}

// at the end of the cycle 2
void ActiveContour::calculate_stopping_conditions2()
{
    // if the relative difference of active contour length between two cycle2 is less of 1%
    if(   double( std::abs(int(previous_lists_length)-int(lists_length)) )
        / double(previous_lists_length)
        < 0.01
      )
    {
        oscillations_in_a_row++;
    }
    else
    {
        oscillations_in_a_row = 0;
    }
    // keep last length to compare after
    previous_lists_length = lists_length;

    // if 3 times consecutively
    if( oscillations_in_a_row == 3 )
    {
        hasOscillation = true;
    }

    if( hasOscillation || iteration >= iteration_max )
    {
        isStopped = true;
    }
}

// to display the active contour position in the standard output
// std::cout << ac << std::endl;
std::ostream& operator<<(std::ostream& os, const ActiveContour& ac)
{
    os << std::endl << " -----------------------" << std::endl;
    os << "         Lout(" << ac.get_iteration() << ")" << std::endl;
    os << " -----------------------" << std::endl;
    os << "  index |     x | y     " << std::endl;
    os << " -----------------------" << std::endl;

    unsigned int x, y;

    int index = 0;
    for( auto Lout_point = ac.get_Lout().begin(); !Lout_point.end(); ++Lout_point )
    {
        ac.phi.get_position(*Lout_point,x,y); // x and y passed by reference

        os.width(7);
        os << std::right << index << " |";

        os.width(6);
        os << std::right << x << " | " << y << std::endl;
        index++;
    }
    os << " -----------------------" << std::endl;


    os << std::endl << " -----------------------" << std::endl;
    os << "         Lin(" << ac.get_iteration() << ")" << std::endl;
    os << " -----------------------" << std::endl;
    os << "  index |     x | y     " << std::endl;
    os << " -----------------------" << std::endl;

    index = 0;
    for( auto Lin_point = ac.get_Lin().begin(); !Lin_point.end(); ++Lin_point )
    {
        ac.phi.get_position(*Lin_point,x,y); // x and y passed by reference

        os.width(7);
        os << std::right << index << " |";

        os.width(6);
        os << std::right << x << " | " << y << std::endl;
        index++;
    }
    os << " -----------------------" << std::endl;

    return os;
}

void ActiveContour::display() const
{
    std::cout << *this;
}



void ActiveContour::calculate_means()
{
}

void ActiveContour::updates_for_means_in1()
{
}

void ActiveContour::updates_for_means_out1()
{
}

// input integer is an offset
void ActiveContour::updates_for_means_in2(unsigned int)
{
}

// input integer is an offset
void ActiveContour::updates_for_means_out2(unsigned int)
{
}

}
