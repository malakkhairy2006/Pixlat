#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

// GUI overloaded version taking crop rectangle (x, y, w, h)
void crop_filter(Image &image, int x, int y, int w, int h)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= image.width || y >= image.height) return;
    if (x + w > image.width) w = image.width - x;
    if (y + h > image.height) h = image.height - y;
    if (w <= 0 || h <= 0) return;

    Image cropped(w, h);

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            for (int k = 0; k < image.channels; k++)
            {
                cropped(i, j, k) = image(x + i, y + j, k);
            }
        }
    }

    image = cropped;
}

// Original console version with interactive cin
void crop_filter(Image &image)
{
    int x, y, w, h;

    cout << "Enter x: ";
    cin >> x;

    cout << "Enter y: ";
    cin >> y;

    cout << "Enter width: ";
    cin >> w;

    cout << "Enter height: ";
    cin >> h;

    crop_filter(image, x, y, w, h);
}
