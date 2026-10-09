#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>

#include "Libraries/Image_Class.h"

// Filters Paths - All connected perfectly to the repository structures
#include "Filters/detect_image_edges_filter.cpp"
#include "Filters/Flip_filter.cpp"
#include "Filters/Invert_filter.cpp"
#include "Filters/black_and_white_filter.cpp"
#include "Filters/Gray_scale_filter.cpp"
#include "Filters/Add_frame_filter.cpp"
#include "Filters/Blur_filter.cpp"
#include "Filters/crop_filter.cpp"
#include "Filters/Darken-lighten_filter.cpp"
#include "Filters/Infrared_filter.cpp"
#include "Filters/purple_filter.cpp"
#include "Filters/Resize_filter.cpp"
#include "Filters/rotate_filter.cpp"
#include "Filters/sunlight_filter.cpp"
#include "Filters/TV_filter.cpp"
using namespace std;

int main()
{
    string imageName;
    string imagePath;
    int choice;
    Image image;

    // Choose image
    while (true)
    {
        cout << "Choose image: ";
        cin >> imageName;

        imagePath = "Images/" + imageName;

        try
        {
            image.loadNewImage(imagePath);
            break;
        }
        catch (...)
        {
            // Also try relative path directly
            try
            {
                image.loadNewImage(imageName);
                break;
            }
            catch (...)
            {
                cout << "Image not found!" << endl;
                cout << "Try again." << endl;
            }
        }
    }

    // Choose filter
    cout << "Choose filter:" << endl;

    cout << "1 => Grayscale" << endl;
    cout << "2 => Black and White" << endl;
    cout << "3 => Inverted" << endl;
    cout << "4 => Add Frame" << endl;
    cout << "5 => Flip" << endl;
    cout << "6 => Rotate" << endl;
    cout << "7 => Darken and Lighten Image" << endl;
    cout << "8 => Resize" << endl;
    cout << "9 => detect edges" << endl;
    cout << "10 => Crop Image" << endl;
    cout << "11 => Blur" << endl;
    cout << "12 => Sunlight" << endl;
    cout << "13 => old TV filter" << endl;
    cout << "14 => purple" << endl;
    cout << "15 => Infrared" << endl;

    cin >> choice;

    if (choice == 1)
    {
        grayscale_filter(image);
    }
    else if (choice == 2)
    {
        black_and_white_filter(image);
    }
    else if (choice == 3)
    {
        invert_filter(image);
    }
    else if (choice == 4)
    {
        add_frame_filter(image);
    }
    else if (choice == 5)
    {
        flip_filter(image);
    }
    else if (choice == 6)
    {
        rotate_filter(image);
    }
    else if (choice == 7)
    {
        darken_lighten_filter(image);
    }
    else if (choice == 8)
    {
        resize_filter(image);
    }
    else if (choice == 9)
    {
        detect_edges_filter(image);
    }
    else if (choice == 10)
    {
        crop_filter(image);
    }
    else if (choice == 11)
    {
        blur_filter(image);
    }
    else if (choice == 12)
    {
        sunlight_filter(image);
    }
    else if (choice == 13)
    {
        TV_filter(image);
    }
    else if (choice == 14)
    {
        purple_filter(image);
    }
    else if (choice == 15)
    {
        infrared_filter(image);
    }
    else
    {
        cout << "Invalid choice!" << endl;
        return 0;
    }

    // Save the new image
    string newImageName;
    string extension;
    int extensionChoice;

    cout << "Choose new image name: ";

    cin.ignore();
    getline(cin, newImageName);

    cout << "Choose extension:" << endl;
    cout << "1 => .JPG" << endl;
    cout << "2 => .JPEG" << endl;
    cout << "3 => .BMP" << endl;
    cout << "4 => .PNG" << endl;
    cout << "5 => .TGA" << endl;

    cin >> extensionChoice;

    if (extensionChoice == 1)
    {
        extension = ".jpg";
    }
    else if (extensionChoice == 2)
    {
        extension = ".jpeg";
    }
    else if (extensionChoice == 3)
    {
        extension = ".bmp";
    }
    else if (extensionChoice == 4)
    {
        extension = ".png";
    }
    else if (extensionChoice == 5)
    {
        extension = ".tga";
    }
    else
    {
        cout << "Invalid extension!" << endl;
        return 0;
    }

    // Save automatically inside Images folder
    string outputPath = "Images/" + newImageName + extension;
    image.saveImage(outputPath);

    cout << "Image saved successfully inside Images folder: " << outputPath << endl;
    return 0;
}
