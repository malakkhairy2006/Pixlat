#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

void sunlight_filter(Image& image) {
    for (int i = 0; i < image.width; i++) {
        for (int j = 0; j < image.height; j++) {
            for (int k = 0; k < image.channels; k++) {
                int value = image(i, j, k);
                if (k == 0) {
                    value = value + 60;
                }
                else if (k == 1) {
                    value = value + 30;
                }
                else if (k == 2) {
                    value = value - 20;
                }
                if (value > 255)
                    value = 255;
                if (value < 0)
                    value = 0;
                image(i, j, k) = value;
            }
        }
    }
}
