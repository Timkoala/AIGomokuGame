#include <QApplication>
#include "mainwindow.h"

/**
 * @brief 程序入口点
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码
 */
int main(int argc, char *argv[]) {
    // 创建Qt应用程序对象
    QApplication app(argc, argv);
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    // 进入应用程序主事件循环
    return app.exec();
} 