#include <DeviceController/MuMuDeviceController.h>
#include <Cmd/CmdTools.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <Log/MiniLog.h>
#include <format>

constexpr std::string ar::MuMuDeviceController::getMuMuManagerPath(std::string path) {
    return path + "\\" + "shell" + "\\" + "MuMuManager.exe";
}

/**
 * @brief 这个是用来检查命令返回值是否合法的函数，通过检查返回值中是否包含"{"来判断
 * 
 * @return 如果返回值合法，返回AR_NO_ERROR，否则返回AR_INVALID_COMMAND
 */
ar::ARDeviceError ar::MuMuDeviceController::checkIsCommandValid(std::string cmd_res) {
    if (cmd_res.find("{") == std::string::npos) return ar::ARDeviceError::AR_INVALID_COMMAND;
    else return ar::ARDeviceError::AR_NO_ERROR;
}

ar::MuMuDeviceController::MuMuDeviceController(const std::string mumu_path) : mumu_path(mumu_path){}

ar::ARDeviceError ar::MuMuDeviceController::getDeviceInfo(const int& index, ar::deviceInfo& info){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {}", ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "info -v", index);
    ar::exec_cmd(cmd, cmd_res);
    // 防御性编程，多次重复执行判断虚拟机是否启动的命令
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d;
    d.Parse(cmd_res.c_str());
    info.error_code = d["error_code"].GetInt();
    if(info.error_code == -200) {
        ar::error("Invalid MuMu index: {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    info.is_empty = false;
    //get offline info
    info.is_process_started = false;
    info.index = index;
    //get online info
    if(d.HasMember("adb_host_ip")){
        info.is_process_started = true;
        info.adb_host_ip = d["adb_host_ip"].GetString();
        info.adb_port = d["adb_port"].GetUint();
    }
    return ar::ARDeviceError::AR_NO_ERROR;
}

/**
 * 通过执行mumu模拟器的info命令从而判断指定index的安卓虚拟机是否正常
 * 
 * @param index: todo 
 * 
 * @return 如果设备索引合法，返回AR_NO_ERROR，否则返回AR_INVALID_INDEX
 */
ar::ARDeviceError ar::MuMuDeviceController::checkIsDeviceIndexValid(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    bool state = false;
    // 接收命令执行的结果
    std::string cmd_res;
    // 应该是构建一个mumu模拟器的命令，用于获取模拟器信息，根据index信息，看看获取设备信息是否成功，从而判断index是否合法
    std::string cmd = std::format("{} {} {}", ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "info -v", index);
    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }

    // 这里进一步判断index是否合法，如果返回的json中包含error_code且值为0，则说明index合法
    rapidjson::Document d;
    d.Parse(cmd_res.c_str());
    if (d.HasMember("error_code") && d["error_code"] == 0) return ar::ARDeviceError::AR_NO_ERROR;
    return ar::ARDeviceError::AR_INVALID_INDEX;
}

/**
 * @brief 同样是构建info命令获取对应虚拟机是否启动，从命令返回的is_process_started标识判断
 * 
 * @param index: 指定index虚拟机的索引
 */
ar::ARDeviceError ar::MuMuDeviceController::checkDeviceState(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    bool state = false;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {}", ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "info -v", index);
    err = checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Index invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    
    // todo 可以优化，直接尝试解析json优化
    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d;
    d.Parse(cmd_res.c_str());
    if(d["error_code"].GetInt() == -200) {
        ar::error("Invalid MuMu index: {}", index);
        std::exit(0);
    }

    // 从info返回的标识去判断设备是否正常
    state = d["is_process_started"].GetBool();
    if (state == true) return ar::ARDeviceError::AR_NO_ERROR;
    return ar::ARDeviceError::AR_DEVICE_OFFLINE;
}

ar::ARDeviceError ar::MuMuDeviceController::lauchDevice(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    // 构建启动mumu模拟器的命令
    std::string cmd = std::format("{} {} {} {}",
        ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "launch");
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }

    // 确认mumu模拟器启动的报错，如果则会在命令行输出json块中的errCode不等于0
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }
    ar::info("Launch MuMu {}", index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

ar::ARDeviceError ar::MuMuDeviceController::shutdownDevice(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "shutdown");
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }

    err = ar::MuMuDeviceController::checkDeviceState(index); 
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("MuMu index : {} offline !", index);
        return ar::ARDeviceError::AR_DEVICE_OFFLINE;
    }

    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }
    ar::info("Shutdown MuMu {}", index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

ar::ARDeviceError ar::MuMuDeviceController::restartDevice(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "restart");
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }
    ar::info("Restart MuMu {}", index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

ar::ARDeviceError ar::MuMuDeviceController::showWindow(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "showWindow");
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    err = ar::MuMuDeviceController::checkDeviceState(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("MuMu index : {} offline !", index);
        return ar::ARDeviceError::AR_DEVICE_OFFLINE;
    }
    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }
    ar::info("Show MuMu {}", index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

/**
 * 同样是利用mumu模拟器的控制参数，去隐藏窗口
 * 构建命令执行
 */
ar::ARDeviceError ar::MuMuDeviceController::hideWindow(const int& index){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "hideWindow");
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    err = ar::MuMuDeviceController::checkDeviceState(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("MuMu index : {} offline !", index);
        return ar::ARDeviceError::AR_DEVICE_OFFLINE;
    }
    ar::exec_cmd(cmd, cmd_res);
    // todo 这里可以使用rapidjson::Document直接尝试解析去优化代码，而不是提前判断是否是合法的jsonge是
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }
    ar::info("Hide MuMu {}", index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

/**
 * 启动指定包名的APP， 模拟器有一个launch命令
 * 
 * @param index: 指定启动的模拟器索引，因为有可能mumu模拟器可能是一个虚拟机管理其，会启动多个虚拟机，通过inde下去选择指定的安卓虚拟机启动应用
 * @param app_name: 指定包名
 */
ar::ARDeviceError ar::MuMuDeviceController::lauchApp(const int& index, const std::string& app_name){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    // 构建启动app的命令
    std::string cmd = std::format("{} {} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "app launch -pkg", app_name);
    // 判断指定的虚拟机是否正常
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    // 判断虚拟机是否启动，但是从代码来看可以和上一步checkIsDeviceIndexValid中进行合并
    err = ar::MuMuDeviceController::checkDeviceState(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("MuMu index : {} offline !", index);
        return ar::ARDeviceError::AR_DEVICE_OFFLINE;
    }
    ar::exec_cmd(cmd, cmd_res);
    //warning ! checkIsCommandValid will not check app_name ! 
    // 这里仅判断返回值有没有大括号
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }

    // 如果执行命令的返回值没有任何错误，则返回启动成功
    ar::info("Launch {} in MuMu {}", app_name, index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

ar::ARDeviceError ar::MuMuDeviceController::closeApp(const int& index, const std::string& app_name){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    std::string cmd = std::format("{} {} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "app close -pkg", app_name);
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    err = ar::MuMuDeviceController::checkDeviceState(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("MuMu index : {} offline !", index);
        return ar::ARDeviceError::AR_DEVICE_OFFLINE;
    }
    ar::exec_cmd(cmd, cmd_res);
    //warning ! checkIsCommandValid will not check app_name !
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d; d.Parse(cmd_res.c_str());
    if (d["errcode"].GetInt() != 0) {
        ar::error("Unknown error in MuMu {} !", index);
        return ar::ARDeviceError::AR_UNKNOWN_ERROR;
    }
    ar::info("Close {} in MuMu {}", app_name, index);
    return ar::ARDeviceError::AR_NO_ERROR;
}

/**
 * 依旧是通过mumu模拟器的命令，获取指定app的状态，从而判断app是否启动成功
 * 
 * @param index: 指定虚拟机的序号
 * @param app_name: 指定获取app信息的包名
 * @param info: 获取返回的app信息
 */
ar::ARDeviceError ar::MuMuDeviceController::getAppState(const int& index, const std::string& app_name, appInfo& info){
    ar::ARDeviceError err = ar::ARDeviceError::AR_NO_ERROR;
    std::string cmd_res;
    // 构建获取app信息的命令
    std::string cmd = std::format("{} {} {} {} {}",
    ar::MuMuDeviceController::getMuMuManagerPath(mumu_path), "control -v", index, "app info -pkg", app_name);
    err = ar::MuMuDeviceController::checkIsDeviceIndexValid(index);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Invalid MuMu index : {}", index);
        return ar::ARDeviceError::AR_INVALID_INDEX;
    }
    // 判断指定 的虚拟机是否启动成功
    err = ar::MuMuDeviceController::checkDeviceState(index); 
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("MuMu index : {} offline !", index);
        return ar::ARDeviceError::AR_DEVICE_OFFLINE;
    }

    // 执行命令，获取对应app的状态，从返回值的state标识中，通过标识确认
    // app是否安装、是否启动
    ar::exec_cmd(cmd, cmd_res);
    err = ar::MuMuDeviceController::checkIsCommandValid(cmd_res);
    if (err != ar::ARDeviceError::AR_NO_ERROR) {
        ar::error("Cmd invalid !");
        return ar::ARDeviceError::AR_INVALID_COMMAND;
    }
    rapidjson::Document d;
    d.Parse(cmd_res.c_str());
    std::string state_res = d["state"].GetString();
    if(state_res == "running") {info.is_install = true; info.is_running = true;}
    if(state_res == "stopped") {info.is_install = true; info.is_running = false;}
    if(state_res == "not_installed") {info.is_install = false; info.is_running = false;}
    return ar::ARDeviceError::AR_NO_ERROR;
}