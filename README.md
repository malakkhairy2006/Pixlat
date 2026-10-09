# PIXLat - Advanced Image Processor (Console & Windows GUI)

A comprehensive Object-Oriented Image Processing desktop application built in C++. Developed as part of the academic coursework for **CS213 (Object-Oriented Programming)** under the Faculty of Computers and Artificial Intelligence, Cairo University (FCAI-CU).

---

## 🌟 Overview & Interfaces

The project provides two independent user interfaces powered by the same underlying image-processing engine:
1. **Interactive Console Application (`main.cpp`)**: Terminal-based menu for sequential image filtering and saving.
2. **Modern Windows GUI (`GUI/GUI.cpp`)**: A full desktop window featuring side-by-side BEFORE and AFTER previews, sequential filter chaining, an intensity slider, Undo/Redo history, and intuitive control dialogs.

---

## 🖼️ Supported Image Formats

Powered by lightweight single-header image decoders and encoders in `Libraries/Image_Class.h`:
* **PNG** (`.png`)
* **JPEG / JPG** (`.jpg`, `.jpeg`)
* **Bitmap** (`.bmp`)
* **Targa** (`.tga`)

---

## 🚀 Windows GUI Documentation

### GUI Layout & Features

The Windows desktop interface uses a purple/lavender theme and the PIXLat brand.
* **BEFORE vs AFTER Side-by-Side Previews**:
  - **BEFORE Preview**: Always maintains the original loaded image as the reference benchmark.
  - **AFTER Preview**: Displays the live edited image with all sequential filters applied.
  - **Aspect-Ratio Preservation**: Automatically scales and centers images inside preview viewports without distorting dimensions.
* **Undo & Redo**: Full step-by-step history tracking. Undo previous filters or Redo them at will.
* **Clear Filters**: One-click restoration of the AFTER image back to the original image state.
* **Universal Live Intensity Slider (0% - 100%) for ALL Filters**:
  - Dragging the intensity slider provides instant, real-time live blending for **every filter** (Grayscale, Black & White, Darken, Lighten, Invert, Infrared, Sunlight, Old TV, Purple, Blur, Detect Edges, Add Frame).
  - Automatically preserves the clean image base state before the filter was selected, allowing continuous adjustment from 0% (original base) to 100% (full filter effect) without compounding or needing to click "Clear" or "Undo".
  - **Apply & Chain Button**: Locks in the current filtered state at any desired intensity, allowing unlimited sequential filter layering.
* **Filter Options Dialogs**:
  - **Flip**: Interactive Horizontal vs. Vertical mirror selection.
  - **Rotate**: 90° Clockwise, 180° Half-Turn, and 270° Clockwise.
  - **Add Frame**: Custom frame thickness and color picker (Red, Blue, White, Gray, etc.).
  - **Resize**: Dimensions scaling (Half scale, Double size, or custom dimensions).
  - **Crop**: Region selection with automatic boundary clamping.
* **Safe Save**: Automatically appends appropriate file extensions (`.png`, `.jpg`, `.bmp`, `.tga`) if omitted by the user.

### GUI Compilation (MinGW / MSYS2 on Windows)

#### Prerequisites
* Windows 7/8/10/11
* MinGW-w64 via MSYS2 (`ucrt64` or `mingw64` environment)
* Libraries required: `gdiplus`, `comctl32`, `comdlg32`, `gdi32`, `user32` (standard built-in Windows SDK libraries included with MinGW).

#
### Easiest way to start on Windows

1. Extract the entire project folder.
2. If `PIXLat.exe` is present, double-click it to launch the app.
3. If the executable is not included, double-click `Start_PIXLat.bat` to build `PIXLat.exe` with MSYS2 UCRT64 `g++` and `windres`, then launch it.
4. To build manually, double-click `BUILD_WINDOWS_EXE.bat`.

**Distribution note:** this source ZIP does not include a precompiled Windows executable. To let a user unzip and immediately find `PIXLat.exe`, the developer must build it once on Windows and include the resulting `PIXLat.exe` in the ZIP. The build scripts now create the EXE in the project root and embed the PIXLat icon, so the EXE does not depend on a separate icon file at runtime. Keep the full extracted folder together because the program uses the `Filters`, `Images`, and `Libraries` folders.

### Compilation Command
Open the MSYS2 UCRT64 terminal (or command prompt with `C:\msys64\ucrt64\bin\g++.exe` in PATH) and run:

