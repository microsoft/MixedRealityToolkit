#include <opencv2\aruco.hpp>
#include <opencv2\highgui.hpp>

const int markerSize = 300;
const int qrCodePixelBorder = 34;
const int width = 3;
const int height = 6;

int main()
{
    cv::Mat calibrationBoard = cv::Mat(height * markerSize, 2 * width * markerSize, CV_8U, cv::Scalar(255));
    auto dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    for (int m = 0; m < height; m++)
    {
        for (int n = 0; n < width; n++)
        {
            const int markerId = (m * width) + n;

            auto qrCodeImageName = "QRCodes\\sv" + std::to_string(markerId) + ".png";
            cv::Mat qrCodeMat = cv::imread(qrCodeImageName, CV_8U);
            auto qrCodeOffset = cv::Rect(
                markerSize * (2 * n),
                markerSize * m,
                markerSize,
                markerSize);
            qrCodeMat.copyTo(calibrationBoard(qrCodeOffset));

            cv::Mat arucoMat;
            cv::aruco::drawMarker(
                dictionary,
                markerId,
                markerSize - (2 * qrCodePixelBorder),
                arucoMat);
            auto arucoOffset = cv::Rect(
                (markerSize * (2 * n + 1)) + qrCodePixelBorder,
                (markerSize * m) + qrCodePixelBorder,
                markerSize - (2 * qrCodePixelBorder),
                markerSize - (2 * qrCodePixelBorder));
            arucoMat.copyTo(calibrationBoard(arucoOffset));
        }
    }

    cv::imwrite("CalibrationBoard.png", calibrationBoard);
}