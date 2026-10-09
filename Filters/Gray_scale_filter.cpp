#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

void grayscale_filter(Image& image){
    for (int i = 0; i < image.width; i++){
        for (int j = 0; j < image.height; j++){
            int red = image(i, j, 0);
            int green = image(i, j, 1);
            int blue = image(i, j, 2);
            int avg = (red + green + blue) / 3;
            image(i, j, 0) = avg;
            image(i, j, 1) = avg;
            image(i, j, 2) = avg;
        }
    }
}
