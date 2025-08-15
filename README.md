# Seam Carving in C++

A minimal implementation of the seam carving algorithm for content-aware image resizing.

## What is Seam Carving?

Seam carving is an algorithm for content-aware image resizing. Instead of scaling or cropping, it removes the least important pixels (seams) from the image, preserving important features.

## Dependencies

**No external libraries needed!** This implementation uses:
- `stb_image.h` - for loading images (header-only)
- `stb_image_write.h` - for saving images (header-only)

Both are included in this project and don't require separate installation.

## Compilation

Using clang++:
```bash
clang++ -o seam_carving seam_carving.cpp -std=c++17 -O2
```

Using g++:
```bash
g++ -o seam_carving seam_carving.cpp -std=c++17 -O2
```

Using Makefile:
```bash
make
```

## Usage

```bash
./seam_carving input.jpg output.png 50
```

Parameters:
- `input.jpg` - Input image (supports JPG, PNG, BMP, TGA)
- `output.png` - Output image (PNG format)
- `50` - Number of vertical seams to remove

## How it Works

1. **Energy Calculation**: Uses Scharr operator (similar to OpenCV's cv2.Scharr) for better gradient detection
2. **Forward Energy**: Implements forward energy calculation for optimal seam selection, considering future pixel rearrangement
3. **Dynamic Programming**: Uses the sophisticated forward energy cumulative map for finding optimal seams
4. **Seam Removal**: Removes seams and shifts remaining pixels efficiently

## Example

```bash
# Remove 100 pixels from the width
./seam_carving landscape.jpg landscape_carved.png 100
```

## Limitations

- Only removes vertical seams (width reduction)
- No seam insertion capability (cannot increase image dimensions)
- No horizontal seam processing (height reduction)
- No mask support for object removal or area protection
- Works best on images with varied content
- May distort important features if too many seams are removed

## Future Improvements

This implementation currently provides basic seam removal functionality. The following features are missing compared to the full Python reference implementation:

### 🚀 High Priority Features
- **Seam Insertion**: Add capability to increase image dimensions by inserting seams
- **Horizontal Seam Processing**: Support row removal/insertion by implementing image rotation
- **Target Dimension Resizing**: Allow specifying exact output dimensions (width x height)
- **Backward Energy Algorithm**: Implement backward energy calculation for seam insertion

### 🔧 Medium Priority Features
- **Mask Support**: 
  - Protected area masks (avoid removing important regions)
  - Object removal masks (force removal of unwanted objects)
- **Image Rotation Utilities**: 90-degree rotation for horizontal seam processing
- **Advanced Pixel Interpolation**: Better averaging for seam insertion

### 🎯 Advanced Features
- **Object Removal Mode**: Complete object removal with automatic seam insertion
- **Seam Record Management**: Track and update multiple seam operations
- **Multi-directional Processing**: Combine horizontal and vertical seam operations
- **Energy Map Visualization**: Debug output showing energy calculations

### 📋 Implementation Status
- ✅ Forward energy calculation with Scharr operators
- ✅ Optimal seam finding with dynamic programming  
- ✅ Efficient seam removal
- ❌ Seam insertion
- ❌ Horizontal processing
- ❌ Mask support
- ❌ Object removal
- ❌ Target dimension resizing

