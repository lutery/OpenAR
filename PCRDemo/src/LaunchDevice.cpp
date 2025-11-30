#include <LaunchDevice.h>
#include <windows.h>
#include <Log/MiniLog.h>

/**
 * @brief 在当前的函数中，主要执行的是先启动对应的虚拟机器设备，然后在启动对应的app应用，主要都是通过命令的方式执行
 * 
 * @param params: 主要使用内部存储的虚拟机索引index参数、启动等待时间参数、设备对象（主要用于执行命令）进行虚拟机的启动和app的启动
 */
void pcr::launchDevice(pcr::global_params& params){
    ar::info("Launch MuMu {}", params.index);
    ar::appInfo info;
    do{
        // todo 这里真的是在利用循环不断的判断模拟器是否启动吗？因为一旦返回的不是NO ERROR 则直接退出了？
        Sleep(params.operate_duration);
        if(params.device_controller->lauchDevice(params.index)!= ar::ARDeviceError::AR_NO_ERROR)std::exit(0);
    }while(params.device_controller->checkDeviceState(params.index) != ar::ARDeviceError::AR_NO_ERROR);
    //是否静默运行，静默运行会将模拟器隐藏
    if(params.start_with_silence) params.device_controller->hideWindow(params.index);
    
    do{
        // 等待模拟器启动成功
        Sleep(params.operate_duration);
        if (params.device_controller->lauchApp(params.index, "com.bilibili.priconne") != ar::ARDeviceError::AR_NO_ERROR)std::exit(0);
        // 如果启动成功，则获取对应app的启动信息
        params.device_controller->getAppState(params.index, "com.bilibili.priconne", info);
    }while(!info.is_running); // 从获取的app启动信息里面，获取启动状态，如果没有已启动则继续尝试启动APP？ todo 每次循环都尝试相同的启动命令？感觉有问题
}