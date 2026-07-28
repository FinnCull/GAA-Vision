#include "../include/tracker.hpp"

#include <cstdint>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
void Tracker::initialise(const cv::Mat& frame, const cv::Rect& roi)
{
    currentPosition = roi;
    templateImage = frame(roi).clone();

    cv::Mat croppedImage = frame(roi);

    cv::cvtColor(croppedImage, templateImage, cv::COLOR_BGR2GRAY);
}

double Tracker::compareImages(const cv::Mat& image1, const cv::Mat& image2)
{
    if (image1.size() != image2.size())
    {
        throw std::invalid_argument("Images must be the same size");
    }

    std::uint64_t totalDifference = 0.0;

    for (int row = 0; row < image1.rows; row++)
    {
        const unsigned char* row1 = image1.ptr(row);
        const unsigned char* row2 = image2.ptr(row);

        for (int col = 0; col < image1.cols; col++)
        {
            totalDifference += std::abs(row1[col] - row2[col]);
        }
    }
    return static_cast<double>(totalDifference) / static_cast<double>(image1.total());
}

cv::Rect Tracker::update(const cv::Mat& frame)
{
    cv::Mat grayFrame;

    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

    const int searchRadius = 10;

    double bestScore = std::numeric_limits<double>::max();

    cv::Rect bestPosition = currentPosition;

    for (int yOffset = -searchRadius; yOffset <= searchRadius; ++yOffset)
    {
        for (int xOffset = -searchRadius; xOffset <= searchRadius; ++xOffset)
        {
            cv::Rect candidatePosition(currentPosition.x + xOffset, currentPosition.y + yOffset,
                                       currentPosition.width, currentPosition.height);
            if (candidatePosition.x < 0 || candidatePosition.y < 0 ||
                candidatePosition.x + candidatePosition.width > grayFrame.cols ||
                candidatePosition.y + yOffset > grayFrame.rows)
            {
                continue;
            }

            cv::Mat candidateImage = grayFrame(candidatePosition);

            const double score = compareImages(templateImage, candidateImage);

            if (score > bestScore)
            {
                bestScore = score;
                bestPosition = candidatePosition;
            }
        }
    }

    currentPosition = bestPosition;
    return currentPosition;
}