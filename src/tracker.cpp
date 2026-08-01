#include "../include/tracker.hpp"

#include <opencv2/core/hal/interface.h>

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

void Tracker::initialise(const cv::Mat& frame, const cv::Rect& roi)
{
    if (frame.empty())
    {
        throw std::invalid_argument("Cannot initialise tracker with empty frame");
    }
    const cv::Rect frameBounds(0, 0, frame.cols, frame.rows);

    if (roi.width <= 0 || roi.height <= 0 || (roi & frameBounds) != roi)
    {
        throw std::invalid_argument("ROI must be fully in the frame");
    }

    currentPosition = roi;

    const cv::Mat croppedImage = frame(roi);
    cv::cvtColor(croppedImage, templateImage, cv::COLOR_BGR2GRAY);

    initialised = true;
}

double Tracker::compareImages(const cv::Mat& image1, const cv::Mat& image2)
{
    if (!initialised)
    {
        throw std::logic_error("tracker must be initialised before update");
    }

    if (image1.empty() || image2.empty())
    {
        throw std::invalid_argument("Images must not be empty");
    }

    if (image1.size() != image2.size())
    {
        throw std::invalid_argument("Images must be the same size");
    }

    if (image1.type() != CV_8UC1 || image2.type() != CV_8UC1)
    {
        throw std::invalid_argument("Images must be 8-bit grayscale images");
    }

    std::uint64_t totalDifference = 0.0;

    for (int row = 0; row < image1.rows; row++)
    {
        const uint8_t* row1 = image1.ptr<std::uint8_t>(row);
        const uint8_t* row2 = image2.ptr<std::uint8_t>(row);

        for (int col = 0; col < image1.cols; col++)
        {
            const int difference = static_cast<int>(row1[col]) - static_cast<int>(row2[col]);
            totalDifference += static_cast<std::uint64_t>(std::abs(difference));
        }
    }
    return static_cast<double>(totalDifference) / static_cast<double>(image1.total());
}

TrackingResult Tracker::update(const cv::Mat& frame)
{
    if (frame.empty())
    {
        throw std::invalid_argument("Frame must not be empty");
    }

    if (!initialised)
    {
        throw std::logic_error("Tracker must be initialised before update");
    }

    cv::Mat grayFrame;

    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

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
                candidatePosition.y + candidatePosition.height > grayFrame.rows)
            {
                continue;
            }

            cv::Mat candidateImage = grayFrame(candidatePosition);

            const double score = compareImages(templateImage, candidateImage);

            if (score < bestScore)
            {
                bestScore = score;
                bestPosition = candidatePosition;
            }
        }
    }

    const bool found = bestScore <= threshold;

    if (found)
    {
        currentPosition = bestPosition;
    }

    return {bestPosition, bestScore, found};
}

Tracker::Tracker(int searchRadius) : searchRadius(searchRadius)
{
    if (searchRadius < 0)
    {
        throw std::invalid_argument("Search radius cannot be negative");
    }
}
