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

#ifndef SETTINGS_WINDOW_HPP
#define SETTINGS_WINDOW_HPP

#include <QtWidgets>

namespace ofeli
{

class Filters;
class PixmapWidget;
class ScrollAreaWidget;
class MainWindow;

class SettingsWindow : public QDialog
{

    Q_OBJECT

public :

    SettingsWindow(QWidget* parent = nullptr);
    void init(const unsigned char* img0, unsigned int img0_width, unsigned int img0_height, bool isRgb0, const QImage& qimg0);
    // ces fonctions ne sont pas des slots car elles ne recoivent pas de signaux de widgets (ne sont pas connectes)
    // elles sont appeles apres fermeture de la fenêtre modale, fenêtre modale qui apparait grace a la methode exec()
    // la methode exec() renvoie un booléen en fonction de Ok ou Cancel
    // clic Ok
    void apply_settings();
    // clic Cancel
    void cancel_settings();
    void update_visu();

    static void get_color(unsigned int index, unsigned char& R, unsigned char& G, unsigned char& B);

    //! This function is called by the the #main_window close event in order to save persistent settings (window size, position, etc... ) of #settings_window.
    void save_settings() const;

private :

    // pour detecter le systeme automatiquement
    #ifdef Q_OS_WIN
    static const int operating_system = 1;
    #endif
    #ifdef Q_OS_MAC
    static const int operating_system = 2;
    #endif
    #ifdef Q_OS_LINUX
    static const int operating_system = 3;
    #endif

    //////////////////////////////////////////
    //   pour la fenêtre de configuration   //
    /////////////////////////////////////////

    // image dans un scrollarea a droite dans la fenêtre
    ofeli::PixmapWidget* imageLabel_settings;
    ofeli::ScrollAreaWidget* scrollArea_settings;

    // onglets a gauche
    QTabWidget* tabs;

    // Ok Cancel en bas
    QDialogButtonBox* dial_buttons;

    /////////////////////////////////////////
    //             onglets                 //
    /////////////////////////////////////////

    // widgets et variables liés à l'onglet algorithm :

    QSpinBox* Na_spin;
    QSpinBox* Ns_spin;
    QRadioButton* chanvese_radio;
    QSpinBox* lambda_out_spin;
    QSpinBox* lambda_in_spin;
    QRadioButton* geodesic_radio;
    QSpinBox* klength_gradient_spin;
    QGroupBox* yuv_groupbox;
    QSpinBox* Yspin;
    QSpinBox* Uspin;
    QSpinBox* Vspin;
    unsigned int model2;
    unsigned int kernel_gradient_length2;
    unsigned int alpha2;
    unsigned int beta2;
    unsigned int gamma2;

    QGroupBox* internalspeed_groupbox;
    QSpinBox* klength_spin;
    QDoubleSpinBox* std_spin;


    /////////////////////////////////////////

    // widgets et variables liés à l'onglet initialization :

    QPushButton* open_phi_button;
    QPushButton* save_phi_button;

    QImage img_phi;
    void open_phi();
    void phiInit2imgPhi();
    void imgPhi2phiInit();

    QRadioButton* rectangle_radio;
    QRadioButton* ellipse_radio;
    bool hasEllipse1;
    bool hasEllipse2;

    QSpinBox* width_shape_spin;
    QSlider* width_slider;
    double init_width1;
    double init_width2;
    QSpinBox* height_shape_spin;
    QSlider* height_slider;
    double init_height1;
    double init_height2;

    QSpinBox* abscissa_spin;
    QSlider* abscissa_slider;
    double center_x1;
    double center_x2;
    QSpinBox* ordinate_spin;
    QSlider* ordinate_slider;
    double center_y1;
    double center_y2;

    QPushButton* add_button;
    QPushButton* subtract_button;
    QPushButton* clean_button;

    static unsigned char otsu_method(const int histogram[], unsigned int img_size);

    /////////////////////////////////////////

    // widgets et variables liés à l'onglet preprocessing :

    QGroupBox* preprocessing();

    QVBoxLayout* noise_layout();

    QGroupBox* gaussian_noise_groupbox;
    bool hasGaussianNoise2;
    QDoubleSpinBox* std_noise_spin;
    double std_noise2;

    QGroupBox* salt_noise_groupbox;
    bool hasSaltNoise2;
    QDoubleSpinBox* proba_noise_spin;
    double proba_noise2;

