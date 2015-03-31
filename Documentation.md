# Documentation #

You can find the developer's documentation [here](http://ofeli.googlecode.com/svn/doc/index.html).

# Structure #

In this project, the Graphical User Interface (GUI) is clearly separated of the image processing part.

GUI part in the folder _gui_ :

  * file [imageviewer.cpp](http://ofeli.googlecode.com/svn/doc/imageviewer_8cpp_source.html) file with the class ofeli::ImageViewer (dependant of the Qt4 framework)
  * file [pixmapwidget.cpp](http://ofeli.googlecode.com/svn/doc/pixmapwidget_8cpp_source.html) with the class ofeli::PixmapWidget (dependant of the Qt4 framework)
  * file [main.cpp](http://ofeli.googlecode.com/svn/doc/main_8cpp_source.html)

Image processing part in the folder _core_ :

  * file [activecontour.cpp](http://ofeli.googlecode.com/svn/doc/activecontour_8cpp_source.html) with the class ofeli::ActiveContour (dependant of the STL library)
  * file [ac\_withoutedges.cpp](http://ofeli.googlecode.com/svn/doc/ac__withoutedges_8cpp_source.html) with the class ofeli::ACwithoutEdges (dependant of the STL library)
  * file [ac\_withoutedges\_yuv.cpp](http://ofeli.googlecode.com/svn/doc/ac__withoutedges__yuv_8cpp_source.html) with the class ofeli::ACwithoutEdgesYUV (dependant of the STL library)
  * file [geodesic\_ac.cpp](http://ofeli.googlecode.com/svn/doc/geodesic__ac_8cpp_source.html) with the class ofeli::GeodesicAC (dependant of the STL library)
  * file [hausdorff\_distance.cpp](http://ofeli.googlecode.com/svn/doc/hausdorff__distance_8cpp_source.html) with the class ofeli::HausdorffDistance
  * file [filters.cpp](http://ofeli.googlecode.com/svn/doc/filters_8cpp_source.html) with the class ofeli::Filters (dependant of the Boost library)
  * file [filters\_rgb.cpp](http://ofeli.googlecode.com/svn/doc/filters__rgb_8cpp_source.html) with class ofeli::FiltersRGB (dependant of the Boost library)