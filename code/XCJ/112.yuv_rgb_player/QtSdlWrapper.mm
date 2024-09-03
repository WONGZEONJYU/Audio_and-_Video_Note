#include <Cocoa/Cocoa.h>

extern "C" void setMetalViewFrame(void *winId, int x,int y,
                                  int width, int height) {
    NSView *metalView = (__bridge NSView *)(winId);
    [metalView setFrame:NSMakeRect(x, y,width,height)];

}

// 封装的桥接函数，用于从 C++ 中调用
extern "C" void addMetalViewToParent(void *parentWinId, void *metalWinId) {
    NSView *parentView = (__bridge NSView *)(parentWinId);
    NSView *metalView = (__bridge NSView *)(metalWinId);

    // 将 metalView 添加到 parentView，位置在其他视图之下
    [parentView addSubview:metalView positioned:NSWindowBelow relativeTo:nil];
}
