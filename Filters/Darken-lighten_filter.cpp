#include <iostream>
#include <string>
#include "../Libraries/Image_Class.h"
using namespace std;

// GUI overloaded version taking dark/light mode and intensity level percentage (0 - 100%)
void darken_lighten_filter(Image &image, bool isDark, double levelPercent) {
    double level = levelPercent / 100.0;
    if (level < 0.0) level = 0.0;
    if (level > 1.0) level = 1.0;

    if (isDark) {
        for(int i = 0; i < image.width; ++i) {
            for(int j = 0; j < image.height; ++j) {
                for(int k = 0; k < image.channels; ++k) {
                    image(i, j, k) = (unsigned char)((1.0 - level) * image(i, j, k));
                }
            }
        }
    } else {
        for(int i = 0; i < image.width; ++i) {
            for(int j = 0; j < image.height; ++j) {
                for(int k = 0; k < image.channels; ++k) {
                    int val = (int)(image(i, j, k) + ((255 - image(i, j, k)) * level));
                    if (val > 255) val = 255;
                    image(i, j, k) = (unsigned char)val;
                }
            }
        }
    }
}

// Original console version with interactive cin
void darken_lighten_filter(Image &image) {
    string option;
    double level;
    cout << " Dark or Light :" << endl;
    cin >> option;
    cout << " choose the level from 0 to 100% : " << endl;
    cin >> level;

    bool isDark = (option == "dark" || option == "Dark" || option == "DARK");
    darken_lighten_filter(image, isDark, level);
}
