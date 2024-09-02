#include <iostream>
#include <fstream>
#include <vector>
#include "XSwsContext.hpp"

using namespace std;

static inline constexpr char yuv_file[]{
    GET_STR(400_300_25.yuv)
};

static inline constexpr char rgba_file[]{
        GET_STR(rgba_800_600.rgba)
};

int main(const int argc,const char *argv[]) {

    (void )argc,(void )argv;

    static constexpr auto yuv_w{400},yuv_h{300};
    static constexpr auto rgba_w{800},rgba_h{600};

    int ret{};
    //YUV420P 平面存储
    // YYYY UU VV ...
    vector<uint8_t > y(yuv_w * yuv_h,0),
                    u(yuv_w * yuv_h / 4,0),
                    v(yuv_w * yuv_h / 4,0);

    uint8_t *yuv[4]{y.data(),
                    u.data(),
                    v.data()};

    const int yuv_line_size[4]{yuv_w,yuv_w / 2 , yuv_w / 2};

    //RGBA 交错存储
    // RGBA RGBA RGBA ...
    vector<uint8_t> r_g_b_a(rgba_w * rgba_h * 4,0);
    uint8_t *rgba[1]{r_g_b_a.data()};
    const int rgba_line_size[1] {rgba_w * 4};

    //YUV转RGBA
    ifstream ifs(yuv_file,ios::binary);
    if (!ifs) {
        PRINT_ERR_TIPS(GET_STR(yuv_file open failed!));
        return -1;
    }

    ofstream ofs(rgba_file,ios::trunc | ios::binary);
    if (!ofs) {
        PRINT_ERR_TIPS(GET_STR(rgba_file open failed!));
        return -1;
    }

    try {
        auto rgba2yuv{newXSwsContext()};

        while (true) {
            if (ifs.eof()){
                break;
            }

            ifs.read(reinterpret_cast<char *>(y.data()),yuv_w * yuv_h);
            ifs.read(reinterpret_cast<char *>(u.data()),yuv_w * yuv_h / 4);
            ifs.read(reinterpret_cast<char *>(v.data()),yuv_w * yuv_h / 4);

            CHECK_EXC(rgba2yuv->reinit(yuv_w,yuv_h,AV_PIX_FMT_YUV420P,
                        rgba_w,rgba_h,AV_PIX_FMT_RGBA));

            int ret_height;
            CHECK_EXC(ret_height = rgba2yuv->scale(yuv, //输入YUV数据
                       yuv_line_size, //输入YUV行字节数
                       {},
                       yuv_h, //输入高度
                       rgba, //输出RGBA数据
                       rgba_line_size)); //输出RGBA行字节数
            cerr << GET_STR(ret_height: ) << ret_height << "\n";
            ofs.write(reinterpret_cast<const char *>(*rgba),rgba_w * rgba_h * 4);
        }

        cerr << GET_STR(yuv420 to rgba complete\n\n);

        ifs.close();
        ofs.close();
        v.clear();u.clear();v.clear();
        r_g_b_a.clear();
        rgba2yuv.reset();

        //RGBA转YUV
        ifs.open(rgba_file,ios::binary);
        if (!ifs){
            PRINT_ERR_TIPS(GET_STR(rgba_file open failed));
            return -1;
        }

        ofs.open(yuv_file,ios::binary);
        if (!ofs){
            PRINT_ERR_TIPS(GET_STR(yuv_file open failed));
            return -1;
        }

        auto yuv2rgba{newXSwsContext()};
        while (true) {
            if (ifs.eof()) {
                break;
            }

            ifs.read(reinterpret_cast<char *>(*rgba),rgba_w * rgba_h * 4);

            CHECK_EXC(yuv2rgba->reinit(rgba_w,rgba_h,
                        AV_PIX_FMT_RGBA,
                        yuv_w,yuv_h,
                        AV_PIX_FMT_YUV420P));

            int ret_height;
            CHECK_EXC(ret_height = yuv2rgba->scale(rgba,rgba_line_size,{},rgba_h,yuv,yuv_line_size));
            cerr << GET_STR(ret_height : ) << ret_height << "\n";
            ofs.write(reinterpret_cast<char *>(yuv[0]),yuv_w * yuv_h);
            ofs.write(reinterpret_cast<char *>(yuv[1]),yuv_w * yuv_h / 4);
            ofs.write(reinterpret_cast<char *>(yuv[2]),yuv_w * yuv_h / 4);
        }

        cerr << GET_STR(rgba to yuv420 complete\n\n);

    } catch (const exception &e) {
        cerr << e.what() << "\n";
        ret = -1;
    }

    ifs.close();
    ofs.close();
    return ret;
}
