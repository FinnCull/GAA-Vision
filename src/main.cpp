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
    std::vector<cv::Point> playerPath;

    while (video.read(frame))
    {
        frameNumber++;

        TrackingResult trackedPosition = tracker.update(frame);

        if (trackedPosition.found)
        {
            std::cout << "Player found at frame " << frameNumber << " with score "
                      << trackedPosition.score << std::endl;
            cv::rectangle(frame, trackedPosition.position, cv::Scalar(0, 255, 0), 2);
            cv::Point playerCentre =
                cv::Point(trackedPosition.position.x + trackedPosition.position.height / 2,
                          trackedPosition.position.y + trackedPosition.position.width / 2);
            playerPath.push_back(playerCentre);
        }
        else
        {
            std::cout << "Player not found at frame " << frameNumber << std::endl;
            cv::rectangle(frame, trackedPosition.position, cv::Scalar(0, 0, 255), 2);
            cv::putText(frame, "Tracking Lost - Press R to reselect", cv::Point(10, 40),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
        }
        std::ostringstream trackingText;
        trackingText << std::fixed << std::setprecision(1)
                     << "Tracker match score: " << trackedPosition.score;
        cv::putText(frame, trackingText.str(), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    trackedPosition.found ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255), 2);

        const double currentTime = frameNumber / fps;
        const std::string timeText = formatTime(currentTime);
        cv::putText(frame, timeText, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(0, 0, 255), 2);

        for (std::size_t i = 0; i < playerPath.size() - 1; ++i)
        {
            cv::line(frame, playerPath[i], playerPath[i + 1], cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Tracking", frame);

        output.write(frame);
        const int key = cv::waitKey(1) & 0xFF;
        if (key == 27 || key == 'q')
        {
            break;
        }

        if (key == 'r')
        {
            const cv::Rect newPlayerBox = cv::selectROI("Select Player", frame, false, false);
            cv::destroyWindow("Select Player");
            if (newPlayerBox.width > 0 && newPlayerBox.height > 0)
            {
                tracker.initialise(frame, newPlayerBox);
            }
            else
            {
                std::cerr << "Error: no player was selected.\n";
                continue;
            }
        }
    }

    video.release();
    output.release();
    cv::destroyAllWindows();

    std::cout << "Processed " << frameNumber << " frames.\n"
              << "Saved output to annotated_match.mp4\n";

    return EXIT_SUCCESS;
}