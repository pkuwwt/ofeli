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

//! \file main.cpp
//! \brief Ofeli
//! \author Fabien Bessy
//! \version 1.1.0
//! \date September 2013

/*! \mainpage Developer's documentation
 *
 * \section intro_sec Introduction
 *
 * <p>Ofeli, as an acronym for <b>O</b>pen, <b>F</b>ast and <b>E</b>fficient <b>L</b>evel set
   <b>I</b>mplementation, demonstrates how to operate an image
   segmentation algorithm of Y. Shi and W. C. Karl <b>[1],</b> using a
   discrete approach for the approximation of level-set-based
   curve evolution (implicit active contours).</p>
   <p>This is a novel (2005) and fast algorithm without the need
   of solving partial differential equations (PDE) while preserving the
   advantages of level set methods, such as the automatic handling of
   topological changes. Considerable speedups (×100) have been
   demonstrated as compared to PDE-based narrow band level-set implementations.</p>
   <p>The Home Page of Ofeli can be found at : http://code.google.com/p/ofeli/ .</p>

 * \section structure_sec Structure
 *
 * In this project, the <b>G</b>raphical <b>U</b>ser <b>I</b>nterface (GUI) is clearly separated of the image processing part.
 *
 * GUI part :
 * - MainWindow.cpp
 * - SettingsWindow.cpp
 * - EvaluationWindow.cpp
 * - CameraWindow.cpp
 * - CameraInterface.cpp
 * - AboutWindow.cpp
 * - LanguageWindow.cpp
 * - PixmapWidget.cpp
 * - ScrollAreaWidget.cpp
 *
 * Image processing part :
 * - List.tpp
 * - ActiveContour.cpp
 *      - RegionBasedActiveContour.cpp
 *      - RegionBasedActiveContourYUV.cpp
 *      - EdgeBasedActiveContour.cpp
 * - HausdorffDistance.cpp
 * - Filters.cpp
 *
 *
 * \section reusability_sec Reusability
 *
 * <p>This Qt project is built as an application and not as a static or shared library. So if you are interested to use this C++ code, especially for the image processing part of the project,
 * you must just include this file(s) thanks to the preprocessor directive in your file(s). After you must just pass to each constructor an input argument pointer on a row-wise image data buffer and for the class ACwithoutEdgesYUV, a pointer on a RGB interleaved data buffer (R1 G1 B1 R2 G2 B2 ...).</p>
 * <p>If you prefer a command-line interface or if you are interested in a tracking example of this algorithm, you can find a fork of this project interfaced with Matlab (MEX-file). Each constructor takes an input pointer on a column-wise image data buffer and for the class ACwithoutEdgesYUV, a pointer on a RGB planar data buffer (R1 R2 R3 ... G1 G2 G3 ... B1 B2 B3 ...).</p>
 *
 *
 * \section license_sec License
 *
 * This software is distributed under the <a href='http://www.cecill.info/licences/Licence_CeCILL_V2-en.html'>CeCILL license version 2</a> <a href='http://www.cecill.info/licences/Licence_CeCILL_V2-fr.html'> (link to the french version here)</a>.
 *
 * \section acknowl_sec Acknowledgments
 *
 * Thanks to :
 * - J. Olivier, R. Boné, J-M. Girault, F. Amed, A. Lissy, C. Rouzière, L. Suta.
 * - <i>pattern recognition and image analysis research team</i>, <i>computer science laboratory</i>,
 * <i>François Rabelais University</i>.
 * - students and professors of the MSc in medical imaging.

<hr></hr>
* \section ref_sec References
<p><b>[1]</b> Y. Shi, W. C. Karl - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_MGIwYmUwYzctYTRkMC00ODMwLWI3YmUtNTFjYThlMTBkOTIy&hl=en&authkey=CPT1xeYN'>A real-time algorithm for the approximation of level-set based curve evolution</a> - <i>IEEE Trans. Image Processing</i>, vol. 17, no. 5, May 2008</p>
<p><b>[2]</b> T. F. Chan, L. A. Vese - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_NWY5ZGMyMmYtNzkwNi00NjI0LWE4ZGMtODllZTVmZWQ5NGRm&hl=en&authkey=CNfMkNEI'>Active contours without edges</a> - <i>IEEE Trans. Image Processing</i>, vol. 10, no. 2, Feb 2001</p>
<p><b>[3]</b> V. Caselles, R. Kimmel, G. Sapiro - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_ZWEzNzk2ZjgtNzlkMi00NDY0LTkzZjQtYWQ5N2EyNDA5NGE3&hl=en&authkey=CKi1w7cE'>Geodesic active contours</a> - <i>International Journal of Computer Vision</i>, 22(1), 61–79 (1997)</p>
<p><b>[4]</b> P. Perona, J. Malik - <a href='https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_NmJmZWZkM2ItN2ZhZS00NjA4LTk3Y2UtNTNmYzkxYjFjNjU4&hl=en&authkey=CPDnxN8H'>Scale-space and edge detection using anistropic diffusion</a> - <i>IEEE Trans. Pattern Analysis and Machine Intelligence</i>, vol. 12, no. 17, Jul 1990</p>
 */

#include "main_window.hpp"

#include <QApplication>
#include <QCoreApplication>

int main( int argc, char* argv[] )
{
    QTextCodec::setCodecForLocale( QTextCodec::codecForName("UTF-8") );
    //QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8") );
    //QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8") );

    QApplication app(argc, argv);

    QSettings settings("Bessy", "Ofeli"); //settings.clear();
    int language = settings.value("Language/current_index",0).toInt();

    QTranslator translator_qt_widget;
    QTranslator translator_ofeli;
    QString locale;

    switch( language )
    {
    case 0 :
    {
        locale = QLocale::system().name().section('_', 0, 0);
        break;
    }
    case 1 :
    {
        locale = QString("en");
        break;
    }
    case 2 :
    {
        locale = QString("fr");
        break;
    }
    default :
    {
        locale = QString("en");
    }
    }

    translator_qt_widget.load( QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath) );
    app.installTranslator(&translator_qt_widget);

    translator_ofeli.load( QString(":Ofeli_") + locale );
    app.installTranslator(&translator_ofeli);

    ofeli::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

/*
    list<float> l(500);
    l.push_front(5);
    l.push_front(15);
    l.push_front(-5);
    l.push_front(54);
    l.push_front(-5);
    l.push_front(13.9);
    l.push_front(14);
    l.push_front(15);
    l.push_front(2);
    l.push_front(0);
    l.push_front(-8);

    l.sort();

    list<float> ll(1000);
    ll.push_front(59);
    ll.push_front(125);

    l.display();
    ll.display();

    l.display();
    ll.display();*/
    /*ll.push_front(-59);
    ll.push_front(59);
    ll.push_front(-53);
    ll.push_front(13.29);
    ll.push_front(14);
    ll.push_front(15);
    ll.push_front(20);
    ll.push_front(0);
    ll.push_front(-82);*/

    //l.display();
    //ll.display();
    //l.splice_front(ll);

   // ll.display();

   // l = ll;
    //ll.clear();

    //l.display();
    //ll.display();
    //l.splice_front(ll);

    //ll.display();
    //ll.display();

    //l.sort(greater<float>());
    //l.display();
    /*l.sort();
    l.display();
    l.sort(greater<float>());
    l.display();
    l.remove_if(predicate_example<float>());
    l.display();
    l.put_away(ll);
    l.display();*/
