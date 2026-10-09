#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

// GUI overloaded version taking size and color code
void add_frame_filter(Image& image, int size, int color)
{
    int r = 0, g = 0, b = 0;
    if (color == 1) { // red
        r = 255; g = 0; b = 0;
    }
    else if (color == 2) { // green
        r = 0; g = 255; b = 0;
    }
    else if (color == 3) { // blue
        r = 0; g = 0; b = 255;
    }
    else if (color == 4) { // black
        r = 0; g = 0; b = 0;
    }
    else if (color == 5) { // white
        r = 255; g = 255; b = 255;
    }
    else if (color == 6) { // gray
        r = 124; g = 124; b = 124;
    }
    else if (color == 7) { // purple
        r = 145; g = 70; b = 210;
    }
    else if (color == 8) { // lavender
        r = 190; g = 150; b = 235;
    }
    else if (color == 9) { // pink
        r = 255; g = 105; b = 180;
    }
    else {
        return;
    }

    if (size <= 0) size = 10;
    if (size >= image.width / 2) size = image.width / 2;
    if (size >= image.height / 2) size = image.height / 2;

    for (int i = 0; i < image.width; ++i)
    {
        for (int j = 0; j < image.height; ++j)
        {
            if (i < size || i >= image.width - size ||
                j < size || j >= image.height - size)
            {
                image(i, j, 0) = r;
                image(i, j, 1) = g;
                image(i, j, 2) = b;
            }
        }
    }
}

// Original console version with interactive cin
void add_frame_filter(Image& image)
{
    int size;
    int color;
    cout << "Enter frame size:" << endl;
    cin >> size;
    cout << "choose frame color:" << endl;
    cout << "1=> red" << endl;
    cout << "2=> green" << endl;
    cout << "3=> blue" << endl;
    cout << "4=> black" << endl;
    cout << "5=> white" << endl;
    cout << "6=> gray" << endl;
    cin >> color;

    if (color < 1 || color > 6) {
        cout << "invalid color choice!" << endl;
        return;
    }

    add_frame_filter(image, size, color);
}