    QGroupBox* speckle_noise_groupbox;
    bool hasSpeckleNoise2;
    QDoubleSpinBox* std_speckle_noise_spin;
    double std_speckle_noise2;

    QVBoxLayout* filter_layout();

    QGroupBox* median_groupbox;
    QSpinBox* klength_median_spin;
    QRadioButton* complex_radio1;
    QRadioButton* complex_radio2;
    bool hasMedianFilt2;
    bool hasO1algo2;
    unsigned int kernel_median_length2;

    QGroupBox* mean_groupbox;
    QSpinBox* klength_mean_spin;
    bool hasMeanFilt2;
    unsigned int kernel_mean_length2;

    QGroupBox* gaussian_groupbox;
    QSpinBox* klength_gaussian_spin;
    QDoubleSpinBox* std_filter_spin;
    bool hasGaussianFilt2;
    unsigned int kernel_gaussian_length2;
    double sigma2;

    QGroupBox* aniso_groupbox;
    QRadioButton* aniso1_radio;
    QRadioButton* aniso2_radio;
    QSpinBox* iteration_filter_spin;
    QDoubleSpinBox* lambda_spin;
    QDoubleSpinBox* kappa_spin;
    bool hasAnisoDiff2;
    unsigned int max_itera2;
    double lambda2;
    double kappa2;
    unsigned int aniso_option2;

    QGroupBox* open_groupbox;
    QSpinBox* klength_open_spin;
    bool hasOpenFilt2;
    unsigned int kernel_open_length2;

    QGroupBox* close_groupbox;
    QSpinBox* klength_close_spin;
    bool hasCloseFilt2;
    unsigned int kernel_close_length2;

    QGroupBox* tophat_groupbox;
    QRadioButton* whitetophat_radio;
    QRadioButton* blacktophat_radio;
    QSpinBox* klength_tophat_spin;
    bool hasTophatFilt2;
    bool isWhiteTophat2;
    unsigned int kernel_tophat_length2;

    QGroupBox* algo_groupbox;
    QRadioButton* complex1_morpho_radio;
    QRadioButton* complex2_morpho_radio;
    bool hasO1morpho2;

    QTabWidget* preprocess_tabs;
    QGroupBox* page3;
    bool hasPreprocess2;
    QLabel* time_filt;


    /////////////////////////////////////////

    // widgets et variables liés à l'onglet display :

    QSpinBox* scale_spin;
    QSlider* scale_slider;
    QCheckBox* histo_checkbox;
    bool hasHistoNormaliz2;


    QCheckBox* step_checkbox;
    QComboBox* outsidecolor_combobox;
    unsigned char Rout2;
    unsigned char Gout2;
    unsigned char Bout2;
    unsigned char Rout_selected2;
    unsigned char Gout_selected2;
    unsigned char Bout_selected2;
    QComboBox* insidecolor_combobox;
    unsigned char Rin2;
    unsigned char Gin2;
    unsigned char Bin2;
    unsigned char Rin_selected2;
    unsigned char Gin_selected2;
    unsigned char Bin_selected2;



    ofeli::Filters* filters2;
    // phi avant nettoyage des frontières
    signed char* phi_init2;
    // phi initial utilisé lors de l'execution du slot start() pour effectuer la segmentation
    signed char* phi_init1_clean;
    // phi initial déterminé par la fenêtre de configuration
    signed char* phi_init2_clean;
    // forme servant a etre ajouté ou retiré a phi
    signed char* shape;
    unsigned int* shape_points;
    unsigned int* Lout_shape1;
    unsigned int* Lin_shape1;
    unsigned int* Lout_2;
    unsigned int* Lin_2;
    QImage img;
    QImage image_filter;
    unsigned char* image_filter_uchar;
    QImage image_phi;
    unsigned char* image_phi_uchar;
    QImage image_shape;
    unsigned char* image_shape_uchar;

    QString last_directory_used;

    QPixmap pixmap_settings;

    void phi_add_shape();
    void phi_subtract_shape();
    void phi_visu(bool dark_color);
    bool isRedundantPointOfLout(signed char* phi, unsigned int x, unsigned int y);
    bool isRedundantPointOfLin(signed char* phi, unsigned int x, unsigned int y);
    void clean_boundaries(signed char* phi, signed char* phi_clean);

    /////////////////////////////////////////////////
    //        variables liés au slot open()        //
    /////////////////////////////////////////////////

