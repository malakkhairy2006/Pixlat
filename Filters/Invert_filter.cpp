#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

void invert_filter(Image &image){
    for (int i = 0; i < image.width; ++i){
        for (int j = 0; j < image.height; ++j){
            for (int k = 0; k < image.channels; ++k){
                image(i, j, k) = 255 - image(i, j, k);
            }
        }
    }
}
