#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>

struct Pixel {
    double r, g, b;
    
    Pixel() : r(0), g(0), b(0) {}
    Pixel(double r, double g, double b) : r(r), g(g), b(b) {}
    Pixel(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
};

class SeamCarver {
private:
    std::vector<std::vector<Pixel>> image;
    int width, height;
    
    // Calculate energy using Scharr operator (similar to cv2.Scharr)
    std::vector<std::vector<double>> calcEnergyMap() {
        std::vector<std::vector<double>> energy(height, std::vector<double>(width, 0.0));
        
        // Scharr kernels for gradient calculation
        int scharrX[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};
        int scharrY[3][3] = {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}};
        
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                double gradX_r = 0, gradX_g = 0, gradX_b = 0;
                double gradY_r = 0, gradY_g = 0, gradY_b = 0;
                
                // Apply Scharr filter
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        Pixel& p = image[y + dy][x + dx];
                        gradX_r += p.r * scharrX[dy + 1][dx + 1];
                        gradX_g += p.g * scharrX[dy + 1][dx + 1];
                        gradX_b += p.b * scharrX[dy + 1][dx + 1];
                        gradY_r += p.r * scharrY[dy + 1][dx + 1];
                        gradY_g += p.g * scharrY[dy + 1][dx + 1];
                        gradY_b += p.b * scharrY[dy + 1][dx + 1];
                    }
                }
                
                energy[y][x] = std::abs(gradX_r) + std::abs(gradX_g) + std::abs(gradX_b) +
                              std::abs(gradY_r) + std::abs(gradY_g) + std::abs(gradY_b);
            }
        }
        
        // Set border pixels to high energy
        for (int x = 0; x < width; x++) {
            energy[0][x] = 1000.0;
            energy[height - 1][x] = 1000.0;
        }
        for (int y = 0; y < height; y++) {
            energy[y][0] = 1000.0;
            energy[y][width - 1] = 1000.0;
        }
        
        return energy;
    }
    
    // Calculate neighbor matrix for forward energy
    std::vector<std::vector<double>> calcNeighborMatrix(int kernel[3][3]) {
        std::vector<std::vector<double>> result(height, std::vector<double>(width, 0.0));
        
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                double sum_r = 0, sum_g = 0, sum_b = 0;
                
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        Pixel& p = image[y + dy][x + dx];
                        sum_r += p.r * kernel[dy + 1][dx + 1];
                        sum_g += p.g * kernel[dy + 1][dx + 1];
                        sum_b += p.b * kernel[dy + 1][dx + 1];
                    }
                }
                
                result[y][x] = std::abs(sum_r) + std::abs(sum_g) + std::abs(sum_b);
            }
        }
        
        return result;
    }

    
    // Forward energy cumulative map calculation
    std::vector<std::vector<double>> cumulativeMapForward(const std::vector<std::vector<double>>& energyMap) {
        // Kernels for forward energy calculation
        int kernelX[3][3] = {{0, 0, 0}, {-1, 0, 1}, {0, 0, 0}};
        int kernelYLeft[3][3] = {{0, 0, 0}, {0, 0, 1}, {0, -1, 0}};
        int kernelYRight[3][3] = {{0, 0, 0}, {1, 0, 0}, {0, -1, 0}};
        
        auto matrixX = calcNeighborMatrix(kernelX);
        auto matrixYLeft = calcNeighborMatrix(kernelYLeft);
        auto matrixYRight = calcNeighborMatrix(kernelYRight);
        
        std::vector<std::vector<double>> output = energyMap;
        
        for (int row = 1; row < height; row++) {
            for (int col = 0; col < width; col++) {
                if (col == 0) {
                    double eRight = output[row - 1][col + 1] + matrixX[row - 1][col + 1] + matrixYRight[row - 1][col + 1];
                    double eUp = output[row - 1][col] + matrixX[row - 1][col];
                    output[row][col] = energyMap[row][col] + std::min(eRight, eUp);
                } else if (col == width - 1) {
                    double eLeft = output[row - 1][col - 1] + matrixX[row - 1][col - 1] + matrixYLeft[row - 1][col - 1];
                    double eUp = output[row - 1][col] + matrixX[row - 1][col];
                    output[row][col] = energyMap[row][col] + std::min(eLeft, eUp);
                } else {
                    double eLeft = output[row - 1][col - 1] + matrixX[row - 1][col - 1] + matrixYLeft[row - 1][col - 1];
                    double eRight = output[row - 1][col + 1] + matrixX[row - 1][col + 1] + matrixYRight[row - 1][col + 1];
                    double eUp = output[row - 1][col] + matrixX[row - 1][col];
                    output[row][col] = energyMap[row][col] + std::min({eLeft, eRight, eUp});
                }
            }
        }
        
        return output;
    }
    
    // Find the optimal seam
    std::vector<int> findSeam(const std::vector<std::vector<double>>& cumulativeMap) {
        std::vector<int> seam(height);
        
        // Find minimum in last row
        int minCol = 0;
        for (int col = 1; col < width; col++) {
            if (cumulativeMap[height - 1][col] < cumulativeMap[height - 1][minCol]) {
                minCol = col;
            }
        }
        seam[height - 1] = minCol;
        
        // Backtrack to find the seam
        for (int row = height - 2; row >= 0; row--) {
            int prevCol = seam[row + 1];
            int minPos = prevCol;
            double minVal = cumulativeMap[row][prevCol];
            
            // Check left neighbor
            if (prevCol > 0 && cumulativeMap[row][prevCol - 1] < minVal) {
                minVal = cumulativeMap[row][prevCol - 1];
                minPos = prevCol - 1;
            }
            
            // Check right neighbor
            if (prevCol < width - 1 && cumulativeMap[row][prevCol + 1] < minVal) {
                minVal = cumulativeMap[row][prevCol + 1];
                minPos = prevCol + 1;
            }
            
            seam[row] = minPos;
        }
        
        return seam;
    }
    
    // Remove vertical seam
    void deleteSeam(const std::vector<int>& seamIdx) {
        std::vector<std::vector<Pixel>> newImage(height, std::vector<Pixel>(width - 1));
        
        for (int row = 0; row < height; row++) {
            int col = seamIdx[row];
            int newCol = 0;
            for (int oldCol = 0; oldCol < width; oldCol++) {
                if (oldCol != col) {
                    newImage[row][newCol] = image[row][oldCol];
                    newCol++;
                }
            }
        }
        
        image = std::move(newImage);
        width--;
    }

