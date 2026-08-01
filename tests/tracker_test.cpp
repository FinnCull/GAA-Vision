#include "tracker.hpp"

#include <cstdlib>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>

namespace
{

int failures = 0;

void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

cv::Mat makeFrame(const cv::Rect& target)
{
    cv::Mat frame = cv::Mat::zeros(100, 100, CV_8UC3);

    // A patterned target is less ambiguous than a solid rectangle.
    cv::rectangle(frame, target, cv::Scalar(40, 120, 220), cv::FILLED);
    cv::line(frame, target.tl(), target.br() - cv::Point(1, 1), cv::Scalar(255, 255, 255), 2);

    return frame;
}

void tracksStationaryTarget()
{
    const cv::Rect target(30, 30, 15, 20);
    const cv::Mat frame = makeFrame(target);

    Tracker tracker;
    tracker.initialise(frame, target);

    const TrackingResult result = tracker.update(frame);

    expect(result.found, "stationary target should be found");
    expect(result.position == target, "stationary target position should not change");
    expect(result.score == 0.0, "identical target should have score zero");
}

void tracksMovingTarget()
{
    const cv::Rect initialTarget(30, 30, 15, 20);
    const cv::Rect movedTarget(36, 34, 15, 20);

    Tracker tracker(10);
    tracker.initialise(makeFrame(initialTarget), initialTarget);

    const TrackingResult result = tracker.update(makeFrame(movedTarget));

    expect(result.found, "moving target should be found");
    expect(result.position == movedTarget, "tracker should follow the moving target");
}

void rejectsInvalidRoi()
{
    Tracker tracker;
    const cv::Mat frame = cv::Mat::zeros(100, 100, CV_8UC3);

    bool threw = false;

    try
    {
        tracker.initialise(frame, cv::Rect(95, 95, 10, 10));
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }

    expect(threw, "ROI outside the frame should throw");
}

void rejectsNegativeSearchRadius()
{
    bool threw = false;

    try
    {
        Tracker tracker(-1);
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }

    expect(threw, "negative search radius should throw");
}

}  // namespace

int main()
{
    tracksStationaryTarget();
    tracksMovingTarget();
    rejectsInvalidRoi();
    rejectsNegativeSearchRadius();

    if (failures != 0)
    {
        std::cerr << failures << " test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tracker tests passed\n";
    return EXIT_SUCCESS;
}