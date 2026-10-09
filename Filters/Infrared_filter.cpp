#include "../Libraries/Image_Class.h"

void infrared_filter(Image &image)
{
    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            int red = image(i, j, 0);
            image(i, j, 0) = 255;
            image(i, j, 1) = 255 - red;
            image(i, j, 2) = 255 - red;
        }
    }
}