public:
    bool loadImage(const char* filename) {
        int channels;
        unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);
        if (!data) {
            std::cerr << "Failed to load image: " << filename << std::endl;
            return false;
        }

        image.assign(height, std::vector<Pixel>(width));
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                image[y][x] = Pixel(data[idx], data[idx + 1], data[idx + 2]);
            }
        }

        stbi_image_free(data);
        return true;
    }

    bool saveImage(const char* filename) {
        std::vector<unsigned char> data(width * height * 3);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                data[idx] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, image[y][x].r)));
                data[idx + 1] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, image[y][x].g)));
                data[idx + 2] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, image[y][x].b)));
            }
        }
        return stbi_write_png(filename, width, height, 3, data.data(), width * 3);
    }

    void seamsRemoval(int numPixel) {
        for (int i = 0; i < numPixel && width > 1; i++) {
            auto energyMap = calcEnergyMap();
            auto cumulativeMap = cumulativeMapForward(energyMap);
            auto seamIdx = findSeam(cumulativeMap);
            deleteSeam(seamIdx);
            std::cout << "Removed seam " << (i + 1) << "/" << numPixel 
                      << " (new width: " << width << ")" << std::endl;
        }
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <input_image> <output_image> <num_seams>" << std::endl;
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];
    int numSeams = std::atoi(argv[3]);

    SeamCarver carver;
    
    if (!carver.loadImage(inputFile)) {
        return 1;
    }

    std::cout << "Loaded image: " << carver.getWidth() << "x" << carver.getHeight() << std::endl;

    if (numSeams >= carver.getWidth()) {
        std::cerr << "Cannot remove " << numSeams << " seams from image with width " 
                  << carver.getWidth() << std::endl;
        return 1;
    }

    carver.seamsRemoval(numSeams);

    if (!carver.saveImage(outputFile)) {
        std::cerr << "Failed to save image: " << outputFile << std::endl;
        return 1;
    }

    std::cout << "Seam carving complete! Output saved to: " << outputFile << std::endl;
    return 0;
}
