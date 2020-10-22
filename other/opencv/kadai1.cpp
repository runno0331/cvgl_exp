#include <opencv2/opencv.hpp>
#include <stdio.h>

#define FLAG 1 // (0: direct access / 1: built −in function )

char *preset_file = "fruits.jpg";

void convertToBlured(cv::Mat &input, cv::Mat &blureded);
void convertToSobeled(cv::Mat &input, cv::Mat &sobeled);

int main(int argc, char *argv[])
{
    char *input_file;
    // 1. prepare Mat objects for input−image and output−image
    cv::Mat input, blured, sobeled;

    if(argc == 2){
        input_file = argv[1];
    }
    else{
        input_file = preset_file;
    }

    // 2. read an image from the specified file
    input = cv::imread(input_file, 1);
    if(input.empty()){
        fprintf(stderr, "cannot open %s\n", input_file);
        exit(0);
    }

    convertToBlured(input, blured);
    convertToSobeled(input, sobeled);
    
    // 5. create windows
    cv::namedWindow("original image", 1);
    cv::namedWindow("blured image", 1);
    cv::namedWindow("sobeled image", 1);

    // 6. show images
    cv::imshow("original image", input);
    cv::imshow("blured image", blured);
    cv::imshow("sobeled image", sobeled);
    
    // 7. wait key input
    cv::waitKey(0);
    
    // 8. save the processed result
    cv::imwrite("blured.jpg", blured);
    cv::imwrite("sobeled.jpg", sobeled);
    
    return 0;
}

void convertToBlured(cv::Mat &input, cv::Mat &blured)
{
#if FLAG // use built−in function
    cv::Size ksize = cv::Size(11, 11);

    // 4. convert to blured image
    cv::GaussianBlur(input, blured, ksize, 0);


#else

    // 3. create Mat for output−image
    cv::Size s = input.size();
    blured.create(s, CV_8UC1);
    int weight_map = {{1, 4, 6, 4, 1}, {4, 16, 24, 16, 4}, {6, 24, 36, 24, 6}, {4, 16, 24, 16, 4}, {1, 4, 6, 4, 1}};

    for(int j = 0; j < s.height; j++){
        uchar *ptr1, *ptr2;
        ptr1 = input.ptr<uchar>(j);
        ptr2 = blured.ptr<uchar>(j);

        // 4. convert into blured
        for(int i = 0; i < s.width; i++){
            int total_weight = 0;

            for(int k=-2; k<=2; k++){
                for(int l=-2; l<=2; l++){
                    if(k == 0 && l == 0) continue;
                    if()
                        total_weight += weight_map[k+2][l+2]
                }
            }
            double y = 0.114 * ((double)ptr1[0]) + 0.587 * (double)ptr1[1] + 0.299 * (double)ptr1[2];
            if(y > 255) y = 255;
            if(y < 0) y = 0;
            
            *ptr2 = (uchar)y;
            ptr1 += 3;
            ptr2 += 3;
        }
    }
#endif
}

void convertToSobeled(cv::Mat &input, cv::Mat &sobeled)
{
#if FLAG // use built−in function
    cv::Mat grayed;
    cv::cvtColor(input, grayed, CV_BGR2GRAY);
    cv::Size s = grayed.size();
    for(int j = 0; j < s.height; j++){
        uchar *ptr1;
        ptr1 = grayed.ptr<uchar>(j);

        // 4. convert color to gray
        for(int i = 0; i < s.width; i++){            
            *ptr1 = (uchar)*ptr1/40*40;
            if(*ptr1 > 255){
                *ptr1 = 255;
            }
            ptr1++;
        }
    }
    cv::imshow("gray image", grayed);

    // 4. convert image using sobel filter
    cv::Sobel(grayed, sobeled, CV_8U, 1, 0, 3);
    cv::threshold(sobeled, sobeled, 30, 255, CV_THRESH_BINARY);

#else

    // 3. create Mat for output−image
    cv::Size s = input.size();
    processed.create(s, CV_8UC1);

    for(int j = 0; j < s.height; j++){
        uchar *ptr1, *ptr2;
        ptr1 = input.ptr<uchar>(j);
        ptr2 = processed.ptr<uchar>(j);

        // 4. convert color to gray
        for(int i = 0; i < s.width; i++){
            double y = 0.114 * ((double)ptr1[0]) + 0.587 * (double)ptr1[1] + 0.299 * (double)ptr1[2];
            if(y > 255) y = 255;
            if(y < 0) y = 0;
            
            *ptr2 = (uchar)y;
            ptr1 += 3;
            ptr2++;
        }
    }
#endif
}