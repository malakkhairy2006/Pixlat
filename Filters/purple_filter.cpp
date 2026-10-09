#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

void purple_filter(Image& image){
    for(int i = 0 ; i < image.width ; ++i){
        for(int j = 0 ; j < image.height ; ++j){
            int r = image(i, j, 0);
            int g = image(i, j, 1);
            int b = image(i, j, 2);
            r = r + 40;
            g = (int)(g * 0.2);
            b = b + 100;
            if(r > 255){ r = 255; }
            if(b > 255){ b = 255; }
            image(i, j, 0) = r;
            image(i, j, 1) = g;
            image(i, j, 2) = b;
        }
    }
}
