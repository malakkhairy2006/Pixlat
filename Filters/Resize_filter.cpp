#include <iostream>
#include "../Libraries/Image_Class.h"
using namespace std;

// GUI overloaded version taking target dimensions directly
void resize_filter(Image &image, int new_width, int new_height)
{
    if (new_width <= 0 || new_height <= 0) return;

    Image resized(new_width, new_height);

    for (int i = 0; i < new_width; ++i)
    {
        for (int j = 0; j < new_height; ++j)
        {
            int old_i = i * image.width / new_width;
            int old_j = j * image.height / new_height;
            for (int k = 0; k < image.channels; ++k)
            {
                resized(i, j, k) = image(old_i, old_j, k);
            }
        }
    }
    image = resized;
}

// Original console version with interactive cin
void resize_filter(Image &image)
{
    int choice;
    int new_width = image.width;
    int new_height = image.height;

    cout << "How do you want to resize the image?" << endl;
    cout << "1 => Enter new dimensions (Width x Height)" << endl;
    cout << "2 => Enter reduction or increase ratio (Percentage)" << endl;
    cin >> choice;

    if (choice == 1)
    {
        cout << "Enter new width: ";
        cin >> new_width;
        cout << "Enter new height: ";
        cin >> new_height;
    }
    else if (choice == 2)
    {
        double ratio;
        cout << "Enter scale ratio percentage: ";
        cin >> ratio;
        new_width = (int)(image.width * (ratio / 100.0));
        new_height = (int)(image.height * (ratio / 100.0));
    }
    else
    {
        cout << "Invalid choice! Keeping original size." << endl;
        return;
    }

    resize_filter(image, new_width, new_height);
}
