/**
 * @file   GUI.cpp
 * @brief  Windows Desktop GUI for PIXLat Image Processor (FCAI CS213 OOP Project)
 *
 * Built with Win32 API and GDI+ for MinGW/MSYS2 on Windows.
 *
 * Features:
 *  - Universal Instant Real-Time Intensity Slider (0% - 100%) for ALL filters!
 *  - Moving the slider smoothly blends the active filter in real-time (0% = clean base, 100% = full effect).
 *  - User can adjust intensity instantly and switch filters freely without having to click "Clear" or "Undo".
 *  - "Apply Changes" button to commit the current intensity and sequentially layer more filters.
 *  - Side-by-side BEFORE (Original) and AFTER (Current) preview areas.
 *  - Aspect-ratio preserving preview scaling and centering.
 *  - Full Undo and Redo histories.
 *  - Clear Filters (reverts AFTER image back to original).
 *  - Options dialogs for Flip, Rotate, Crop, Resize, and Add Frame.
 *  - Reuses existing filters from Filters/ directory without code duplication.
 *
 * Compilation command (MinGW/MSYS2):
 *  g++ -std=c++17 GUI/GUI.cpp -o GUI/gui.exe -lgdiplus -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -mwindows
 */

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <gdiplus.h>

#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cwchar>
#include <cctype>
#include <stdexcept>

// Include the project core Image class
#include "../Libraries/Image_Class.h"

// Include all project filter implementations directly
#include "../Filters/detect_image_edges_filter.cpp"
#include "../Filters/Flip_filter.cpp"
#include "../Filters/Invert_filter.cpp"
#include "../Filters/black_and_white_filter.cpp"
#include "../Filters/Gray_scale_filter.cpp"
#include "../Filters/Add_frame_filter.cpp"
#include "../Filters/Blur_filter.cpp"
#include "../Filters/crop_filter.cpp"
#include "../Filters/Darken-lighten_filter.cpp"
#include "../Filters/Infrared_filter.cpp"
#include "../Filters/purple_filter.cpp"
#include "../Filters/Resize_filter.cpp"
#include "../Filters/rotate_filter.cpp"
#include "../Filters/sunlight_filter.cpp"
#include "../Filters/TV_filter.cpp"

// -------------------------------------------------------------
// Control IDs
// -------------------------------------------------------------
enum ControlIDs {
    ID_BTN_OPEN = 1001,
    ID_BTN_SAVE,
    ID_BTN_UNDO,
    ID_BTN_REDO,
    ID_BTN_CLEAR,
    ID_BTN_COMMIT, // Apply Changes

    // Filter Buttons
    ID_FILTER_GRAYSCALE,
    ID_FILTER_BW,
    ID_FILTER_DARKEN,
    ID_FILTER_LIGHTEN,
    ID_FILTER_INVERT,
    ID_FILTER_INFRARED,
    ID_FILTER_FRAME,
    ID_FILTER_FLIP,
    ID_FILTER_ROTATE,
    ID_FILTER_BLUR,
    ID_FILTER_CROP,
    ID_FILTER_PRECISE_CROP,
    ID_FILTER_RESIZE,
    ID_FILTER_SUNLIGHT,
    ID_FILTER_TV,
    ID_FILTER_PURPLE,
    ID_FILTER_EDGES,

    // Slider Controls
    ID_SLIDER_INTENSITY,
    ID_LABEL_INTENSITY,
    ID_LABEL_ACTIVE_FILTER
};

// -------------------------------------------------------------
// Filter Type Identification
// -------------------------------------------------------------
enum FilterType {
    FILTER_NONE = 0,
    FILTER_GRAYSCALE,
    FILTER_BW,
    FILTER_DARKEN,
    FILTER_LIGHTEN,
    FILTER_INVERT,
    FILTER_INFRARED,
    FILTER_FRAME,
    FILTER_BLUR,
    FILTER_SUNLIGHT,
    FILTER_TV,
    FILTER_PURPLE,
    FILTER_EDGES
};

// -------------------------------------------------------------
// Global Application State (Beginner-Friendly naming)
// -------------------------------------------------------------
Image originalImage;              ///< Original loaded image (BEFORE image, always preserved)
Image currentImage;               ///< Currently displayed image (AFTER image)
Image baseImageBeforeFilter;      ///< Base state right before active filter was selected
Image fullyFilteredImage;         ///< 100% filtered version used for instant linear alpha-blending
bool  hasImageLoaded = false;     ///< True if an image has been successfully loaded

std::vector<Image> undoStack;     ///< Stack of previous image states for Undo
std::vector<Image> redoStack;     ///< Stack of undone states for Redo

int currentIntensity = 100;       ///< Slider intensity percentage (0% to 100%)
FilterType activeFilter = FILTER_NONE; ///< Currently active filter receiving live slider updates

// Windows & Controls
HWND hMainWindow = NULL;
HWND hSliderIntensity = NULL;
HWND hLabelIntensity = NULL;
HWND hLabelActiveFilter = NULL;
HWND hStatusLabel = NULL;
HWND hBeforeLabel = NULL;
HWND hAfterLabel = NULL;

// Preview area layout rectangles
RECT rectBeforeArea = { 42, 88, 468, 414 };
RECT rectAfterArea  = { 522, 88, 948, 414 };

// Free-crop interaction state: drag directly over the AFTER preview.
bool freeCropMode = false;
bool freeCropDragging = false;
POINT freeCropStart = {0, 0};
POINT freeCropEnd = {0, 0};
std::vector<std::pair<HWND, RECT>> baseChildLayouts;
bool childLayoutsCaptured = false;
HWND layoutParent = NULL;
const int DESIGN_CLIENT_WIDTH = 1010;
const int DESIGN_CLIENT_HEIGHT = 660;

BOOL CALLBACK CaptureChildLayout(HWND child, LPARAM) {
    RECT r;
    GetWindowRect(child, &r);
    MapWindowPoints(NULL, layoutParent, reinterpret_cast<POINT*>(&r), 2);
    baseChildLayouts.push_back({child, r});
    return TRUE;
}

void CaptureBaseChildLayouts(HWND parent) {
    layoutParent = parent;
    baseChildLayouts.clear();
    EnumChildWindows(parent, CaptureChildLayout, 0);
    childLayoutsCaptured = true;
}

void ResizeResponsiveLayout(HWND parent, int clientW, int clientH) {
    if (!childLayoutsCaptured || clientW < 300 || clientH < 250) return;
    double sx = (double)clientW / DESIGN_CLIENT_WIDTH;
    double sy = (double)clientH / DESIGN_CLIENT_HEIGHT;
    for (const auto& item : baseChildLayouts) {
        if (!IsWindow(item.first)) continue;
        const RECT& r = item.second;
        int x = (int)(r.left * sx);
        int y = (int)(r.top * sy);
        int w = std::max(1, (int)((r.right - r.left) * sx));
        int h = std::max(1, (int)((r.bottom - r.top) * sy));
        SetWindowPos(item.first, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    RECT b = {42, 88, 468, 414};
    RECT a = {522, 88, 948, 414};
    rectBeforeArea = {(LONG)(b.left*sx), (LONG)(b.top*sy), (LONG)(b.right*sx), (LONG)(b.bottom*sy)};
    rectAfterArea = {(LONG)(a.left*sx), (LONG)(a.top*sy), (LONG)(a.right*sx), (LONG)(a.bottom*sy)};
}

// -------------------------------------------------------------
// String Conversion Utilities
// -------------------------------------------------------------
std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], sizeNeeded);
    return wstr;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], sizeNeeded, NULL, NULL);
    return str;
}

