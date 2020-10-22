#include <opencv2/opencv.hpp>
#include <stdio.h>

cv::Mat inpaint_mask;
cv::Mat original_image, whiteLined_image, inpainted, temp_image;

void myMouseEventHandler(int event, int x , int y , int flags, void *){
    if(whiteLined_image.empty()){
        return;
    }

    static bool isBrushDown = false;
    static cv::Point pressedPt;
    cv::Point pt(x, y);

    temp_image = whiteLined_image.clone();

    // place the button was pressed
    if(event == CV_EVENT_LBUTTONDOWN){
            pressedPt = pt;
    }

    bool isLButtonPressedBeforeEvent = (bool)(flags & CV_EVENT_FLAG_LBUTTON);

    if(isLButtonPressedBeforeEvent && isBrushDown){
        cv::rectangle(temp_image, pressedPt, pt, cv::Scalar(255), 5, 8);
        cv::imshow("image", temp_image);
    }else{
        cv::imshow("image", whiteLined_image);
    }
    if(event == CV_EVENT_LBUTTONUP){
        int start_x = std::min(pressedPt.x, pt.x), start_y = std::min(pressedPt.y, pt.y), end_x = std::max(pressedPt.x, pt.x), end_y = std::max(pressedPt.y, pt.y);

        cv::Size s = whiteLined_image.size();

        for(int j = 0; j < s.height; j++){
            uchar *ptr1;
            ptr1 = whiteLined_image.ptr<uchar>(j);
            if(j < start_y || end_y <= j){
                ptr1 += 3*s.width;
                continue;
            }

            // 4. convert color to gray
            for(int i = 0; i < s.width; i++){
                if(i < start_x || end_x <= i){
                    ptr1 += 3;
                    continue;
                }
                
                for(int k=0; k<3; k++){
                    ptr1[k] = ~ptr1[k];
                }

                ptr1 += 3;
            }
        }
    }

    bool isLButtonPressedAfterEvent = isLButtonPressedBeforeEvent
        ^ ((event == CV_EVENT_LBUTTONDOWN) || (event == CV_EVENT_LBUTTONUP));
    if(isLButtonPressedAfterEvent){
        isBrushDown = true;
    }else{
        isBrushDown = false;
    }
}

int main(int argc, char *argv[]){

    // 1. read image file
    char *filename = (argc >= 2) ? argv[1] : (char *)"fruits.jpg";
    original_image = cv::imread(filename);
    if(original_image.empty()){
        printf("ERROR: image not found!\n");
        return 0;
    }

    // print hot keys
    printf( "Hot keys: \n"
        "\tESC - quit the program\n"
        "\ti or ENTER - run inpainting algorithm\n"
        "\t\t(before running it, paint something on the image)\n");

    // 2. prepare window
    cv::namedWindow("image",1);

    // 3. prepare Mat objects for processing −mask and processed−image
    whiteLined_image = original_image.clone();
    inpainted = original_image.clone();
    inpaint_mask.create(original_image.size(), CV_8UC1);

    inpaint_mask = cv::Scalar(0);
    inpainted = cv::Scalar(0);

    // 4. show image to window for generating mask
    cv::imshow("image", whiteLined_image);

    // 5. set callback function for mouse operations
    cv::setMouseCallback("image", myMouseEventHandler, 0);

    bool loop_flag = true;

    while(loop_flag){
        // 6. wait for key input
        int c = cv::waitKey(0);

        // 7. process according to input
        switch(c){
            case 27:// ESC
            case 'q':
                loop_flag = false;
                break;

            case 'r':
                inpaint_mask = cv::Scalar(0);
                original_image.copyTo(whiteLined_image);
                cv::imshow("image", whiteLined_image);
                break;

            case 'i':
            case 10:// ENTER
                cv::namedWindow("inpainted image", 1);
                cv::inpaint(whiteLined_image, inpaint_mask, inpainted, 3.0, cv::INPAINT_TELEA);
                cv::imshow("inpainted image", inpainted);
                break;
        }
    }
    return 0;
}