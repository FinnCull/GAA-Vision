#include <opencv2/core.hpp>
#pragma once

struct TrackingResult
{
    cv::Rect position;
    double score;
    bool found;
};
class Tracker
{
   public:
    explicit Tracker(int searchRadius = 20);

    void initialise(const cv::Mat& frame, const cv::Rect& roi);

    TrackingResult update(const cv::Mat& frame);

   private:
    int searchRadius;
    bool initialised = false;
    double threshold = 20;
    cv::Mat templateImage;
    cv::Rect currentPosition;

    double compareImages(const cv::Mat& image1, const cv::Mat& image2);
};