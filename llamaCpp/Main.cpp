//
// Created by pwipo on 08.11.2019.
//

#include "Main.h"

#ifndef _strdup
char* _strdup(const char* c) {
    if (c == nullptr)
        return nullptr;
    // size_t n = strlen(c);
    // char* buffer = (char*)calloc(n + 1, sizeof(char));
    // if (buffer != nullptr)
    //     strncpy(buffer, c, n);
    char* buffer = (char*)malloc(strlen(c) + 1);
    if (buffer != nullptr)
        strcpy(buffer, c);
    return buffer;
}
#endif

void addChatMessage(std::vector<llama_chat_message>& messages, const std::string& message, const std::string& role) {
    messages.push_back({_strdup(role.c_str()), _strdup(message.c_str())});
}

void cleanHolder(LlamaContextHolder* holder) {
    if (!holder)
        return;
    holder->messages.clear();
    holder->prev_len = 0;
    if (holder->ctx)
        llama_kv_cache_clear(holder->ctx);
}

// bool is_file_exist(const char* fileName) {
//     std::ifstream infile(fileName);
//     return infile.good();
// }

inline bool is_file_exist_2(const std::string& name) {
    if (FILE* file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}

void MainCls::start(IConfigurationTool* tool, IValueFactory* factory) {
    modelPath = *tool->getConfiguration()->getSetting(L"modelPath")->getValueString();
    temperature = tool->getConfiguration()->getSetting(L"temperature")->getValueNumber()->floatValue();
    minP = tool->getConfiguration()->getSetting(L"minP")->getValueNumber()->floatValue();
    contextSize = tool->getConfiguration()->getSetting(L"contextSize")->getValueNumber()->intValue();
    ngl = tool->getConfiguration()->getSetting(L"ngl")->getValueNumber()->intValue();
    nBatch = tool->getConfiguration()->getSetting(L"nBatch")->getValueNumber()->intValue();
    nThreds = tool->getConfiguration()->getSetting(L"nThreds")->getValueNumber()->intValue();
    flashAttn = tool->getConfiguration()->getSetting(L"flashAttn")->getValueBoolean();
    gpu_split_mode = tool->getConfiguration()->getSetting(L"gpu_split_mode")->getValueNumber()->intValue();
    main_gpu = tool->getConfiguration()->getSetting(L"main_gpu")->getValueNumber()->intValue();

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = ngl;
    model_params.split_mode = static_cast<llama_split_mode>(gpu_split_mode);
    model_params.main_gpu = main_gpu;

    model = nullptr;
    if (modelPath.size() > 2) {
        std::wstring modelPathFull = tool->getWorkDirectory() + separator() + modelPath;
        tool->loggerTrace(L"init model " + modelPathFull);
        std::string modelPathFullStr = converterTo.to_bytes(modelPathFull);
        if (is_file_exist_2(modelPathFullStr.c_str()))
            model = llama_model_load_from_file(modelPathFullStr.c_str(), model_params);
    }
    if (!model) {
        tool->loggerError(L"error: unable to load model");
        std::string errMsg = __func__;
        errMsg += ": error: unable to load model";
        throw ModuleException(converterFrom.from_bytes(errMsg));
    }
    if (nThreds <= 0)
        nThreds = std::thread::hardware_concurrency();

    vocab = (llama_vocab*)(void*)llama_model_get_vocab(model);

    sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler, llama_sampler_init_min_p(minP, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(temperature));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
}

LlamaContextHolder* MainCls::addOrCreateContextHolder(int ctxId, bool init) {
    if (ctxId < 0)
        throw ModuleException(L"invalid context id");
    auto pos = llamaContextHolders.find(ctxId);
    LlamaContextHolder* holder = nullptr;
    if (pos != llamaContextHolders.end())
        holder = pos->second;
    if (holder != nullptr && (holder->ctx != nullptr || !init))
        return holder;

    // tool->loggerTrace(L"initialize the context");
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = contextSize;
    ctx_params.n_batch = nBatch;
    ctx_params.n_threads = nThreds;
    ctx_params.n_threads_batch = nThreds;
    ctx_params.flash_attn = flashAttn;

    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        std::string errMsg = __func__;
        errMsg += ": error: failed to create the llama_context";
        throw ModuleException(converterFrom.from_bytes(errMsg));
    }
    if (holder != nullptr && holder->ctx == nullptr)
        holder->ctx = ctx;

    if (holder == nullptr) {
        holder = new LlamaContextHolder{ctx, 0};
        llamaContextHolders.insert(std::pair<int, LlamaContextHolder*>(ctxId, holder));
    }
    return holder;
}

