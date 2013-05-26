#include <stdio.h>
#include <sys/socket.h>
#include "cv.h"
#include "highgui.h"
#include "jpeglib.h"
#include <image_processing.h>

int ipl2jpeg(IplImage *frame, unsigned char **outbuffer, long unsigned int *outlen)
{
    unsigned char *outdata = (uchar *) frame->imageData;
    struct jpeg_compress_struct cinfo = {0};
    struct jpeg_error_mgr jerr;
    JSAMPROW row_ptr[1];
    int row_stride;

    *outbuffer = NULL;
    *outlen = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, outbuffer, outlen);

    cinfo.image_width = frame->width;
    cinfo.image_height = frame->height;
    cinfo.input_components = frame->nChannels;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 50, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = frame->width * frame->nChannels;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_ptr[0] = &outdata[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_ptr, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return 0;

}

void *VideoCaptureAndSend(void *ptr)
{
    long unsigned int jpeg_len;
    unsigned char *outbuffer;
    int socket = (int) ptr;

    /* Инициализация openCV */
    CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );
    cvSetCaptureProperty(capture,CV_CAP_PROP_FPS,10);
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,320);
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,240);
    double width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    double height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    IplImage* frame;
    printf("Разрешение камеры: %.0f x %.0f\n", width, height );
    if ( !capture )
    {
        fprintf( stderr, "ERROR: capture is NULL \n" );
    }
    else
    {
        CvFont font;
        cvInitFont( &font, CV_FONT_HERSHEY_COMPLEX,1.0, 1.0, 0, 1, CV_AA);
        CvPoint pt = cvPoint( 50, 50 );
        do
        {
            frame = cvQueryFrame( capture );
            if ( !frame )
            {
                fprintf( stderr, "ERROR: frame is null...\n" );
                break;
            }
            cvPutText(frame, "OpenCV Step By Step", pt, &font, CV_RGB(255, 0, 0) );
            ipl2jpeg(frame, &outbuffer, &jpeg_len);
        }
        while(send(socket,outbuffer,jpeg_len,0)!=-1);
        cvReleaseCapture(&capture);
        cvReleaseImage(&frame);
    }
	return 0;
}