#include <Controller/Controller.h>
#include <Controller/MuMuController.h>
#include <Controller/AdbController.h>

/**
 * 根据控制器的参数和控制器的类型，创建对应的控制器对象
 * 
 * 如果创建成功则返回
 */
std::unique_ptr<ar::Controller> ar::ControllerFactory::createController(ar::controllerParams& params, ar::controllerType type){
    switch(type){
        case(ar::controllerType::MUMU) :
            return std::make_unique<ar::MuMuController>(params.adb_path, params.adb_port, params.mumu_path, params.mumu_index);
        case(ar::controllerType::ADB) :
            return std::make_unique<ar::AdbController>(params.adb_path, params.adb_port);
    }
    //add other controller maybe ...
    return nullptr;
}