void MainCls::process(IConfigurationTool* configurationTool, IExecutionContextTool* executionContextTool, IValueFactory* factory) {
    try {
        if (executionContextTool->getExecutionContext()->getType() == L"default" || executionContextTool->getExecutionContext()->getType() == L"talk") {
            for (long i = 0; i < executionContextTool->getExecutionContext()->countSource(); ++i) {
                auto actions = executionContextTool->getMessages(i);
                for (IAction* action : *actions) {
                    auto messages = action->getMessages();
                    if (messages && !messages->empty()) {
                        auto message = messages->front();
                        // messages->erase(messages->begin());
                        int ctxId = 0;
                        bool removeFirst = false;
                        if (message->getType() != VT_STRING && message->getType() != VT_OBJECT_ARRAY && message->getType() != VT_BYTES && message->getType() != VT_BOOLEAN) {
                            ctxId = message->getValueNumber()->intValue();
                            removeFirst = true;
                        }
                        // std::vector<IMessage*> newVec(removeFirst ? messages->begin() + 1 : messages->begin(), messages->end());
                        auto holder = addOrCreateContextHolder(ctxId, true);
                        if (executionContextTool->getExecutionContext()->getType() == L"talk") {
                            holder->messages.clear();
                            holder->prev_len = 0;
                        }
                        talk(configurationTool, executionContextTool, factory, holder, *messages, removeFirst ? 1 : 0);
                    }
                }
            }
        }
        else if (executionContextTool->getExecutionContext()->getType() == L"clean") {
            int ctxId = 0;
            if (executionContextTool->getExecutionContext()->countSource() > 0) {
                auto actions = executionContextTool->getMessages(0);
                if (actions && !actions->empty()) {
                    auto messages = actions->at(actions->size() - 1)->getMessages();
                    if (messages && !messages->empty()) {
                        auto message = messages->at(0);
                        if (message->getType() != VT_STRING && message->getType() != VT_OBJECT_ARRAY && message->getType() != VT_BYTES && message->getType() != VT_BOOLEAN)
                            ctxId = message->getValueNumber()->intValue();
                    }
                }
            }
            auto holder = addOrCreateContextHolder(ctxId, false);
            cleanHolder(holder);
        }
        else if (executionContextTool->getExecutionContext()->getType() == L"remove") {
            int ctxId = 0;
            if (executionContextTool->getExecutionContext()->countSource() > 0) {
                auto actions = executionContextTool->getMessages(0);
                if (actions && !actions->empty()) {
                    auto messages = actions->at(actions->size() - 1)->getMessages();
                    if (messages && !messages->empty()) {
                        auto message = messages->at(0);
                        if (message->getType() != VT_STRING && message->getType() != VT_OBJECT_ARRAY && message->getType() != VT_BYTES && message->getType() != VT_BOOLEAN)
                            ctxId = message->getValueNumber()->intValue();
                    }
                }
            }
            auto holder = addOrCreateContextHolder(ctxId, false);
            cleanHolder(holder);
            if (holder->ctx) {
                llama_free(holder->ctx);
                holder->ctx = nullptr;
            }
            delete holder;
            llamaContextHolders.erase(ctxId);
        }
    }
    catch (ModuleException& e) {
        executionContextTool->addError(factory->createData(e.getMessage()));
    }
}

void MainCls::update(IConfigurationTool* tool, IValueFactory* factory) {
    stop(tool, factory);
    start(tool, factory);
}

void MainCls::stop(IConfigurationTool* tool, IValueFactory* factory) {
    // for (int i = 0; llamaContextHolders.size(); i++) {
    for (auto it = llamaContextHolders.begin(); it != llamaContextHolders.end(); ++it) {
        // auto holder = llamaContextHolders[i];
        auto holder = it->second;
        cleanHolder(holder);
        if (holder->ctx) {
            llama_free(holder->ctx);
            holder->ctx = nullptr;
        }
        delete holder;
    }
    llamaContextHolders.clear();
    if (sampler) {
        llama_sampler_free(sampler);
        sampler = nullptr;
    }
    if (model) {
        llama_model_free(model);
        model = nullptr;
    }
    if (vocab)
        vocab = nullptr;

    delete this;
}

