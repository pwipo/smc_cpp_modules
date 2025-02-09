//
// Created by pwipo on 08.11.2019.
//

#include <codecvt>
#include "Main.h"
#include <locale>
#include <memory>

void addChatMessage(std::vector<llama_chat_message>& messages, const std::string& message, const std::string& role) {
    messages.push_back({_strdup(role.c_str()), _strdup(message.c_str())});
}

void MainCls::start(IConfigurationTool* tool, IValueFactory* factory) {
    // tool->loggerTrace(L"start");
    // tool->loggerTrace(L"start get settings");
    modelPath = *tool->getConfiguration()->getSetting(L"modelPath")->getValueString();
    temperature = tool->getConfiguration()->getSetting(L"temperature")->getValueNumber()->floatValue();
    minP = tool->getConfiguration()->getSetting(L"minP")->getValueNumber()->floatValue();
    contextSize = tool->getConfiguration()->getSetting(L"contextSize")->getValueNumber()->intValue();
    ngl = tool->getConfiguration()->getSetting(L"ngl")->getValueNumber()->intValue();
    nBatch = tool->getConfiguration()->getSetting(L"nBatch")->getValueNumber()->intValue();
    // tool->loggerTrace(L"stop get settings");

    // tool->loggerTrace(L"initialize the model");
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = ngl;

    model = nullptr;
    if (modelPath.size() > 2) {
        std::wstring modelPathFull = tool->getWorkDirectory() + separator() + modelPath;
        tool->loggerTrace(L"init model " + modelPathFull);
        std::string modelPathFullStr = converterTo.to_bytes(modelPathFull);
        model = llama_model_load_from_file(modelPathFullStr.c_str(), model_params);
    }
    if (!model) {
        tool->loggerError(L"error: unable to load model");
        std::string errMsg = __func__;
        errMsg += ": error: unable to load model";
        throw ModuleException(converterFrom.from_bytes(errMsg));
    }

    // tool->loggerTrace(L"get vocab");
    vocab = (llama_vocab*)(void*)llama_model_get_vocab(model);

    // tool->loggerTrace(L"initialize the context");
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = contextSize;
    ctx_params.n_batch = nBatch;

    ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        std::string errMsg = __func__;
        errMsg += ": error: failed to create the llama_context";
        throw ModuleException(converterFrom.from_bytes(errMsg));
    }

    // tool->loggerTrace(L"initialize the sampler");
    sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler, llama_sampler_init_min_p(minP, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(temperature));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
}

void MainCls::process(IConfigurationTool* configurationTool, IExecutionContextTool* executionContextTool, IValueFactory* factory) {
    // configurationTool->loggerTrace(L"start process");
    try {
        for (long i = 0; i < executionContextTool->getExecutionContext()->countSource(); ++i) {
            auto actions = executionContextTool->getMessages(i);
            for (IAction* action : *actions) {
                std::vector<llama_chat_message> messages;
                std::vector<char> formatted(llama_n_ctx(ctx));
                int prev_len = 0;
                // configurationTool->loggerTrace(L"start new processing, count messages=" + std::to_wstring(action->getMessages()->size()));
                for (IMessage* message : *action->getMessages()) {
                    if (message->getType() == VT_STRING) {
                        addChatMessage(messages, converterTo.to_bytes(*message->getValueString()), "user");
                    }
                    else if (message->getType() == VT_OBJECT_ARRAY) {
                        ObjectArray* objectArray = message->getValueObjectArray();
                        if (objectArray->getType() != OT_OBJECT_ELEMENT || objectArray->size() == 0)
                            continue;
                        for (int j = 0; j < objectArray->size(); j++) {
                            auto* o = (ObjectElement*)objectArray->get(j);
                            auto fRole = o->findField(L"role");
                            auto fContent = o->findField(L"content");
                            if (fRole && fRole->getType() == OT_STRING && fContent && fContent->getType() == OT_STRING)
                                addChatMessage(messages, converterTo.to_bytes(*fContent->getValueString()), converterTo.to_bytes(*fRole->getValueString()));
                        }
                    }
                }
                const char* tmpl = llama_model_chat_template(model, /* name */ nullptr);
                int new_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, formatted.data(), formatted.size());
                if (new_len > (int)formatted.size()) {
                    formatted.resize(new_len);
                    new_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, formatted.data(), formatted.size());
                }
                if (new_len < 0)
                    throw ModuleException(L"failed to apply the chat template");
                // configurationTool->loggerTrace(L"new_len=" + std::to_wstring(new_len));
                // remove previous messages to obtain the prompt to generate the response
                std::string prompt(formatted.begin() + prev_len, formatted.begin() + new_len);

                for (llama_chat_message& message : messages) {
                    delete message.content;
                    delete message.role;
                }
                messages.clear();
                // configurationTool->loggerTrace(L"get request: " + converterFrom.from_bytes(prompt));

                std::string response = generate(configurationTool, prompt);

                // configurationTool->loggerTrace(L"start create response");
                ObjectArray oa(OT_OBJECT_ELEMENT);
                auto* oe = new ObjectElement();
                // oe->getFields()->push_back(new ObjectField(L"value", new Number(100L)));
                oe->getFields()->push_back(new ObjectField(L"role", new std::wstring(L"assistant")));
                oe->getFields()->push_back(new ObjectField(L"content", new std::wstring(converterFrom.from_bytes(response))));
                oa.add(oe);
                executionContextTool->addMessage(factory->createData(&oa));
                // configurationTool->loggerTrace(L"send response: " + respnseW);
                // executionContextTool->addMessage(factory->createData(respnseW));
            }
        }
    }
    catch (ModuleException& e) {
        executionContextTool->addError(factory->createData(e.getMessage()));
    }
    // configurationTool->loggerTrace(L"stop process");
}

