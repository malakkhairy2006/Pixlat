#include "../Libraries/Image_Class.h"

void blur_filter(Image &image)
{
    if (image.width < 5 || image.height < 5) return;
    Image temp = image;
    for (int i = 2; i < image.width - 2; ++i)
    {
        for (int j = 2; j < image.height - 2; ++j)
        {
            for (int k = 0; k < image.channels; ++k)
            {
                int sum = 0;
                for (int x = -2; x <= 2; ++x)
                {
                    for (int y = -2; y <= 2; ++y)
                    {
                        sum += temp(i + x, j + y, k);
                    }
                }
                image(i, j, k) = sum / 25;
            }
        }
    }
}