```bash
windres PIXLat.rc -O coff -o PIXLat_resources.o
g++ -std=c++17 GUI/GUI.cpp PIXLat_resources.o -o PIXLat.exe -lgdiplus -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -mwindows
```

#### Running the GUI
```bash
./PIXLat.exe
```
or double-click `gui.exe` inside the `GUI/` directory in Windows File Explorer.

---

## 💻 Console Application Compilation & Setup

### Build Console Version
From the root project directory:
```bash
g++ -std=c++17 main.cpp -o PIXLatProcessor
```

### Run Console Version
```bash
./PIXLatProcessor
```
Follow the interactive terminal prompts to select an image from `Images/`, choose a filter (1-16), and export the edited file.

---

## 🎨 Available Filters

The project implements 16 image filters located in `Filters/`:

| Filter | Description | GUI Parameter / Control |
| :--- | :--- | :--- |
| **Grayscale** | Luminance averaging to grayscale | One click |
| **Black & White** | Monochrome binarization | Controlled via **Intensity Slider (0-100%)** |
| **Darken** | Reduces brightness exposure | Controlled via **Intensity Slider (0-100%)** |
| **Lighten** | Increases brightness exposure | Controlled via **Intensity Slider (0-100%)** |
| **Invert** | Photographic negative color inversion | One click |
| **Infrared** | False-color infrared spectrum rendering | One click |
| **Add Frame** | Solid outer border with custom thickness & color | Dialog with thickness and color choices |
| **Flip** | Matrix reflection along horizontal or vertical axis | Dialog (Horizontal / Vertical) |
| **Rotate** | Geometric rotation | Dialog (90°, 180°, 270°) |
| **Blur** | 5x5 neighborhood smoothing convolution | One click |
| **Crop** | Bounds-checked rectangular sub-region crop | Dialog (X, Y, Width, Height) |
| **Resize** | Nearest-neighbor dimension scaling | Dialog (Preset or custom width & height) |
| **Sunlight** | Warm solar tint enhancement | One click |
| **Old TV** | Retro CRT scanline effect | One click |
| **Purple** | Violet color cast enhancement | One click |
| **Detect Edges** | Gradient difference edge boundary extraction | One click |

---

## 📁 Project Directory Structure

```text
OOP_Projectt/
├── Filters/
│   ├── Add_frame_filter.cpp      # Border padding
│   ├── Blur_filter.cpp           # Smoothing convolution
│   ├── black_and_white_filter.cpp# Binarization thresholding
│   ├── crop_filter.cpp           # Crop subregion
│   ├── Darken-lighten_filter.cpp # Exposure adjustment
│   ├── detect_image_edges_filter.cpp # Sobel/difference edge detection
│   ├── Flip_filter.cpp           # Horizontal & Vertical mirror
│   ├── Gray_scale_filter.cpp     # Grayscale transformation
│   ├── Infrared_filter.cpp       # Thermal channel inversion
│   ├── Invert_filter.cpp         # RGB negation
│   ├── purple_filter.cpp         # Magenta tinting
│   ├── Resize_filter.cpp         # Dimension scaling
│   ├── rotate_filter.cpp         # 90/180/270 grid rotation
│   ├── sunlight_filter.cpp       # Warmth filter
│   └── TV_filter.cpp             # Interlaced scanline effect
├── GUI/
│   ├── GUI.cpp                   # Complete Win32 + GDI+ Desktop GUI
│   └── gui.exe                   # Compiled GUI executable (after build)
├── Images/                       # Asset images directory (sample input/output)
│   ├── mario.bmp
│   └── sample.bmp
├── Libraries/
│   ├── Image_Class.h             # Core Image matrix class
│   ├── stb_image.h               # Image loader backend
│   └── stb_image_write.h         # Image export backend
├── main.cpp                      # Console interface
└── README.md                     # Documentation & build instructions
```

---

## 👥 Academic Acknowledgments
* **Supervision:** Dr. Mohammad El-Ramly (FCAI-CU)
* **Course:** CS213 - Object-Oriented Programming

## PIXLat branding assets (Windows GUI)

The native Win32 GUI uses `PIXLat_logo.png` in the project root for the header and `PIXLat.ico` for the application/window icon. Keep these files in the project root and launch `GUI\gui.exe` with the project root as the working directory. The main action button is labeled **Apply Changes**.