// -------------------------------------------------------------
// Universal Image Blend for Real-Time Intensity (0% - 100%)
// out = (1 - alpha) * base + alpha * filtered
// -------------------------------------------------------------
void BlendImages(Image& out, const Image& base, const Image& filtered, int intensityPercent) {
    double alpha = (double)intensityPercent / 100.0;
    if (alpha <= 0.0) {
        out = base;
        return;
    }
    if (alpha >= 1.0) {
        out = filtered;
        return;
    }

    out = base;
    for (int i = 0; i < out.width; ++i) {
        for (int j = 0; j < out.height; ++j) {
            for (int k = 0; k < out.channels; ++k) {
                int bVal = base(i, j, k);
                int fVal = filtered(i, j, k);
                out(i, j, k) = (unsigned char)((1.0 - alpha) * bVal + alpha * fVal);
            }
        }
    }
}

// -------------------------------------------------------------
// Helper: Draw an Image to a Windows Device Context (HDC)
// Fits image maintaining aspect ratio and centers it
// -------------------------------------------------------------
void RenderImageFit(HDC hdc, const Image& img, const RECT& targetRect) {
    int boxW = targetRect.right - targetRect.left;
    int boxH = targetRect.bottom - targetRect.top;

    // Fill background with dark slate
    HBRUSH bgBrush = CreateSolidBrush(RGB(36, 24, 52));
    FillRect(hdc, &targetRect, bgBrush);
    DeleteObject(bgBrush);

    // Box border
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(150, 105, 190));
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, targetRect.left, targetRect.top, targetRect.right, targetRect.bottom);
    SelectObject(hdc, oldPen);
    DeleteObject(borderPen);

    if (img.width <= 0 || img.height <= 0 || img.imageData == nullptr) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(190, 170, 210));
        RECT textRect = targetRect;
        DrawTextW(hdc, L"No image loaded\nClick 'Open Image' to start", -1, &textRect,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    double scaleX = (double)(boxW - 16) / img.width;
    double scaleY = (double)(boxH - 16) / img.height;
    double scale = (scaleX < scaleY) ? scaleX : scaleY;

    if (scale > 1.0 && img.width < boxW && img.height < boxH) {
        scale = 1.0;
    }

    int destW = (int)(img.width * scale);
    int destH = (int)(img.height * scale);
    int destX = targetRect.left + (boxW - destW) / 2;
    int destY = targetRect.top + (boxH - destH) / 2;

    int rowStride = (img.width * 3 + 3) & ~3;
    std::vector<unsigned char> bgrBuffer(rowStride * img.height);

    for (int y = 0; y < img.height; y++) {
        int dibRow = img.height - 1 - y;
        unsigned char* dstRow = &bgrBuffer[dibRow * rowStride];
        for (int x = 0; x < img.width; x++) {
            dstRow[x * 3 + 0] = img(x, y, 2); // Blue
            dstRow[x * 3 + 1] = img(x, y, 1); // Green
            dstRow[x * 3 + 2] = img(x, y, 0); // Red
        }
    }

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = img.width;
    bmi.bmiHeader.biHeight = img.height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    SetStretchBltMode(hdc, HALFTONE);
    StretchDIBits(hdc, destX, destY, destW, destH,
                  0, 0, img.width, img.height,
                  bgrBuffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
}

// -------------------------------------------------------------
// Helper: Record state before applying filter for Undo support
// -------------------------------------------------------------
void SaveStateForUndo() {
    if (!hasImageLoaded) return;
    undoStack.push_back(currentImage);
    redoStack.clear();
}

// -------------------------------------------------------------
// Commit current live filter and make it the new base
// -------------------------------------------------------------
void CommitCurrentFilter() {
    if (activeFilter != FILTER_NONE) {
        baseImageBeforeFilter = currentImage;
        activeFilter = FILTER_NONE;
        if (hLabelActiveFilter) {
            SetWindowTextW(hLabelActiveFilter, L"Active Filter: (Committed / None)");
        }
    }
}

// -------------------------------------------------------------
// Update Window Interface and Repaint Previews
// -------------------------------------------------------------
void RefreshGUI() {
    if (hasImageLoaded) {
        std::wstring beforeInfo = L"BEFORE (Original): " +
            std::to_wstring(originalImage.width) + L" x " + std::to_wstring(originalImage.height) + L" px";
        std::wstring afterInfo = L"AFTER (Current): " +
            std::to_wstring(currentImage.width) + L" x " + std::to_wstring(currentImage.height) + L" px";
        SetWindowTextW(hBeforeLabel, beforeInfo.c_str());
        SetWindowTextW(hAfterLabel, afterInfo.c_str());

        std::wstring status = L"Ready | Undo: " + std::to_wstring(undoStack.size()) +
            L" | Redo: " + std::to_wstring(redoStack.size()) +
            L" | Drag slider to instantly preview any intensity (0-100%)!";
        SetWindowTextW(hStatusLabel, status.c_str());
    } else {
        SetWindowTextW(hBeforeLabel, L"BEFORE (Original Image)");
        SetWindowTextW(hAfterLabel, L"AFTER (Current Edited Image)");
        SetWindowTextW(hStatusLabel, L"Welcome! Click 'Open Image' to get started.");
    }

    InvalidateRect(hMainWindow, NULL, FALSE);
}

// -------------------------------------------------------------
// Recalculate Live Filter Intensity Instantly
// -------------------------------------------------------------
void UpdateLiveFilterIntensity() {
    if (!hasImageLoaded || activeFilter == FILTER_NONE) return;

    // Instantly blend from the base image to the fully filtered version!
    BlendImages(currentImage, baseImageBeforeFilter, fullyFilteredImage, currentIntensity);
    InvalidateRect(hMainWindow, NULL, FALSE);
}

// -------------------------------------------------------------
// Setup a Filter for Instant Live Intensity Adjustment
// -------------------------------------------------------------
void SetupLiveFilter(FilterType type, const wchar_t* filterName, void (*applyFunc)(Image&)) {
    if (!hasImageLoaded) return;

    // If no filter was active, establish base image and save undo
    if (activeFilter == FILTER_NONE) {
        SaveStateForUndo();
        baseImageBeforeFilter = currentImage;
    }

    activeFilter = type;
    std::wstring label = L"Active Filter: [" + std::wstring(filterName) + L"]";
    SetWindowTextW(hLabelActiveFilter, label.c_str());

    // Generate the 100% filtered version once
    fullyFilteredImage = baseImageBeforeFilter;
    applyFunc(fullyFilteredImage);

    // Apply current intensity instantly without needing to clear!
    UpdateLiveFilterIntensity();
    RefreshGUI();
}


// -------------------------------------------------------------
// PIXLat themed custom dialogs (real buttons and editable options)
// -------------------------------------------------------------
struct PIXLatDialogField {
    std::wstring label;
    std::wstring value;
};

struct PIXLatDialogState {
    bool choiceDialog = false;
    bool accepted = false;
    int selectedChoice = -1;
    std::wstring title;
    std::wstring message;
    std::vector<PIXLatDialogField> fields;
    std::wstring comboLabel;
    std::vector<std::wstring> comboOptions;
    int selectedCombo = 0;
    std::vector<HWND> edits;
    HWND combo = NULL;
};

static PIXLatDialogState* gPIXLatDialogState = nullptr;
static const wchar_t* PIXLAT_DIALOG_CLASS = L"PIXLatCustomOptionsDialog";

LRESULT CALLBACK PIXLatDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    PIXLatDialogState* state = reinterpret_cast<PIXLatDialogState*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = static_cast<PIXLatDialogState*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }

    if (!state) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_CREATE: {
        int y = 18;
        if (!state->message.empty()) {
            CreateWindowW(L"STATIC", state->message.c_str(),
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                22, y, 390, 42, hwnd, NULL, NULL, NULL);
            y += 48;
        }

        if (state->choiceDialog) {
            int buttonY = y + 10;
            int buttonW = 360;
            for (size_t i = 0; i < state->comboOptions.size(); ++i) {
                CreateWindowW(L"BUTTON", state->comboOptions[i].c_str(),
                    WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                    22, buttonY, buttonW, 34, hwnd,
                    (HMENU)(INT_PTR)(9100 + i), NULL, NULL);
                buttonY += 42;
            }
            CreateWindowW(L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                300, buttonY + 2, 82, 30, hwnd,
                (HMENU)(INT_PTR)9002, NULL, NULL);
        } else {
            for (size_t i = 0; i < state->fields.size(); ++i) {
                CreateWindowW(L"STATIC", state->fields[i].label.c_str(),
                    WS_CHILD | WS_VISIBLE | SS_LEFT,
                    22, y, 150, 25, hwnd, NULL, NULL, NULL);
                HWND edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
                    state->fields[i].value.c_str(),
                    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
                    180, y - 3, 210, 28, hwnd,
                    (HMENU)(INT_PTR)(9200 + i), NULL, NULL);
                state->edits.push_back(edit);
                y += 42;
            }

            if (!state->comboOptions.empty()) {
                CreateWindowW(L"STATIC", state->comboLabel.c_str(),
                    WS_CHILD | WS_VISIBLE | SS_LEFT,
                    22, y, 150, 25, hwnd, NULL, NULL, NULL);
                state->combo = CreateWindowW(L"COMBOBOX", L"",
                    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                    180, y - 4, 210, 150, hwnd,
                    (HMENU)(INT_PTR)9300, NULL, NULL);
                for (const auto& option : state->comboOptions)
                    SendMessageW(state->combo, CB_ADDSTRING, 0, (LPARAM)option.c_str());
                SendMessageW(state->combo, CB_SETCURSEL, state->selectedCombo, 0);
                y += 46;
            }

            CreateWindowW(L"BUTTON", L"Apply",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
                218, y + 4, 82, 32, hwnd, (HMENU)(INT_PTR)9001, NULL, NULL);
            CreateWindowW(L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
                308, y + 4, 82, 32, hwnd, (HMENU)(INT_PTR)9002, NULL, NULL);
        }
        return 0;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(68, 35, 98));
        SetBkColor(hdc, RGB(248, 244, 255));
        static HBRUSH brush = CreateSolidBrush(RGB(248, 244, 255));
        return (LRESULT)brush;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* dis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (dis && dis->CtlType == ODT_BUTTON) {
            COLORREF fillColor = RGB(111, 55, 165);
            if (dis->itemState & ODS_DISABLED) fillColor = RGB(175, 160, 190);
            else if (dis->itemState & ODS_SELECTED) fillColor = RGB(78, 35, 120);
            else if (dis->itemState & ODS_HOTLIGHT) fillColor = RGB(145, 78, 205);

            HBRUSH brush = CreateSolidBrush(fillColor);
            FillRect(dis->hDC, &dis->rcItem, brush);
            DeleteObject(brush);
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(180, 145, 215));
            HPEN oldPen = (HPEN)SelectObject(dis->hDC, pen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
            RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top,
                dis->rcItem.right, dis->rcItem.bottom, 8, 8);
            SelectObject(dis->hDC, oldBrush);
            SelectObject(dis->hDC, oldPen);
            DeleteObject(pen);

            wchar_t buttonText[128] = L"";
            GetWindowTextW(dis->hwndItem, buttonText, 128);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, RGB(255, 255, 255));
            RECT textRect = dis->rcItem;
            InflateRect(&textRect, -4, -2);
            DrawTextW(dis->hDC, buttonText, -1, &textRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            return TRUE;
        }
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == 9002) {
            state->accepted = false;
            DestroyWindow(hwnd);
            return 0;
        }
        if (id == 9001 && !state->choiceDialog) {
            for (size_t i = 0; i < state->edits.size(); ++i) {
                wchar_t buffer[128] = L"";
                GetWindowTextW(state->edits[i], buffer, 128);
                state->fields[i].value = buffer;
            }
            if (state->combo)
                state->selectedCombo = (int)SendMessageW(state->combo, CB_GETCURSEL, 0, 0);
            state->accepted = true;
            DestroyWindow(hwnd);
            return 0;
        }
        if (state->choiceDialog && id >= 9100 && id < 9100 + (int)state->comboOptions.size()) {
            state->selectedChoice = id - 9100;
            state->accepted = true;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    }
    case WM_CLOSE:
        state->accepted = false;
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool RunPIXLatDialog(PIXLatDialogState& state, int width, int height) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = PIXLatDialogProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = PIXLAT_DIALOG_CLASS;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(248, 244, 255));
        RegisterClassW(&wc);
        registered = true;
    }

    HWND owner = hMainWindow;
    EnableWindow(owner, FALSE);
    HWND dialog = CreateWindowExW(WS_EX_DLGMODALFRAME, PIXLAT_DIALOG_CLASS,
        state.title.c_str(), WS_CAPTION | WS_SYSMENU | WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, owner, NULL,
        GetModuleHandleW(NULL), &state);

    if (!dialog) {
        EnableWindow(owner, TRUE);
        return false;
    }

    RECT wr;
    GetWindowRect(owner, &wr);
    SetWindowPos(dialog, NULL,
        wr.left + ((wr.right - wr.left) - width) / 2,
        wr.top + ((wr.bottom - wr.top) - height) / 2,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(dialog, SW_SHOW);
    UpdateWindow(dialog);

    MSG msg;
    while (IsWindow(dialog) && GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageW(dialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    EnableWindow(owner, TRUE);
    SetForegroundWindow(owner);
    return state.accepted;
}

int PIXLatShowChoice(const std::wstring& title, const std::wstring& message,
                     const std::vector<std::wstring>& choices) {
    PIXLatDialogState state;
    state.choiceDialog = true;
    state.title = title;
    state.message = message;
    state.comboOptions = choices;
    if (!RunPIXLatDialog(state, 430, 120 + (int)choices.size() * 42 + 40))
        return -1;
    return state.selectedChoice;
}

bool PIXLatShowForm(PIXLatDialogState& state) {
    int height = 100 + (int)state.fields.size() * 42;
    if (!state.comboOptions.empty()) height += 46;
    return RunPIXLatDialog(state, 430, height + 65);
}


// -------------------------------------------------------------
// Filter Triggers (Instantly Interactive with Live Slider!)
// -------------------------------------------------------------
void TriggerGrayscale()   { SetupLiveFilter(FILTER_GRAYSCALE, L"Grayscale", grayscale_filter); }
void TriggerInvert()      { SetupLiveFilter(FILTER_INVERT, L"Invert", invert_filter); }
void TriggerInfrared()    { SetupLiveFilter(FILTER_INFRARED, L"Infrared", infrared_filter); }
void TriggerBlur()        { SetupLiveFilter(FILTER_BLUR, L"Blur", blur_filter); }
void TriggerSunlight()    { SetupLiveFilter(FILTER_SUNLIGHT, L"Sunlight", sunlight_filter); }
void TriggerTV()          { SetupLiveFilter(FILTER_TV, L"Old TV", TV_filter); }
void TriggerPurple()      { SetupLiveFilter(FILTER_PURPLE, L"Purple", purple_filter); }
void TriggerDetectEdges() { SetupLiveFilter(FILTER_EDGES, L"Detect Edges", detect_edges_filter); }

void TriggerBW() {
    auto bwFunc = [](Image& img) { black_and_white_filter(img, 100.0); };
    SetupLiveFilter(FILTER_BW, L"Black & White", bwFunc);
}

void TriggerDarken() {
    auto darkenFunc = [](Image& img) { darken_lighten_filter(img, true, 100.0); };
    SetupLiveFilter(FILTER_DARKEN, L"Darken", darkenFunc);
}

void TriggerLighten() {
    auto lightenFunc = [](Image& img) { darken_lighten_filter(img, false, 100.0); };
    SetupLiveFilter(FILTER_LIGHTEN, L"Lighten", lightenFunc);
}

void TriggerAddFrame() {
    if (!hasImageLoaded) return;

    CommitCurrentFilter();

    PIXLatDialogState dialog;
    dialog.title = L"Add a Frame";
    dialog.message = L"Customize your frame before applying it.";
    dialog.fields = {
        {L"Thickness (pixels)", L"15"}
    };
    dialog.comboLabel = L"Frame color";
    dialog.comboOptions = {
        L"Purple", L"Lavender", L"Pink", L"Red", L"Green",
        L"Blue", L"Black", L"White", L"Gray"
    };
    dialog.selectedCombo = 0;

    if (!PIXLatShowForm(dialog)) return;

    int frameSize = 0;
    try {
        frameSize = std::stoi(dialog.fields[0].value);
    } catch (...) {
        MessageBoxW(hMainWindow, L"Enter a valid number for frame thickness.",
                    L"Invalid Thickness", MB_OK | MB_ICONWARNING);
        return;
    }

    int maxThickness = std::min(currentImage.width, currentImage.height) / 2;
    if (frameSize < 1 || frameSize > maxThickness) {
        std::wstring msg = L"Thickness must be between 1 and " +
            std::to_wstring(maxThickness) + L" pixels for this image.";
        MessageBoxW(hMainWindow, msg.c_str(), L"Invalid Thickness",
                    MB_OK | MB_ICONWARNING);
        return;
    }

    // Existing filter supports six colors; map the new palette to those colors.
    static const int colorCodes[] = { 7, 8, 9, 1, 2, 3, 4, 5, 6 };
    int colorCode = colorCodes[std::max(0, std::min(dialog.selectedCombo, 8))];

    SaveStateForUndo();
    baseImageBeforeFilter = currentImage;
    fullyFilteredImage = baseImageBeforeFilter;
    add_frame_filter(fullyFilteredImage, frameSize, colorCode);

    currentIntensity = 100;
    SendMessageW(hSliderIntensity, TBM_SETPOS, TRUE, currentIntensity);
    SetWindowTextW(hLabelIntensity, L"100%");
    currentImage = fullyFilteredImage;
    activeFilter = FILTER_NONE;
    SetWindowTextW(hLabelActiveFilter, L"Active Filter: (None)");
    baseImageBeforeFilter = currentImage;
    RefreshGUI();
}

// -------------------------------------------------------------
// Geometric Transformation Filters (Commit any active filter first)
// -------------------------------------------------------------
void ShowFlipDialog() {
    if (!hasImageLoaded) return;
    CommitCurrentFilter();

    int choice = PIXLatShowChoice(L"Flip Image",
        L"Choose how you want to mirror the image:",
        {L"Flip Horizontally", L"Flip Vertically"});
    if (choice < 0) return;

    SaveStateForUndo();
    flip_filter(currentImage, choice == 0 ? 1 : 2);
    baseImageBeforeFilter = currentImage;
    RefreshGUI();
}

void ShowRotateDialog() {
    if (!hasImageLoaded) return;
    CommitCurrentFilter();

    int choice = PIXLatShowChoice(L"Rotate Image",
        L"Select a rotation angle:",
        {L"90° Clockwise", L"180° Half Turn", L"270° Clockwise"});
    if (choice < 0) return;

    const int angles[] = {90, 180, 270};
    SaveStateForUndo();
    rotate_filter(currentImage, angles[choice]);
    baseImageBeforeFilter = currentImage;
    RefreshGUI();
}

void ShowResizeDialog() {
    if (!hasImageLoaded) return;
    CommitCurrentFilter();

    PIXLatDialogState dialog;
    dialog.title = L"Resize Image";
    dialog.message = L"Enter the new image dimensions in pixels.";
    dialog.fields = {
        {L"Width", std::to_wstring(currentImage.width)},
        {L"Height", std::to_wstring(currentImage.height)}
    };

    if (!PIXLatShowForm(dialog)) return;

    int newW = 0, newH = 0;
    try {
        newW = std::stoi(dialog.fields[0].value);
        newH = std::stoi(dialog.fields[1].value);
    } catch (...) {
        MessageBoxW(hMainWindow, L"Width and height must be whole numbers.",
                    L"Invalid Dimensions", MB_OK | MB_ICONWARNING);
        return;
    }

    if (newW < 1 || newH < 1 || newW > 12000 || newH > 12000) {
        MessageBoxW(hMainWindow, L"Enter dimensions between 1 and 12000 pixels.",
                    L"Invalid Dimensions", MB_OK | MB_ICONWARNING);
        return;
    }

    SaveStateForUndo();
    resize_filter(currentImage, newW, newH);
    baseImageBeforeFilter = currentImage;
    RefreshGUI();
}
void ShowPreciseCropDialog() {
    if (!hasImageLoaded) return;
    CommitCurrentFilter();

    PIXLatDialogState dialog;
    dialog.title = L"Precise Crop";
    dialog.message = L"Enter the crop rectangle in pixels. X and Y are measured from the top-left corner.";
    dialog.fields = {
        {L"X position", std::to_wstring(currentImage.width / 4)},
        {L"Y position", std::to_wstring(currentImage.height / 4)},
        {L"Width", std::to_wstring(std::max(1, currentImage.width / 2))},
        {L"Height", std::to_wstring(std::max(1, currentImage.height / 2))}
    };

    if (!PIXLatShowForm(dialog)) return;

    int x = 0, y = 0, w = 0, h = 0;
    try {
        x = std::stoi(dialog.fields[0].value);
        y = std::stoi(dialog.fields[1].value);
        w = std::stoi(dialog.fields[2].value);
        h = std::stoi(dialog.fields[3].value);
    } catch (...) {
        MessageBoxW(hMainWindow, L"All crop values must be whole numbers.",
                    L"Invalid Crop Values", MB_OK | MB_ICONWARNING);
        return;
    }

    if (x < 0 || y < 0 || w < 1 || h < 1 ||
        x >= currentImage.width || y >= currentImage.height ||
        x + w > currentImage.width || y + h > currentImage.height) {
        MessageBoxW(hMainWindow,
            L"Crop rectangle is outside the image. Choose X/Y within the image and a width/height that fits.",
            L"Invalid Crop Area", MB_OK | MB_ICONWARNING);
        return;
    }

    // Commit the crop as a geometric edit, independent of any previous live filter.
    SaveStateForUndo();
    crop_filter(currentImage, x, y, w, h);

    // A crop changes the image dimensions, so discard stale live-filter buffers.
    activeFilter = FILTER_NONE;
    currentIntensity = 100;
    if (hSliderIntensity) SendMessageW(hSliderIntensity, TBM_SETPOS, TRUE, 100);
    if (hLabelIntensity) SetWindowTextW(hLabelIntensity, L"100%");
    if (hLabelActiveFilter) SetWindowTextW(hLabelActiveFilter, L"Active Filter: (None)");
    baseImageBeforeFilter = currentImage;
    fullyFilteredImage = currentImage;
    RefreshGUI();
}

// Forward declarations for the combined Crop tool helpers.
RECT GetFittedImageRect(const Image& img, const RECT& targetRect);
void StartFreeCropMode();

// One Crop tool for both beginners and advanced users. Free Crop is the first
// (default) option; Precise Crop opens the pixel-coordinate form.
void ShowCropModeDialog() {
    if (!hasImageLoaded) {
        MessageBoxW(hMainWindow, L"Open an image first, then choose Crop.",
                    L"No Image Loaded", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int choice = PIXLatShowChoice(L"Crop Image",
        L"Choose how you want to crop. Free Crop is easiest; Precise Crop lets you enter exact pixels.",
        {L"Free Crop (drag with mouse)", L"Precise Crop (enter pixels)"});
    if (choice == 0) StartFreeCropMode();
    else if (choice == 1) ShowPreciseCropDialog();
}

// Show live source-image pixel coordinates and dimensions while dragging.
void UpdateFreeCropPixelStatus() {
    if (!hasImageLoaded || !freeCropDragging || !hStatusLabel) return;
    RECT imageRect = GetFittedImageRect(currentImage, rectAfterArea);
    // POINT coordinates are LONG on Windows; convert them to int before
    // mixing them with the image rectangle values in std::min/std::max.
    const int startX = static_cast<int>(freeCropStart.x);
    const int endX   = static_cast<int>(freeCropEnd.x);
    const int startY = static_cast<int>(freeCropStart.y);
    const int endY   = static_cast<int>(freeCropEnd.y);
    const int rectLeft   = static_cast<int>(imageRect.left);
    const int rectRight  = static_cast<int>(imageRect.right);
    const int rectTop    = static_cast<int>(imageRect.top);
    const int rectBottom = static_cast<int>(imageRect.bottom);

    int left   = std::max(std::min(startX, endX), rectLeft);
    int right  = std::min(std::max(startX, endX), rectRight);
    int top    = std::max(std::min(startY, endY), rectTop);
    int bottom = std::min(std::max(startY, endY), rectBottom);
    int viewW = std::max(1, (int)(imageRect.right - imageRect.left));
    int viewH = std::max(1, (int)(imageRect.bottom - imageRect.top));
    int x = std::max(0, (int)((left - imageRect.left) * (double)currentImage.width / viewW));
    int y = std::max(0, (int)((top - imageRect.top) * (double)currentImage.height / viewH));
    int w = std::max(0, (int)((right - left) * (double)currentImage.width / viewW));
    int h = std::max(0, (int)((bottom - top) * (double)currentImage.height / viewH));
    std::wstring status = L"FREE CROP  |  X: " + std::to_wstring(x) + L" px  Y: " +
        std::to_wstring(y) + L" px  |  Size: " + std::to_wstring(w) + L" × " +
        std::to_wstring(h) + L" px  |  Release mouse to apply, Esc to cancel";
    SetWindowTextW(hStatusLabel, status.c_str());
}

// Calculate the exact image rectangle used by RenderImageFit, so mouse
// coordinates can be translated to source-image pixels accurately.
RECT GetFittedImageRect(const Image& img, const RECT& targetRect) {
    RECT r = targetRect;
    if (img.width <= 0 || img.height <= 0) return r;
    int boxW = targetRect.right - targetRect.left;
    int boxH = targetRect.bottom - targetRect.top;
    double scaleX = (double)(boxW - 16) / img.width;
    double scaleY = (double)(boxH - 16) / img.height;
    double scale = (scaleX < scaleY) ? scaleX : scaleY;
    if (scale > 1.0 && img.width < boxW && img.height < boxH) scale = 1.0;
    int destW = (int)(img.width * scale);
    int destH = (int)(img.height * scale);
    r.left = targetRect.left + (boxW - destW) / 2;
    r.top = targetRect.top + (boxH - destH) / 2;
    r.right = r.left + destW;
    r.bottom = r.top + destH;
    return r;
}

bool PointInsideRect(const POINT& p, const RECT& r) {
    return p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}

void ApplyFreeCropFromSelection() {
    if (!hasImageLoaded) return;
    RECT imageRect = GetFittedImageRect(currentImage, rectAfterArea);
    int left = std::min(freeCropStart.x, freeCropEnd.x);
    int right = std::max(freeCropStart.x, freeCropEnd.x);
    int top = std::min(freeCropStart.y, freeCropEnd.y);
    int bottom = std::max(freeCropStart.y, freeCropEnd.y);
    left = std::max(left, (int)imageRect.left); right = std::min(right, (int)imageRect.right);
    top = std::max(top, (int)imageRect.top); bottom = std::min(bottom, (int)imageRect.bottom);
    if (right - left < 3 || bottom - top < 3) {
        MessageBoxW(hMainWindow, L"Drag a larger rectangle over the AFTER image to crop it.",
                    L"Crop Area Too Small", MB_OK | MB_ICONINFORMATION);
        return;
    }

    double sx = (double)currentImage.width / (imageRect.right - imageRect.left);
    double sy = (double)currentImage.height / (imageRect.bottom - imageRect.top);
    int x = (int)((left - imageRect.left) * sx);
    int y = (int)((top - imageRect.top) * sy);
    int w = (int)((right - imageRect.left) * sx) - x;
    int h = (int)((bottom - imageRect.top) * sy) - y;
    x = std::max(0, std::min(x, currentImage.width - 1));
    y = std::max(0, std::min(y, currentImage.height - 1));
    w = std::max(1, std::min(w, currentImage.width - x));
    h = std::max(1, std::min(h, currentImage.height - y));

    SaveStateForUndo();
    crop_filter(currentImage, x, y, w, h);
    activeFilter = FILTER_NONE;
    currentIntensity = 100;
    if (hSliderIntensity) SendMessageW(hSliderIntensity, TBM_SETPOS, TRUE, 100);
    if (hLabelIntensity) SetWindowTextW(hLabelIntensity, L"100%");
    if (hLabelActiveFilter) SetWindowTextW(hLabelActiveFilter, L"Active Filter: (None)");
    baseImageBeforeFilter = currentImage;
    fullyFilteredImage = currentImage;
    freeCropMode = false;
    freeCropDragging = false;
    SetWindowTextW(hStatusLabel, L"Crop applied. Use Undo to restore the previous image.");
    RefreshGUI();
}

void StartFreeCropMode() {
    if (!hasImageLoaded) {
        MessageBoxW(hMainWindow, L"Open an image first, then click Free Crop.",
                    L"No Image Loaded", MB_OK | MB_ICONINFORMATION);
        return;
    }
    CommitCurrentFilter();
    freeCropMode = true;
    freeCropDragging = false;
    SetFocus(hMainWindow);
    SetWindowTextW(hStatusLabel, L"FREE CROP: Drag a rectangle over the AFTER image. Press Esc to cancel.");
    SetCursor(LoadCursor(NULL, IDC_CROSS));
    InvalidateRect(hMainWindow, NULL, FALSE);
}

// -------------------------------------------------------------
// GDI+ helpers for real Windows image-format support
// -------------------------------------------------------------
int GetEncoderClsid(const WCHAR* mimeType, CLSID* pClsid) {
    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    std::vector<BYTE> buffer(size);
    auto* codecs = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    if (Gdiplus::GetImageEncoders(num, size, codecs) != Gdiplus::Ok) return -1;
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(codecs[i].MimeType, mimeType) == 0) {
            *pClsid = codecs[i].Clsid;
            return static_cast<int>(i);
        }
    }
    return -1;
}

Image LoadImageWithGDIPlus(const WCHAR* path) {
    Gdiplus::Bitmap bitmap(path);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        throw std::runtime_error("Windows could not decode this image. Try a valid JPG, PNG, or BMP file.");
    }
    UINT w = bitmap.GetWidth();
    UINT h = bitmap.GetHeight();
    if (w == 0 || h == 0 || w > 20000 || h > 20000) {
        throw std::runtime_error("The image dimensions are invalid or too large.");
    }
    Image loaded(static_cast<int>(w), static_cast<int>(h));
    loaded.channels = 3;
    for (UINT y = 0; y < h; ++y) {
        for (UINT x = 0; x < w; ++x) {
            Gdiplus::Color color;
            if (bitmap.GetPixel(static_cast<INT>(x), static_cast<INT>(y), &color) != Gdiplus::Ok) {
                throw std::runtime_error("Could not read a pixel from the selected image.");
            }
            loaded(static_cast<int>(x), static_cast<int>(y), 0) = color.GetR();
            loaded(static_cast<int>(x), static_cast<int>(y), 1) = color.GetG();
            loaded(static_cast<int>(x), static_cast<int>(y), 2) = color.GetB();
        }
    }
    return loaded;
}

void SaveImageWithGDIPlus(const Image& img, const WCHAR* path, const WCHAR* mimeType) {
    if (img.width <= 0 || img.height <= 0 || img.imageData == nullptr) {
        throw std::runtime_error("There is no image data to save.");
    }
    Gdiplus::Bitmap bitmap(img.width, img.height, PixelFormat24bppRGB);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        throw std::runtime_error("Could not create the output image.");
    }
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Gdiplus::Color color(255, img(x, y, 0), img(x, y, 1), img(x, y, 2));
            if (bitmap.SetPixel(x, y, color) != Gdiplus::Ok) {
                throw std::runtime_error("Could not write pixels to the output image.");
            }
        }
    }
    CLSID encoder;
    if (GetEncoderClsid(mimeType, &encoder) < 0) {
        throw std::runtime_error("The selected image format is not available on this Windows installation.");
    }
    if (bitmap.Save(path, &encoder, nullptr) != Gdiplus::Ok) {
        throw std::runtime_error("Windows could not save the image to that location.");
    }
}

