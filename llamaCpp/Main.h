//
// Created by pwipo on 08.11.2019.
//

#ifndef SMCMODULES_MAIN_H
#define SMCMODULES_MAIN_H

#include "SMCApi.h"
#include <iostream>
#include <sstream>
#include "llama.h"
#include <codecvt>
#include <locale>
#include <memory>
#include <string>
#include <cstring>
#include <thread>
#include <map>

using namespace SMCApi;

inline wchar_t separator() {
#ifdef _WIN32
    return L'\\';
#else
    return L'/';
#endif
}

struct LlamaContextHolder {
    llama_context* ctx;
    int prev_len;
};

class MainCls : public IMethod {
    std::wstring modelPath;
    float temperature;
    float minP;
    int contextSize;
    int ngl;
    int nBatch;
    int nThreds;
    bool flashAttn;
    int gpu_split_mode;
    int main_gpu;

    llama_model* model;
    llama_vocab* vocab;
    llama_sampler* sampler;
    std::map<int, LlamaContextHolder*> llamaContextHolders;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converterTo;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converterFrom;

    LlamaContextHolder* MainCls::addOrCreateContextHolder(int ctxId);
    void MainCls::talk(IConfigurationTool* configurationTool, IExecutionContextTool* executionContextTool, IValueFactory* factory, LlamaContextHolder* holder,
                       std::vector<IMessage*>& messageLst);
    std::string generate(IConfigurationTool* tool, LlamaContextHolder* holder, const std::string& prompt);

public:
    MainCls();

    void start(IConfigurationTool* tool, IValueFactory* factory) override;

    void process(IConfigurationTool* tool, IExecutionContextTool* contextTool, IValueFactory* factory) override;

    void update(IConfigurationTool* tool, IValueFactory* factory) override;

    void stop(IConfigurationTool* tool, IValueFactory* factory) override;

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
