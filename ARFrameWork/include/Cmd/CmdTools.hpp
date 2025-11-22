#pragma once
#include <string>
#include <array>
#include <memory>
namespace ar{

    /**
     * @brief
     * 
     * @param cmd: 要执行的命令
     * @param cmd_res: 命令的执行结果
     */
inline void exec_cmd(const std::string& cmd, std::string& cmd_res){
    // 创建一个固定大小的缓冲区，作用类似C数组，性能和普通数组一致
    std::array<char,128> buffer;

    // _popen：执行 shell 命令并返回文件句柄，可以从中读取命令的输出。其中的r表示需要获取命令的返回值
    // _pclose是一个关闭_popen创建的对象的方法，在这里是传入自定义的删除对象的方法，这里需要使用_pclose函数而不是delete
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    // 由于返回的是一个FILE对象，所以可以使用fgets获取命令行的返回值
    while(fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr) {
        // 拼接返回值，这里估计是一个重载方法，会自动的将C字符串的风格转换为string
        cmd_res += buffer.data();
    }
}

}