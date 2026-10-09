#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

// GUI overloaded version taking angle directly (90, 180, 270)
void rotate_filter(Image &image, int angle)
{
    if (angle == 90)
    {
        Image rotated(image.height, image.width);
        for(int i=0 ; i<image.width ; i++)
        {
            for(int j=0 ; j<image.height ; j++)
            {
                for(int k=0 ; k<image.channels ; k++)
                {
                    rotated(image.height - 1 - j, i, k) = image(i, j, k);
                }
            }
        }
        image = rotated;
    }
    else if(angle == 180)
    {
        // For 180 rotation, dimensions remain width x height
        Image rotated(image.width, image.height);
        for(int i=0 ; i<image.width ; i++)
        {
            for(int j=0 ; j<image.height ; j++)
            {
                for(int k=0 ; k<image.channels ; k++)
                {
                    rotated(image.width - 1 - i, image.height - 1 - j, k) = image(i, j, k);
                }
            }
        }
        image = rotated;
    }
    else if(angle == 270)
    {
        Image rotated(image.height, image.width);
        for(int i=0 ; i<image.width ; i++)
        {
            for(int j=0 ; j<image.height ; j++)
            {
                for(int k=0 ; k<image.channels ; k++)
                {
                    rotated(j, image.width - 1 - i, k) = image(i, j, k);
                }
            }
        }
        image = rotated;
    }
}

// Original console version with interactive cin
void rotate_filter(Image &image)
{
    int angle;
    cout << "choose rotation angle (90, 180, 270): "<< endl;
    cin >> angle;
    rotate_filter(image, angle);
}
