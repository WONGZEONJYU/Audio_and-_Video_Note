//
// Created by Administrator on 2024/8/28.
//

#include "xvideo_view.hpp"
#include "xsdl.hpp"

XVideoView *XVideoView::create(const XVideoView::RenderType &renderType) {

    switch (renderType) {
        case SDL:
            return new XSDL();
        default:
            return {};
    }
}
