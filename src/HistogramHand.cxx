#include "HistogramHand.hxx"

static const cv::Mat closekernel = (cv::Mat_<uchar>(cv::Size(5,5)) <<
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0);

Hand_ HistogramHand::create() {
    return Hand_(new HistogramHand());
}

HistogramHand::HistogramHand()
    : theRanges_{calibration_.ranges[0],
        calibration_.ranges[1],
        calibration_.ranges[2]},
      erodeFilter_(cv::createMorphologyFilter(cv::MORPH_ERODE, CV_8UC1, closekernel)) {
}

static inline bool findBlob(const cv::Mat & binsearch,
        cv::Point * loc = NULL, const cv::Point & startloc = cv::Point(0, 0)) {
    for (int x = startloc.x; x < binsearch.cols; ++x) {
        if (binsearch.at<uchar>(startloc.y, x)) {
            if (loc) {
                loc->x = x;
                loc->y = startloc.y;
            }
            return true;
        }
    }
    for (int y = startloc.y + 1; y < binsearch.rows; ++y) {
        for (int x = 0; x < binsearch.cols; ++x) {
            if (binsearch.at<uchar>(y, x)) {
                if (loc) {
                    loc->x = x;
                    loc->y = y;
                }
                return true;
            }
        }
    }
    return false;
}

void HistogramHand::update(const cv::Mat & cam) {
    cv::cvtColor(cam, hsv_, CV_BGR2HSV);
    cv::calcBackProject(&hsv_, 1, calibration_.chs, calibration_.hist, bp_, theRanges_);
    erodeFilter_->apply(bp_, bp_);

    cv::Rect rect(0, 0, 0, 0);
    int area = 0;

    cv::Point maxl;
    double max;
    cv::minMaxLoc(bp_, NULL, &max, NULL, &maxl);
    double limit = max - 1;
    if (limit > 0) {
        area = cv::floodFill(bp_, maxl, cv::Scalar(0), &rect,
                cv::Scalar(limit), cv::Scalar(0), 8 | CV_FLOODFILL_FIXED_RANGE);
    }
    
    if (rect.height > 0 && rect.area() * calibration_.fillRatio < area) {
        position_.x = cam.cols - 1 - (rect.x + rect.width / 2.0f);
        position_.y = rect.y + rect.height / 2.0f;
        position_.z = std::max(rect.height, rect.width) / 2.0f;
        valid_ = true;
    } else {
        valid_ = false;
    }
}
