# ofeli
Automatically exported from code.google.com/p/ofeli

<iframe width="425" height="344" src="https://www.youtube.com/embed/oxJ0WmNzooU" frameborder="0" allowfullscreen></iframe>

## Review
**Ofeli** (as an acronym for Open, Fast and Efficient Level set Implementation) demonstrates how to operate an image segmentation algorithm of Y. Shi and W. C. Karl |1|, using a discrete approach for the approximation of level-set based curve evolution (implicit active contours). This is a novel (2005) and fast algorithm without the need of solving partial differential equations (PDE) while preserving the advantages of level set methods, such as the automatic handling of topological changes. Considerable speedups (×100) have been demonstrated as compared to PDE-based narrow band level-set implementations.


## Features
**an image processing library written in C++ language with** :

  * an abstract implementation of the Y.Shi and W. C. Karl 's Fast-Two-Cycle (FTC) algorithm |1|.
  * inheritance-based implementations with specific speeds derived from Chan-Vese model |2| (region-based) and geodesic model |3| (edge-based) to create an API for a good reusability of the code.
  * preprocessing classes for color and grayscale images including filters (gaussian filter, median filter |4|, Perona-Malik anisotropic diffusion |5|, mathematical morphology operators) and noise generators (gaussian white noise, impulsional noise and speckle).
  * the Hausdorff distance and the modified Hausdorff distance between 2 lists of boundary points (to evaluate for example, the accuracy of the segmentation between a ground truth/manual segmentation and an automatic segmentation).

**cross-platform (Windows, Mac OS X and GNU/Linux) graphical user interface written in C++/Qt** :

  * a main window based on [Image Viewer](http://doc.qt.nokia.com/4.7/widgets-imageviewer.html) example of the Qt documentation to display the input image and each step of the active contour.
  * a settings window to control external speed models, optional internal speed, active contour initialization, preprocessing and image display of the main window with a refresh/interactive view of each modification and a mouse event handling for initialization.
  * an evaluation window to open 2 segmented images and to compute Hausdorff distance and modified Hausdorff distance with a mouse event handling to select the color of lists of points in the images.

**a developer's documentation** :

  * generated from the source code with [Doxygen](http://www.stack.nl/~dimitri/doxygen/index.html).
  * documentation's link : [http://pkuwwt.github.io/ofeli/doc/index.html](http://pkuwwt.github.io/ofeli/doc/index.html)

## License

This software is distributed under the [CeCILL license version 2](http://www.cecill.info/licences/Licence_CeCILL_V2-en.html) ([link to the french version here](http://www.cecill.info/licences/Licence_CeCILL_V2-fr.html)).


## Author

This application has been developed by Fabien Bessy, under the supervision of Julien Olivier and Romuald Boné, during an internship in the pattern recognition and image analysis research team (RFAI-LI) of the François Rabelais University's computer science laboratory, at Tours, as part of the MSc in medical imaging of the same university, in 2010. If you have any questions, comments, or suggestions, please contact me via my email address : fabien.bessy@gmail.com.

## Acknowledgments

  * J. Olivier, R. Boné, J-M. Girault, F. Amed, A. Lissy, C. Rouzière, L. Suta.
  * pattern recognition and image analysis research team, computer science laboratory, François Rabelais University.
  * students and professors of the MSc in medical imaging of François Rabelais University.

## Contribution

L. Suta, F. Bessy, C. Veja, M-F. Vaida - [Active Contours : Application to plant recognition](https://drive.google.com/file/d/1hyDgBYyrIPra6b60tRHwmC6nmxEcopvvFwpKOH3lLWNPODRIdNWpFEpwXrxY/view) - IEEE International Conference on Intelligent Computer Communication and Processing, Aug 2012.

## References

  * |1| Y. Shi, W. C. Karl - [A real-time algorithm for the approximation of level-set based curve evolution](https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_MGIwYmUwYzctYTRkMC00ODMwLWI3YmUtNTFjYThlMTBkOTIy&hl=en&authkey=CPT1xeYN) - IEEE Trans. Image Processing, vol. 17, no. 5, May 2008.
  * |2| T. F. Chan, L. A. Vese - [Active contours without edges](https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_NWY5ZGMyMmYtNzkwNi00NjI0LWE4ZGMtODllZTVmZWQ5NGRm&hl=en&authkey=CNfMkNEI) - IEEE Trans. Image Processing, vol. 10, no. 2, Feb 2001.
  * |3| V. Caselles, R. Kimmel, G. Sapiro - [Geodesic active contours](https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_ZWEzNzk2ZjgtNzlkMi00NDY0LTkzZjQtYWQ5N2EyNDA5NGE3&hl/edit?usp=sharing) - International Journal of Computer Vision, 22(1), 61–79 (1997).
  * |4| S. Perreault, P. Hébert - [Median filtering in constant time](https://docs.google.com/file/d/0Bzx5IoqehNE_Y3RsdnpVODFVcjA/edit?usp=sharing) - IEEE Trans. Image Processing, vol. 16, no. 9, Sep 2007.
  * |5| P. Perona, J. Malik - [Scale-space and edge detection using anistropic diffusion](https://docs.google.com/viewer?a=v&pid=explorer&chrome=true&srcid=0Bzx5IoqehNE_NmJmZWZkM2ItN2ZhZS00NjA4LTk3Y2UtNTNmYzkxYjFjNjU4&hl=en&authkey=CPDnxN8H) - IEEE Trans. Pattern Analysis and Machine Intelligence, vol. 12, no. 17, Jul 1990.
