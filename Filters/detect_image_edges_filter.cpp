#include <iostream>
#include <cmath>
#include <cstdlib>
#include "../Libraries/Image_Class.h" 
using namespace std; 

void grayscale_filter(Image &image); 

void detect_edges_filter(Image &image) 
{ 
    // turn pic to grey 
    grayscale_filter(image);

    // detect edges 
    int max_diff = 30; 
    for (int i = 0; i < image.width - 1; i++) { 
        for (int j = 0; j < image.height - 1; j++) { 
            int current = image(i, j, 0); 
            int right = image(i + 1, j, 0); 
            int down = image(i, j + 1, 0); 
            int diffR = abs(current - right); 
            int diffD = abs(current - down); 
            if (diffR > max_diff || diffD > max_diff) { 
                image(i, j, 0) = 0; 
                image(i, j, 1) = 0; 
                image(i, j, 2) = 0; 
            } 
            else { 
                image(i, j, 0) = 255; 
                image(i, j, 1) = 255; 
                image(i, j, 2) = 255; 
            } 
        } 
    } 
}
