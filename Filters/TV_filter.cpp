#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

void TV_filter(Image & image){
    for(int i = 0 ; i < image.width ; ++i){
        for(int j = 0 ; j < image.height ; ++j){
            for(int k = 0 ; k < image.channels ; ++k){
                if(j % 2 == 0){
                    image(i, j, k) = image(0, 0, 0);
                }
            }
        }
    }
}