void MainCls::talk(IConfigurationTool* configurationTool, IExecutionContextTool* executionContextTool, IValueFactory* factory,
                   LlamaContextHolder* holder, std::vector<IMessage*>& messageLst, int start) {
    if (messageLst.empty() || start >= messageLst.size())
        return;
    std::vector<llama_chat_message> messages;
    std::vector<char> formatted(llama_n_ctx(holder->ctx));
    std::wstring* pResponseContent = nullptr;
    try {
        // configurationTool->loggerTrace(L"start new processing, count messages=" + std::to_wstring(action->getMessages()->size()));
        // for (IMessage* message : messageLst) {
        for (int i = start; i < messageLst.size(); i++) {
            IMessage* message = messageLst[i];
            if (message->getType() == VT_STRING) {
                std::string contentStr = converterTo.to_bytes(*message->getValueString());
                addChatMessage(messages, contentStr, "user");
            }
            else if (message->getType() == VT_OBJECT_ARRAY) {
                ObjectArray* objectArray = message->getValueObjectArray();
                if (objectArray->getType() != OT_OBJECT_ELEMENT || objectArray->size() == 0)
                    continue;
                for (int j = 0; j < objectArray->size(); j++) {
                    auto* o = (ObjectElement*)objectArray->get(j);
                    auto fRole = o->findField(L"role");
                    auto fContent = o->findField(L"content");
                    if (fRole && fRole->getType() == OT_STRING && fContent && fContent->getType() == OT_STRING) {
                        std::string contentStr = converterTo.to_bytes(*fContent->getValueString());
                        std::string roleStr = converterTo.to_bytes(*fRole->getValueString());
                        addChatMessage(messages, contentStr, roleStr);
                    }
                }
            }
        }
        bool isSameReq = false;
        if (messages.size() >= holder->messages.size()) {
            isSameReq = true;
            for (int i = 0; i < holder->messages.size(); ++i) {
                std::string str = messages[i].content;
                if (holder->messages[i] != str) {
                    isSameReq = false;
                    break;
                }
            }
        }
        if (!isSameReq) {
            configurationTool->loggerDebug(L"it was determined that this is a new request " + std::to_wstring(holder->prev_len));
            cleanHolder(holder);
        }

        // configurationTool->loggerTrace(L"messages size=" + std::to_wstring(messages.size()));
        const char* tmpl = llama_model_chat_template(model, /* name */ nullptr);
        int new_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, formatted.data(), formatted.size());
        if (new_len > (int)formatted.size()) {
            formatted.resize(new_len);
            new_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, formatted.data(), formatted.size());
        }
        if (new_len < 0)
            throw ModuleException(L"failed to apply the chat template");
        if (new_len < holder->prev_len) {
            configurationTool->loggerDebug(L"detect new messages, clean context cache " + std::to_wstring(new_len) + L" " + std::to_wstring(holder->prev_len));
            cleanHolder(holder);
        }

        holder->messages.clear();
        for (auto& m : messages)
            holder->messages.push_back(m.content);

        if (holder->prev_len == new_len)
            return;

        std::string prompt(formatted.begin() + holder->prev_len, formatted.begin() + new_len);
        // configurationTool->loggerTrace(L"get request: " + converterFrom.from_bytes(prompt));
        std::string response = generate(configurationTool, holder, prompt);

        holder->prev_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), false, nullptr, 0);
        if (holder->prev_len < 0)
            throw ModuleException(L"failed to apply the chat template");
        pResponseContent = new std::wstring(converterFrom.from_bytes(response));
    }
    catch (std::exception& ex) {
        for (llama_chat_message& message : messages) {
            delete message.content;
            delete message.role;
        }
        messages.clear();
        throw;
    }

    if (pResponseContent) {
        ObjectArray oa(OT_OBJECT_ELEMENT);
        auto* oe = new ObjectElement();
        oe->getFields()->push_back(new ObjectField(L"role", new std::wstring(L"assistant")));
        oe->getFields()->push_back(new ObjectField(L"content", pResponseContent));
        oa.add(oe);
        executionContextTool->addMessage(factory->createData(&oa));
    }
}

std::string MainCls::generate(IConfigurationTool* tool, LlamaContextHolder* holder, const std::string& prompt) {
    // tool->loggerTrace(L"start generate for prompt: " + std::to_wstring(prompt.size()));
    std::string response;

    const bool is_first = llama_get_kv_cache_used_cells(holder->ctx) == 0;

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
        unsigned long n_ctx = llama_n_ctx(holder->ctx);
        unsigned long n_batch = llama_n_batch(holder->ctx);
        int n_ctx_used = llama_get_kv_cache_used_cells(holder->ctx);
        if (n_ctx_used + batch.n_tokens > n_ctx || batch.n_tokens > n_batch) {
            tool->loggerWarn(
                L"context size exceeded: n_ctx=" + std::to_wstring(n_ctx) + L" n_tokens=" + std::to_wstring(batch.n_tokens) + L" n_prompt_tokens=" +
                std::to_wstring(n_prompt_tokens) + L" n_batch=" + std::to_wstring(n_batch) + L" n_ctx_used=" + std::to_wstring(n_ctx_used));
            return response;
        }

        // tool->loggerTrace(L"run the model");
        if (llama_decode(holder->ctx, batch))
            throw ModuleException(L"failed to decode");

        // tool->loggerTrace(L"sample the next token");
        new_token_id = llama_sampler_sample(sampler, holder->ctx, -1);

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
    nThreds = 0;
    flashAttn = false;
    gpu_split_mode = 0;
    main_gpu = 0;

    sampler = nullptr;
    vocab = nullptr;
    model = nullptr;
    // ctx = nullptr;

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
