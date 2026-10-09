#include <iostream>
#include "../Libraries/Image_Class.h"
#include <string>
#include <algorithm>
using namespace std;
void merge_filter(Image& image)
{
    string imageName;
    cout << "Enter second image name: ";
    cin >> imageName;

    Image image2(imageName);

    int choice;

    cout << "Choose option:" << endl;
    cout << "1. Resize smaller image" << endl;
    cout << "2. Merge common area" << endl;
    cin >> choice;

    if (choice == 1)
    {
        int newwidth = max(image.width, image2.width);
        int newheight = max(image.height, image2.height);

        Image resized1(newwidth, newheight);
        Image resized2(newwidth, newheight);

        for (int i = 0; i < newwidth; i++)
        {
            for (int j = 0; j < newheight; j++)
            {
                int x1 = i * image.width / newwidth;
                int y1 = j * image.height / newheight;

                int x2 = i * image2.width / newwidth;
                int y2 = j * image2.height / newheight;

                for (int k = 0; k < image.channels; k++)
                {
                    resized1(i, j, k) = image(x1, y1, k);
                    resized2(i, j, k) = image2(x2, y2, k);
                }
            }
        }

        Image merged(newwidth, newheight);

        for (int i = 0; i < newwidth; i++)
        {
            for (int j = 0; j < newheight; j++)
            {
                for (int k = 0; k < image.channels; k++)
                {
                    merged(i, j, k) =
                        (resized1(i, j, k) + resized2(i, j, k)) / 2;
                }
            }
        }

        image = merged;
    }

    else if (choice == 2)
    {
        int newwidth = min(image.width, image2.width);
        int newheight = min(image.height, image2.height);

        Image merged(newwidth, newheight);

        for (int i = 0; i < newwidth; i++)
        {
            for (int j = 0; j < newheight; j++)
            {
                for (int k = 0; k < image.channels; k++)
                {
                    merged(i, j, k) =
                        (image(i, j, k) + image2(i, j, k)) / 2;
                }
            }
        }

        image = merged;
    }
}
