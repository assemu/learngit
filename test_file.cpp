#include <iostream>
#include <opencv/opencv2.hpp>

int main(int argc, char** argv)
{
    int i = 0;

	auto img = cv::imread("C:\\Download\\test.jpg");

    for(i; i < 30; i++)
    {
        std::cout << "hello, world!\n" << std::endl;
    }
    return 0;
}