// -------------------------------------------------------------
// File Dialogs: Open and Save Image
// -------------------------------------------------------------
void HandleOpenImage() {
    WCHAR filename[MAX_PATH] = L"";

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFilter = L"All Supported Images (*.png;*.jpg;*.jpeg;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0"
                      L"PNG Image (*.png)\0*.png\0"
                      L"JPEG Image (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0"
                      L"Bitmap Image (*.bmp)\0*.bmp\0"
                      L"TGA Image (*.tga)\0*.tga\0"
                      L"All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrInitialDir = L"Images";

    if (GetOpenFileNameW(&ofn)) {
        // Use Windows GDI+ to decode JPG/PNG/BMP from the original wide path.
        try {
            Image loaded = LoadImageWithGDIPlus(filename);
            originalImage = loaded;
            currentImage = loaded;
            baseImageBeforeFilter = loaded;
            hasImageLoaded = true;

            undoStack.clear();
            redoStack.clear();
            activeFilter = FILTER_NONE;
            SetWindowTextW(hLabelActiveFilter, L"Active Filter: (None)");

            RefreshGUI();
            MessageBoxW(hMainWindow, L"Image loaded successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
        }
        catch (const std::exception& e) {
            std::wstring err = L"Could not open image: " + StringToWString(e.what());
            MessageBoxW(hMainWindow, err.c_str(), L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

void HandleSaveImage() {
    if (!hasImageLoaded) {
        MessageBoxW(hMainWindow, L"Please open an image first before saving.", L"Notice", MB_OK | MB_ICONWARNING);
        return;
    }

    WCHAR filename[MAX_PATH] = L"edited_image.png";

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFilter = L"PNG Image (*.png)\0*.png\0"
                      L"JPEG Image (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0"
                      L"Bitmap Image (*.bmp)\0*.bmp\0"
                      L"TGA Image (*.tga)\0*.tga\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"png";
    ofn.lpstrInitialDir = L"Images";

    if (GetSaveFileNameW(&ofn)) {
        std::string savePath = WStringToString(filename);

        std::string lowerPath = savePath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        // Check only the final extension (not a folder name containing ".png").
        size_t slashPos = lowerPath.find_last_of("\\/");
        size_t dotPos = lowerPath.find_last_of('.');
        bool hasDotInFilename = dotPos != std::string::npos &&
            (slashPos == std::string::npos || dotPos > slashPos);
        std::string extension = hasDotInFilename ? lowerPath.substr(dotPos) : "";

        std::string selectedExtension = ".png";
        if (ofn.nFilterIndex == 2) selectedExtension = ".jpg";
        else if (ofn.nFilterIndex == 3) selectedExtension = ".bmp";
        else if (ofn.nFilterIndex == 4) selectedExtension = ".tga";

        bool extensionMatchesFilter =
            (ofn.nFilterIndex == 1 && extension == ".png") ||
            (ofn.nFilterIndex == 2 && (extension == ".jpg" || extension == ".jpeg")) ||
            (ofn.nFilterIndex == 3 && extension == ".bmp") ||
            (ofn.nFilterIndex == 4 && extension == ".tga");

        // Keep the selected format and filename extension in sync.
        if (!extensionMatchesFilter) {
            if (hasDotInFilename) savePath.resize(dotPos);
            savePath += selectedExtension;
            extension = selectedExtension;
        }

        try {
            std::wstring outputPath = StringToWString(savePath);
            if (extension == ".png") {
                SaveImageWithGDIPlus(currentImage, outputPath.c_str(), L"image/png");
            } else if (extension == ".jpg" || extension == ".jpeg") {
                SaveImageWithGDIPlus(currentImage, outputPath.c_str(), L"image/jpeg");
            } else if (extension == ".bmp") {
                SaveImageWithGDIPlus(currentImage, outputPath.c_str(), L"image/bmp");
            } else if (extension == ".tga") {
                currentImage.saveImage(savePath);
            } else {
                throw std::runtime_error("Please choose PNG, JPG, BMP, or TGA.");
            }
            std::wstring msg = L"Image saved successfully to:\n" + outputPath;
            MessageBoxW(hMainWindow, msg.c_str(), L"Saved Successfully", MB_OK | MB_ICONINFORMATION);
        }
        catch (const std::exception& e) {
            std::wstring err = L"Could not save image: " + StringToWString(e.what());
            MessageBoxW(hMainWindow, err.c_str(), L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

// -------------------------------------------------------------
// Undo, Redo, Clear Filters
// -------------------------------------------------------------
void HandleUndo() {
    if (undoStack.empty()) {
        MessageBoxW(hMainWindow, L"No more undo steps available.", L"Undo", MB_OK | MB_ICONINFORMATION);
        return;
    }
    CommitCurrentFilter();
    redoStack.push_back(currentImage);
    currentImage = undoStack.back();
    baseImageBeforeFilter = currentImage;
    undoStack.pop_back();
    RefreshGUI();
}

void HandleRedo() {
    if (redoStack.empty()) {
        MessageBoxW(hMainWindow, L"No more redo steps available.", L"Redo", MB_OK | MB_ICONINFORMATION);
        return;
    }
    CommitCurrentFilter();
    undoStack.push_back(currentImage);
    currentImage = redoStack.back();
    baseImageBeforeFilter = currentImage;
    redoStack.pop_back();
    RefreshGUI();
}

void HandleClearFilters() {
    if (!hasImageLoaded) return;
    SaveStateForUndo();
    CommitCurrentFilter();
    currentImage = originalImage; // Restore AFTER image to original, keep BEFORE original unchanged
    baseImageBeforeFilter = originalImage;
    RefreshGUI();
}

void HandleCommitFilter() {
    if (!hasImageLoaded || activeFilter == FILTER_NONE) return;
    CommitCurrentFilter();
    MessageBoxW(hMainWindow, L"Filter applied and locked in! You can now chain another filter on top.",
                L"Filter Committed", MB_OK | MB_ICONINFORMATION);
    RefreshGUI();
}

// -------------------------------------------------------------
// Window Procedure (Message Handling)
// -------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(74, 35, 112));
            SetBkMode(hdc, TRANSPARENT);
            static HBRUSH staticBrush = CreateSolidBrush(RGB(247, 243, 255));
            return (LRESULT)staticBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(54, 30, 75));
            SetBkColor(hdc, RGB(255, 255, 255));
            static HBRUSH editBrush = CreateSolidBrush(RGB(255, 255, 255));
            return (LRESULT)editBrush;
        }
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(74, 35, 112));
            SetBkColor(hdc, RGB(232, 216, 250));
            static HBRUSH buttonBrush = CreateSolidBrush(RGB(232, 216, 250));
            return (LRESULT)buttonBrush;
        }
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
            InitCommonControlsEx(&icex);

            // Title labels for BEFORE and AFTER
            hBeforeLabel = CreateWindowW(L"STATIC", L"BEFORE  |  Original Image",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                30, 58, 450, 22, hWnd, NULL, NULL, NULL);

            hAfterLabel = CreateWindowW(L"STATIC", L"AFTER  |  Edited Preview",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                510, 58, 450, 22, hWnd, NULL, NULL, NULL);

            HFONT uiFont = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            SendMessageW(hBeforeLabel, WM_SETFONT, (WPARAM)uiFont, TRUE);
            SendMessageW(hAfterLabel, WM_SETFONT, (WPARAM)uiFont, TRUE);

            // Action Toolbar Buttons (row 1, y = 435)
            int btnY = 435;
            int btnH = 34;
            CreateWindowW(L"BUTTON", L"Open Image", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                30, btnY, 100, btnH, hWnd, (HMENU)ID_BTN_OPEN, NULL, NULL);

            CreateWindowW(L"BUTTON", L"Save Image", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                135, btnY, 100, btnH, hWnd, (HMENU)ID_BTN_SAVE, NULL, NULL);

            CreateWindowW(L"BUTTON", L"Undo", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                240, btnY, 80, btnH, hWnd, (HMENU)ID_BTN_UNDO, NULL, NULL);

            CreateWindowW(L"BUTTON", L"Redo", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                325, btnY, 80, btnH, hWnd, (HMENU)ID_BTN_REDO, NULL, NULL);

            CreateWindowW(L"BUTTON", L"Clear Filters", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                410, btnY, 105, btnH, hWnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);

            CreateWindowW(L"BUTTON", L"Apply Changes", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                520, btnY, 110, btnH, hWnd, (HMENU)ID_BTN_COMMIT, NULL, NULL);

            // Active Filter Name Label
            hLabelActiveFilter = CreateWindowW(L"STATIC", L"Active Filter: (None)",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                645, btnY - 2, 230, 16, hWnd, (HMENU)ID_LABEL_ACTIVE_FILTER, NULL, NULL);

            // Intensity Slider & Label
            hSliderIntensity = CreateWindowW(TRACKBAR_CLASSW, L"",
                WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ,
                640, btnY + 14, 250, 26, hWnd, (HMENU)ID_SLIDER_INTENSITY, NULL, NULL);
            SendMessageW(hSliderIntensity, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
            SendMessageW(hSliderIntensity, TBM_SETPOS, TRUE, 100);

            hLabelIntensity = CreateWindowW(L"STATIC", L"100%",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                895, btnY + 16, 60, 20, hWnd, (HMENU)ID_LABEL_INTENSITY, NULL, NULL);

            // Filter Buttons - Section Title (y = 480)
            CreateWindowW(L"STATIC", L"✦  PIXLat FILTER STUDIO  ✦",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                30, 480, 930, 20, hWnd, NULL, NULL, NULL);

            // Filter Buttons Rows (y = 505 and y = 545)
            int fy1 = 505;
            int fW = 100;
            int fH = 30;
            int gap = 8;
            int fx = 30;

            // Row 1
            CreateWindowW(L"BUTTON", L"Grayscale", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_GRAYSCALE, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Black & White", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_BW, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Darken", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_DARKEN, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Lighten", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_LIGHTEN, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Invert", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_INVERT, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Infrared", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_INFRARED, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Add Frame", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_FRAME, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Flip", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_FLIP, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Rotate", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy1, fW, fH, hWnd, (HMENU)ID_FILTER_ROTATE, NULL, NULL);

            // Row 2
            int fy2 = 545;
            fx = 30;
            CreateWindowW(L"BUTTON", L"Blur", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW, fH, hWnd, (HMENU)ID_FILTER_BLUR, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Crop", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW + 12, fH, hWnd, (HMENU)ID_FILTER_CROP, NULL, NULL);
            fx += fW + 12 + gap;
            CreateWindowW(L"BUTTON", L"Resize", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW, fH, hWnd, (HMENU)ID_FILTER_RESIZE, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Sunlight", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW, fH, hWnd, (HMENU)ID_FILTER_SUNLIGHT, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Old TV", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW, fH, hWnd, (HMENU)ID_FILTER_TV, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Purple", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW, fH, hWnd, (HMENU)ID_FILTER_PURPLE, NULL, NULL);
            fx += fW + gap;
            CreateWindowW(L"BUTTON", L"Detect Edges", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                fx, fy2, fW, fH, hWnd, (HMENU)ID_FILTER_EDGES, NULL, NULL);

            // Bottom instruction/status area: use a wrapping, left-aligned label with
            // enough height so longer guidance does not run into itself.
            hStatusLabel = CreateWindowW(L"STATIC", L"Ready. Open an image, choose a filter, then use Undo/Redo or Clear Filters.",
                WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
                30, 584, 930, 42, hWnd, NULL, NULL, NULL);

            CaptureBaseChildLayouts(hWnd);
            return 0;
        }

        // Real-Time Instant Live Slider Tracking for ALL Filters
        case WM_HSCROLL: {
            if ((HWND)lParam == hSliderIntensity) {
                currentIntensity = (int)SendMessageW(hSliderIntensity, TBM_GETPOS, 0, 0);
                std::wstring label = std::to_wstring(currentIntensity) + L"%";
                SetWindowTextW(hLabelIntensity, label.c_str());

                // If user drags slider without a filter, default to Grayscale live preview
                if (hasImageLoaded && activeFilter == FILTER_NONE) {
                    TriggerGrayscale();
                } else {
                    UpdateLiveFilterIntensity();
                }
            }
            return 0;
        }

        case WM_SIZE: {
            if (wParam != SIZE_MINIMIZED) {
                RECT client;
                GetClientRect(hWnd, &client);
                ResizeResponsiveLayout(hWnd, client.right - client.left, client.bottom - client.top);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE && freeCropMode) {
                freeCropMode = false;
                freeCropDragging = false;
                ReleaseCapture();
                SetWindowTextW(hStatusLabel, L"Free Crop cancelled.");
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                InvalidateRect(hWnd, NULL, FALSE);
                return 0;
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (freeCropMode && hasImageLoaded) {
                POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                RECT imageRect = GetFittedImageRect(currentImage, rectAfterArea);
                if (PointInsideRect(p, imageRect)) {
                    freeCropDragging = true;
                    freeCropStart = freeCropEnd = p;
                    SetCapture(hWnd);
                    InvalidateRect(hWnd, NULL, FALSE);
                } else {
                    SetWindowTextW(hStatusLabel, L"Click and drag inside the AFTER image preview.");
                }
                return 0;
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if (freeCropMode && freeCropDragging) {
                freeCropEnd.x = GET_X_LPARAM(lParam);
                freeCropEnd.y = GET_Y_LPARAM(lParam);
                UpdateFreeCropPixelStatus();
                InvalidateRect(hWnd, NULL, FALSE);
                return 0;
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (freeCropMode && freeCropDragging) {
                freeCropEnd.x = GET_X_LPARAM(lParam);
                freeCropEnd.y = GET_Y_LPARAM(lParam);
                freeCropDragging = false;
                ReleaseCapture();
                ApplyFreeCropFromSelection();
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return 0;
            }
            break;
        }
        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            switch (controlId) {
                case ID_BTN_OPEN:      HandleOpenImage(); break;
                case ID_BTN_SAVE:      HandleSaveImage(); break;
                case ID_BTN_UNDO:      HandleUndo(); break;
                case ID_BTN_REDO:      HandleRedo(); break;
                case ID_BTN_CLEAR:     HandleClearFilters(); break;
                case ID_BTN_COMMIT:    HandleCommitFilter(); break;

                // Live Intensity Filters (All 0-100% Instant Real-Time)
                case ID_FILTER_GRAYSCALE: TriggerGrayscale(); break;
                case ID_FILTER_BW:        TriggerBW(); break;
                case ID_FILTER_DARKEN:    TriggerDarken(); break;
                case ID_FILTER_LIGHTEN:   TriggerLighten(); break;
                case ID_FILTER_INVERT:    TriggerInvert(); break;
                case ID_FILTER_INFRARED:  TriggerInfrared(); break;
                case ID_FILTER_BLUR:      TriggerBlur(); break;
                case ID_FILTER_SUNLIGHT:  TriggerSunlight(); break;
                case ID_FILTER_TV:        TriggerTV(); break;
                case ID_FILTER_PURPLE:    TriggerPurple(); break;
                case ID_FILTER_EDGES:     TriggerDetectEdges(); break;
                case ID_FILTER_FRAME:     TriggerAddFrame(); break;

                // Geometric Transformation Filters
                case ID_FILTER_FLIP:      ShowFlipDialog(); break;
                case ID_FILTER_ROTATE:    ShowRotateDialog(); break;
                case ID_FILTER_CROP:      ShowCropModeDialog(); break;
                case ID_FILTER_PRECISE_CROP: ShowPreciseCropDialog(); break;
                case ID_FILTER_RESIZE:    ShowResizeDialog(); break;
            }
            return 0;
        }

        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* dis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
            if (dis && dis->CtlType == ODT_BUTTON) {
                COLORREF fillColor = RGB(111, 55, 165);
                if (dis->itemState & ODS_DISABLED) fillColor = RGB(175, 160, 190);
                else if (dis->itemState & ODS_SELECTED) fillColor = RGB(78, 35, 120);
                else if (dis->itemState & ODS_HOTLIGHT) fillColor = RGB(145, 78, 205);

                HBRUSH brush = CreateSolidBrush(fillColor);
                FillRect(dis->hDC, &dis->rcItem, brush);
                DeleteObject(brush);

                HPEN pen = CreatePen(PS_SOLID, 1, RGB(180, 145, 215));
                HPEN oldPen = (HPEN)SelectObject(dis->hDC, pen);
                HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
                RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top,
                    dis->rcItem.right, dis->rcItem.bottom, 8, 8);
                SelectObject(dis->hDC, oldBrush);
                SelectObject(dis->hDC, oldPen);
                DeleteObject(pen);

                wchar_t buttonText[128] = L"";
                GetWindowTextW(dis->hwndItem, buttonText, 128);
                SetBkMode(dis->hDC, TRANSPARENT);
                SetTextColor(dis->hDC, RGB(255, 255, 255));
                RECT textRect = dis->rcItem;
                InflateRect(&textRect, -4, -2);
                DrawTextW(dis->hDC, buttonText, -1, &textRect,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            int clientW = clientRect.right - clientRect.left;
            int clientH = clientRect.bottom - clientRect.top;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientW, clientH);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            HBRUSH bgBrush = CreateSolidBrush(RGB(246, 247, 251));
            FillRect(memDC, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            // Clean modern purple header and subtle preview cards.
            int uiScale = std::max(1, std::min(clientW / DESIGN_CLIENT_WIDTH, clientH / DESIGN_CLIENT_HEIGHT));
            RECT headerRect = { 0, 0, clientW, 52 * uiScale };
            HBRUSH headerBrush = CreateSolidBrush(RGB(55, 32, 83));
            FillRect(memDC, &headerRect, headerBrush);
            DeleteObject(headerBrush);

            HPEN cardPen = CreatePen(PS_SOLID, 1, RGB(226, 220, 237));
            HBRUSH cardBrush = CreateSolidBrush(RGB(255, 255, 255));
            HPEN oldCardPen = (HPEN)SelectObject(memDC, cardPen);
            HBRUSH oldCardBrush = (HBRUSH)SelectObject(memDC, cardBrush);
            RoundRect(memDC, rectBeforeArea.left - 18 * uiScale, rectBeforeArea.top - 36 * uiScale,
                      rectBeforeArea.right + 18 * uiScale, rectBeforeArea.bottom + 12 * uiScale, 14 * uiScale, 14 * uiScale);
            RoundRect(memDC, rectAfterArea.left - 18 * uiScale, rectAfterArea.top - 36 * uiScale,
                      rectAfterArea.right + 18 * uiScale, rectAfterArea.bottom + 12 * uiScale, 14 * uiScale, 14 * uiScale);
            SelectObject(memDC, oldCardBrush);
            SelectObject(memDC, oldCardPen);
            DeleteObject(cardBrush);
            DeleteObject(cardPen);

            // Draw the real PIXLat logo asset (keep PIXLat_logo.png beside the project root).
            // GDI+ keeps the PNG alpha channel, so it blends into the purple header.
            {
                Gdiplus::Image logoImage(L"PIXLat_logo.png");
                if (logoImage.GetLastStatus() == Gdiplus::Ok) {
                    Gdiplus::Graphics logoGraphics(memDC);
                    logoGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
                    logoGraphics.DrawImage(&logoImage, 10 * uiScale, 2 * uiScale, 38 * uiScale, 38 * uiScale);
                }
            }

            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            HFONT hFont = CreateFontW(20 * uiScale, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                      DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT oldFont = (HFONT)SelectObject(memDC, hFont);
            DrawTextW(memDC, L"PIXLat  |  CREATIVE IMAGE STUDIO", -1, &headerRect,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);

            // Draw BEFORE and AFTER image preview frames
            RenderImageFit(memDC, originalImage, rectBeforeArea);
            RenderImageFit(memDC, currentImage, rectAfterArea);

            if (freeCropMode) {
                RECT imageRect = GetFittedImageRect(currentImage, rectAfterArea);
                HPEN guidePen = CreatePen(PS_DASH, 2, RGB(255, 255, 255));
                HPEN oldGuidePen = (HPEN)SelectObject(memDC, guidePen);
                HBRUSH oldGuideBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
                Rectangle(memDC, imageRect.left, imageRect.top, imageRect.right, imageRect.bottom);
                if (freeCropDragging) {
                    int l = std::min(freeCropStart.x, freeCropEnd.x);
                    int r = std::max(freeCropStart.x, freeCropEnd.x);
                    int t = std::min(freeCropStart.y, freeCropEnd.y);
                    int b = std::max(freeCropStart.y, freeCropEnd.y);
                    HPEN selectionPen = CreatePen(PS_SOLID, 2, RGB(190, 125, 255));
                    SelectObject(memDC, selectionPen);
                    Rectangle(memDC, l, t, r, b);
                    DeleteObject(selectionPen);
                }
                SelectObject(memDC, oldGuideBrush);
                SelectObject(memDC, oldGuidePen);
                DeleteObject(guidePen);
            }

            BitBlt(hdc, 0, 0, clientW, clientH, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// -------------------------------------------------------------
// Application Entry Point (WinMain)
// -------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    const wchar_t CLASS_NAME[] = L"PIXLatImageProcessorWindow";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    // Load the PIXLat logo embedded in the executable (resource ID 101).
    // This keeps the app icon working even when the .ico file is not beside the EXE.
    HICON hPixlatIcon = (HICON)LoadImageW(
        hInstance, MAKEINTRESOURCEW(101), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    wc.hIcon = hPixlatIcon;
    wc.hIconSm = hPixlatIcon;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassExW(&wc);

    int winWidth = 1010;
    int winHeight = 690;

    hMainWindow = CreateWindowExW(
        0,
        CLASS_NAME,
        L"PIXLat - Purple Image Studio",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        winWidth, winHeight,
        NULL, NULL, hInstance, NULL
    );

    if (hMainWindow == NULL) {
        if (hPixlatIcon) DestroyIcon(hPixlatIcon);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 0;
    }
    if (hPixlatIcon) {
        SendMessageW(hMainWindow, WM_SETICON, ICON_BIG, (LPARAM)hPixlatIcon);
        SendMessageW(hMainWindow, WM_SETICON, ICON_SMALL, (LPARAM)hPixlatIcon);
    }

    // Launch maximized so the workspace uses the available screen instead of
    // opening as a small fixed-size legacy-style window.
    ShowWindow(hMainWindow, SW_SHOWMAXIMIZED);
    UpdateWindow(hMainWindow);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hPixlatIcon) DestroyIcon(hPixlatIcon);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}