void MainCls::update(IConfigurationTool* tool, IValueFactory* factory) {
    //std::wcout << "update " << tool->getName() << std::endl;
    stop(tool, factory);
    start(tool, factory);
}

void MainCls::stop(IConfigurationTool* tool, IValueFactory* factory) {
    //std::wcout << "stop " << tool->getName() << std::endl;
    if (ctx)
        llama_kv_cache_clear(ctx);
    if (sampler) {
        llama_sampler_free(sampler);
        sampler = nullptr;
    }
    if (ctx) {
        llama_free(ctx);
        ctx = nullptr;
    }
    if (model) {
        llama_model_free(model);
        model = nullptr;
    }
    if (vocab)
        vocab = nullptr;

    delete this;
}

std::string MainCls::generate(IConfigurationTool* tool, const std::string& prompt) {
    // tool->loggerTrace(L"start generate");
    std::string response;

    const bool is_first = llama_get_kv_cache_used_cells(ctx) == 0;

    // tokenize the prompt
    const int n_prompt_tokens = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, is_first, true);
    std::vector<llama_token> prompt_tokens(n_prompt_tokens);
    if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), is_first, true) < 0)
        throw ModuleException(L"failed to tokenize the prompt");

    // tool->loggerTrace(L"prepare a batch for the prompt");
    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    llama_token new_token_id;
    while (true) {
        // check if we have enough space in the context to evaluate this batch
        unsigned long n_ctx = llama_n_ctx(ctx);
        unsigned long n_batch = llama_n_batch(ctx);
        int n_ctx_used = llama_get_kv_cache_used_cells(ctx);
        if (n_ctx_used + batch.n_tokens > n_ctx || batch.n_tokens > n_batch) {
            tool->loggerWarn(
                L"context size exceeded: n_ctx=" + std::to_wstring(n_ctx) + L" n_tokens=" + std::to_wstring(batch.n_tokens) + L" n_prompt_tokens=" +
                std::to_wstring(n_prompt_tokens) + L" n_batch=" + std::to_wstring(n_batch) + L" n_ctx_used=" + std::to_wstring(n_ctx_used));
            return response;
        }

        if (llama_decode(ctx, batch))
            throw ModuleException(L"failed to decode");

        // tool->loggerTrace(L"sample the next token");
        new_token_id = llama_sampler_sample(sampler, ctx, -1);

        // tool->loggerTrace(L"is it an end of generation?");
        if (llama_vocab_is_eog(vocab, new_token_id))
            break;

        // convert the token to a string, print it and add it to the response
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n < 0)
            throw ModuleException(L"failed to convert token to piece");

        std::string piece(buf, n);
        // printf("%s", piece.c_str());
        // fflush(stdout);
        response += piece;

        // tool->loggerTrace(L"prepare the next batch with the sampled token");
        batch = llama_batch_get_one(&new_token_id, 1);
    }

    // tool->loggerTrace(L"stop generate");
    return response;
}

MainCls::~MainCls() {
}

MainCls::MainCls() {
    // modelPath = nullptr;
    temperature = 0.;
    minP = 0.;
    contextSize = 0;
    ngl = 0;
    nBatch = 0;

    model = nullptr;
    // ctx = nullptr;
    // vocab = nullptr;
    // sampler = nullptr;

    // only print errors
    llama_log_set([](enum ggml_log_level level, const char* text, void* /* user_data */) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
            fprintf(stderr, "%s", text);
        }
    }, nullptr);

    // load dynamic backends
    // ggml_backend_load_all();
}

IMethod* getInstance() {
    return new MainCls();
}
