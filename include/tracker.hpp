#include <opencv2/core.hpp>
#pragma once
class Tracker
{
   public:
    void initialise(const cv::Mat& frame, const cv::Rect& roi);

    cv::Rect update(const cv::Mat& frame);

   private:
    cv::Mat templateImage;
    cv::Rect currentPosition;

    double compareImages(const cv::Mat& image1, const cv::Mat& image2);
};