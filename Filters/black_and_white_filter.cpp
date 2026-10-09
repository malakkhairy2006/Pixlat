#include <iostream>
#include <string>
#include "../Libraries/Image_Class.h"
using namespace std;

// GUI overloaded version taking intensity percentage (0 - 100%)
void black_and_white_filter(Image &image, double levelPercent) {
    double level = levelPercent / 100.0;
    if (level < 0.0) level = 0.0;
    if (level > 1.0) level = 1.0;

    for (int i = 0; i < image.width; i++)
    {
        for (int j = 0; j < image.height; j++)
        {
            int avg = 0;
            // calc avg
            for (int k = 0; k < image.channels; k++)
            {
                avg += image(i, j, k);
            }
            avg = avg / image.channels;

            // convert bright to white and dark to black
            int BW = (avg > 128) ? 255 : 0;

            // apply intensity level chosen of filter to image
            for (int k = 0; k < image.channels; k++)
            {
                image(i, j, k) = (unsigned char)((image(i, j, k) * (1.0 - level)) + (BW * level));
            }
        }
    }
}

// Original console version with interactive cin
void black_and_white_filter(Image &image){
    string option;
    double level = 100.0;

    cout << "coloured or B&W " << endl;
    cin >> option;

    if (option == "B&W" || option == "b&w" || option == "bw" || option == "BW")
    {
        cout << " choose level from 0 to 100% : " << endl;
        cin >> level;
        black_and_white_filter(image, level);
    }
}
