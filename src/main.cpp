#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <sstream>
#include <string>

#include "tracker.hpp"
namespace
{

std::string formatTime(double seconds)
{
    const int totalSeconds = static_cast<int>(seconds);
    const int minutes = totalSeconds / 60;
    const int remainingSeconds = totalSeconds % 60;

    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(2) << minutes << ":" << std::setw(2)
           << remainingSeconds;

    return stream.str();
}

}  // namespace

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <video-path>\n";
        return EXIT_FAILURE;
    }

    const std::string inputPath = argv[1];

    cv::VideoCapture video(inputPath);

    if (!video.isOpened())
    {
        std::cerr << "Error: could not open video: " << inputPath << '\n';
        return EXIT_FAILURE;
    }

    const double fps = video.get(cv::CAP_PROP_FPS);
    const int width = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
    const int height = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));
    const int frameCount = static_cast<int>(video.get(cv::CAP_PROP_FRAME_COUNT));

    if (fps <= 0.0 || width <= 0 || height <= 0)
    {
        std::cerr << "Error: invalid video metadata.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Video opened successfully\n"
              << "Resolution: " << width << "x" << height << '\n'
              << "FPS: " << fps << '\n'
              << "Frames: " << frameCount << '\n';

    cv::VideoWriter output("annotated_match.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps,
                           cv::Size(width, height));

    if (!output.isOpened())
    {
        std::cerr << "Error: could not create output video.\n";
        return EXIT_FAILURE;
    }

    cv::Mat frame;

    if (!video.read(frame))
    {
        std::cerr << "Error: could not read the first frame.\n";
        return EXIT_FAILURE;
    }

    cv::Rect playerBox = cv::selectROI("Select Player", frame, false, false);

    cv::destroyWindow("Select Player");

    if (playerBox.width == 0 || playerBox.height == 0)
    {
        std::cerr << "Error: no player was selected.\n";
        return EXIT_FAILURE;
    }

    Tracker tracker;
    tracker.initialise(frame, playerBox);

    int frameNumber = 0;

    while (video.read(frame))
    {
        cv::Rect trackedPosition = tracker.update(frame);

        cv::rectangle(frame, trackedPosition, cv::Scalar(0, 255, 0), 2);

        cv::imshow("Tracking", frame);

        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }

    video.release();
    output.release();
    cv::destroyAllWindows();

    std::cout << "Processed " << frameNumber << " frames.\n"
              << "Saved output to annotated_match.mp4\n";

    return EXIT_SUCCESS;
}