    // chaîne de caractère du chemin+nom de l'image obtenu a partir d'une boite de dialogue ouverture de Qt
    QString fileName_phi;
    // buffer et informations des images
    // buffer 8 bits
    const unsigned char* img1;
    bool isRgb1;
    unsigned int image_format;
    // ligne
    unsigned int img_height;
    // colonne
    unsigned int img_width;
    // taille de l'image
    unsigned int img_size;
    unsigned int find_offset(unsigned int x, unsigned int y) const;

    static const unsigned int list_end = -9999999;

    bool eventFilter(QObject* object, QEvent* event);
    // position du curseur souris
    unsigned int positionX;
    unsigned int positionY;
    void show_phi_list_value();

    // pour la fenêtre de configuration des parametres
    void mouse_move_event_settings(QMouseEvent* event);
    void mouse_press_event_settings(QMouseEvent* event);

    bool hasContoursHidden;
    void img1_visu();
    bool hasShowImg1;

    void drag_enter_event_phi(QDragEnterEvent* event);
    void drag_move_event_phi(QDragMoveEvent* event);
    void drop_event_phi(QDropEvent* event);
    void drag_leave_event_phi(QDragLeaveEvent* event);

    QStringList nameFilters;

    struct Settings
    {
        Settings();

        unsigned int Na1;
        unsigned int Ns1;
        unsigned int model;
        unsigned int lambda_out1;
        unsigned int lambda_in1;
        unsigned int kernel_gradient_length1;
        unsigned int alpha1;
        unsigned int beta1;
        unsigned int gamma1;

        bool hasSmoothingCycle1;
        unsigned int kernel_curve1;
        double std_curve1;

        /////////////////////////////////////////

        const signed char* phi_init;
        unsigned int phi_width;
        unsigned int phi_height;

        /////////////////////////////////////////

        bool hasPreprocess;

        bool hasGaussianNoise;
        double std_noise;
        bool hasSaltNoise;
        double proba_noise;
        bool hasSpeckleNoise;
        double std_speckle_noise;

        bool hasMedianFilt;
        unsigned int kernel_median_length1;
        bool hasO1algo1;
        bool hasMeanFilt;
        unsigned int kernel_mean_length1;
        bool hasGaussianFilt;
        unsigned int kernel_gaussian_length1;
        double sigma;

        bool hasAnisoDiff;
        unsigned int aniso_option1;
        unsigned int max_itera1;
        double lambda1;
        double kappa1;

        bool hasOpenFilt;
        unsigned int kernel_open_length1;
        bool hasCloseFilt;
        unsigned int kernel_close_length1;
        bool hasTophatFilt;
        bool isWhiteTophat;
        unsigned int kernel_tophat_length1;

        bool hasO1morpho1;

        /////////////////////////////////////////

        bool hasHistoNormaliz;

        bool hasDisplayEach;

        int outside_combo;
        int inside_combo;

        unsigned char Rout1;
        unsigned char Gout1;
        unsigned char Bout1;
        unsigned char Rout_selected1;
        unsigned char Gout_selected1;
        unsigned char Bout_selected1;
        unsigned char Rin1;
        unsigned char Gin1;
        unsigned char Bin1;
        unsigned char Rin_selected1;
        unsigned char Gin_selected1;
        unsigned char Bin_selected1;
    };

    Settings mainwindow_settings;

public :

    const ofeli::SettingsWindow::Settings& get_settings() const { return mainwindow_settings; }

private slots :

    //void do_scale0(int value);
    void default_settings();

    // slot appelé à chaque changement d'onglet
    void tab_visu(int value);

    // slots appelés depuis l'onglet initialization
    void openFilenamePhi();
    void save_phi();
    void clean_phi_visu();
    void shape_visu(int value);
    void phi_visu(int value);
    void add_visu();
    void subtract_visu();
    void shape_visu();

    // slots appelés depuis l'onglet preprocessing
    // slot appelé aussi une fois dans l'onglet algorithm pour voir le gradient de l'image pour le contour geodesique
    void filtering_visu();

    // slots appelés depuis l'onglet Display
    void do_scale(int value);
    void change_display_size();
    void color_out();
    void color_in();

    void adjustVerticalScroll_settings(int min, int max);
    void adjustHorizontalScroll_settings(int min, int max);

    void wheel_zoom(int,ScrollAreaWidget*);

signals :

    void changed(const QMimeData* mimeData = 0);
};

inline unsigned int SettingsWindow::find_offset(unsigned int x, unsigned int y) const
{
    return x+y*img_width;
}

}

#endif // SETTINGS_WINDOW_HPP
