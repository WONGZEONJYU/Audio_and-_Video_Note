//
// Created by Administrator on 2024/7/3.
//

#include "DecoderAbstract.hpp"

DecoderAbstract::DecoderAbstract(DecoderAbstract::Cv_Any_Type &cv,PacketQueue& q):
m_cv{cv},m_queue(q) {

}

void DecoderAbstract::Notify_All() noexcept(true) {
    m_cv.notify_all();
}

