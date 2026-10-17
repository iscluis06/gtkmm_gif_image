#include "GIFImage.h"
#include <gif_lib.h>
#include <iostream>

GIFImage::GIFImage(char *image_path) {
    int image_path_size = strlen(image_path);
    this->image_path_ = new char[image_path_size+1];
    memset(this->image_path_,0, sizeof(char) * (image_path_size+1));
    memcpy(image_path_, image_path, image_path_size);
    Initialize();
}

void GIFImage::Initialize() {
    int error = 0;
    auto gif_handle = DGifOpenFileName(image_path_, &error);
    if (!gif_handle) {
        std::cerr << "Error opening GIF File" << std::endl;
        std::cerr << "Error code: " << error << std::endl;
        return;
    }
    DGifSlurp(gif_handle);
    GraphicsControlBlock gcb;
    DGifSavedExtensionToGCB(gif_handle, 0, &gcb);
    int delay = static_cast<int>((gcb.DelayTime /100.0) * 1000);

    this->image_count_ = gif_handle->ImageCount;
    if (this->image_count_ <= 0) {
        std::cerr << "Gif file doesn't contain any frames." << std::endl;
        return;
    }
    this->frame_buffer_handle_list = new FrameBufferHandle[image_count_];
    for (int i=0;i<this->image_count_;i++) {
        auto frame = gif_handle->SavedImages[i];
        this->frame_buffer_handle_list[i].width = frame.ImageDesc.Width;
        this->frame_buffer_handle_list[i].height = frame.ImageDesc.Height;
        ColorMapObject *colorMap = frame.ImageDesc.ColorMap ? frame.ImageDesc.ColorMap : gif_handle->SColorMap;
        int pixels = this->frame_buffer_handle_list[i].width*this->frame_buffer_handle_list[i].height;
        this->frame_buffer_handle_list[i].buffer = new unsigned char[pixels*3];
        for (int j=0;j<pixels;j++) {
            GifPixelType pixelIndex = frame.RasterBits[j];
            GifColorType color = colorMap->Colors[pixelIndex];
            this->frame_buffer_handle_list[i].buffer[j*3+0] = color.Red;
            this->frame_buffer_handle_list[i].buffer[j*3+1] = color.Green;
            this->frame_buffer_handle_list[i].buffer[j*3+2] = color.Blue;
        }
    }
    DGifCloseFile(gif_handle,&error);
    m_connection_ = Glib::signal_timeout().connect(sigc::mem_fun(*this, &GIFImage::UpdateImage), delay);
}
GIFImage::~GIFImage() {
    if (this->image_count_ <= 0) {
        return;
    }
    for (int i=0; i<this->image_count_; i++) {
        delete this->frame_buffer_handle_list[i].buffer;
    }
    delete[] this->frame_buffer_handle_list;
}

bool GIFImage::UpdateImage() {
    if (index_ == this->image_count_-1) {
        index_ = -1;
    }
    index_++;
    this->frame_buffer_handle_list[index_];
    if (this->frame_buffer_handle_list[index_].buffer == nullptr) {
        return false;
    }
    auto pixbuf = Gdk::Pixbuf::create_from_data(this->frame_buffer_handle_list[index_].buffer,Gdk::Colorspace::RGB,false, 8,this->frame_buffer_handle_list[index_].width,
        this->frame_buffer_handle_list[index_].height, this->frame_buffer_handle_list[index_].width*3);
    auto texture = Gdk::Texture::create_for_pixbuf(pixbuf);
    set(texture);
    return true;
}