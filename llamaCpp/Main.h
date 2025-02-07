//
// Created by pwipo on 08.11.2019.
//

#ifndef SMCMODULES_MAIN_H
#define SMCMODULES_MAIN_H

#include "SMCApi.h"
#include <iostream>
#include <sstream>
#include "llama.h"

using namespace SMCApi;

inline wchar_t separator() {
#ifdef _WIN32
    return L'\\';
#else
    return L'/';
#endif
}

class MainCls : public IMethod {
    std::wstring modelPath;
    float temperature;
    float minP;
    int contextSize;
    int ngl;

    llama_model* model;
    llama_context* ctx;
    llama_vocab* vocab;
    llama_sampler* _sampler;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converterTo;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converterFrom;

public:
    MainCls();

    void start(IConfigurationTool* tool, IValueFactory* factory) override;

    void process(IConfigurationTool* tool, IExecutionContextTool* contextTool, IValueFactory* factory) override;

    void update(IConfigurationTool* tool, IValueFactory* factory) override;

    void stop(IConfigurationTool* tool, IValueFactory* factory) override;

    void addChatMessage(std::vector<llama_chat_message>& messages, const std::string& message, const std::string& role);

    std::string generate(IConfigurationTool* tool, const std::string& prompt);

    ~MainCls();
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
__declspec(dllexport) IMethod* getInstance();
#else
__attribute__((visibility("default"))) IMethod *getInstance();
#endif

#ifdef __cplusplus
}

#endif

#endif //SMCMODULES_MAIN_H
