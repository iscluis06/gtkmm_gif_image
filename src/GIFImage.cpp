#include "GIFImage.h"
#include <gif_lib.h>
#include <iostream>

GIFImage::GIFImage(char *image_path) {
    int image_path_size = strlen(image_path);
    this->image_path_ = new char[image_path_size];
    memcpy(image_path_, image_path, image_path_size);
    this->image_count_ = GetImageCount();
    if (this->image_count_ > 0) {
        m_connection_ = Glib::signal_timeout().connect(sigc::mem_fun(*this, &GIFImage::UpdateImage), GetFrameDelay());
    }
}
bool GIFImage::UpdateImage() {
    auto rgb_buffer = GetFrameBuffer();
    if (rgb_buffer.buffer == nullptr) {
        return false;
    }
    auto pixbuf = Gdk::Pixbuf::create_from_data(rgb_buffer.buffer,Gdk::Colorspace::RGB,false, 8,rgb_buffer.width,
        rgb_buffer.height, rgb_buffer.width*3);
    auto texture = Gdk::Texture::create_for_pixbuf(pixbuf);
    set(texture);
    delete rgb_buffer.buffer;
    return true;
}
GIFImage::FrameBufferHandle GIFImage::GetFrameBuffer() {
    GIFImage::FrameBufferHandle frame_buffer_hande{};
    int error = 0;
    auto gif_handle = DGifOpenFileName(image_path_, &error);
    if (!gif_handle) {
        std::cerr << "Error opening GIF file " << std::endl;
        return frame_buffer_hande;
    }
    DGifSlurp(gif_handle);
    if (index_ >= image_count_-1) {
        index_ =-1;
    }
    index_++;
    auto frame = gif_handle->SavedImages[index_];
    frame_buffer_hande.width = frame.ImageDesc.Width;
    frame_buffer_hande.height = frame.ImageDesc.Height;
    ColorMapObject *colorMap = frame.ImageDesc.ColorMap ? frame.ImageDesc.ColorMap : gif_handle->SColorMap;
    int pixels = frame_buffer_hande.width*frame_buffer_hande.height;
    frame_buffer_hande.buffer = new unsigned char[pixels*3];
    for (int i=0;i<pixels;i++) {
        GifPixelType pixelIndex = frame.RasterBits[i];
        GifColorType color = colorMap->Colors[pixelIndex];
        frame_buffer_hande.buffer[i*3+0] = color.Red;
        frame_buffer_hande.buffer[i*3+1] = color.Green;
        frame_buffer_hande.buffer[i*3+2] = color.Blue;
    }
    DGifCloseFile(gif_handle, &error);
    return frame_buffer_hande;
}
int GIFImage::GetImageCount() {
    int error = 0;
    auto gif_handle = DGifOpenFileName(image_path_, &error);
    if (!gif_handle) {
        std::cerr << "Error opening GIF File" << std::endl;
        std::cerr << "Error code: " << error << std::endl;
        return 0;
    }
    DGifSlurp(gif_handle);
    int image_count = gif_handle->ImageCount;
    DGifCloseFile(gif_handle,&error);
    return image_count;
}
int GIFImage::GetFrameDelay() {
    int error = 0;
    auto gif_handle = DGifOpenFileName(image_path_, &error);
    if (!gif_handle) {
        std::cerr << "Error opening GIF File" << std::endl;
        std::cerr << "Error code: " << error << std::endl;
        return 0;
    }
    DGifSlurp(gif_handle);

    if (!gif_handle->ImageCount) {
        std::cerr << "Error GIF File has no images?" << std::endl;
        return -1;
    }

    GraphicsControlBlock gcb;
    DGifSavedExtensionToGCB(gif_handle, 0, &gcb);
    double delay_milliseconds = ((gcb.DelayTime /100.0) * 1000);
    DGifCloseFile(gif_handle,&error);
    return static_cast<int>(delay_milliseconds);
}
