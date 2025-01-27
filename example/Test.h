//
// Created by pwipo on 08.11.2019.
//

#ifndef SMCMODULES_TEST_H
#define SMCMODULES_TEST_H

#include "SMCApi.h"
#include <iostream>
#include <sstream>

using namespace SMCApi;

class Test : public IMethod {
    long counter;
    std::wstring value;
    std::wstring param;
    std::wstring fileTextValue;
public:
    Test();

    void start(IConfigurationTool *tool, IValueFactory *factory) override;

    void process(IConfigurationTool *tool, IExecutionContextTool *contextTool, IValueFactory *factory) override;

    void update(IConfigurationTool *tool, IValueFactory *factory) override;

    void stop(IConfigurationTool *tool, IValueFactory *factory) override;

    ~Test();
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
__declspec(dllexport) IMethod *getInstance();
#else
__attribute__((visibility("default"))) IMethod *getInstance();
#endif

#ifdef __cplusplus
}

#endif

#endif //SMCMODULES_TEST_H
