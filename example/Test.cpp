//
// Created by pwipo on 08.11.2019.
//

#include <codecvt>
#include "Test.h"
#include <locale>
#include <memory>

void Test::start(IConfigurationTool *tool, IValueFactory *factory) {
    // get settings
    //std::cout << "Test START" << std::endl;
    value = *tool->getConfiguration()->getSetting(L"value")->getValueString();
    param = *tool->getConfiguration()->getSetting(L"param")->getValueString();
    counter = 1;
    //std::cout << "Test END" << std::endl;

    // reed file from home folder
    fileTextValue = L"";
    for (IFileTool *file: *(tool->getHomeFolder()->getChildrens())) {
        if (file->getName() == L"text.txt") {
            std::string t_str = file->getData();
            /*
            //left trim
            t_str.erase(t_str.begin(), std::find_if(t_str.begin(), t_str.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            */
            //right trim
            t_str.erase(std::find_if(t_str.rbegin(), t_str.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), t_str.end());
            //std::stringstream trimmer;
            //trimmer << t_str;
            //fileTextValue.clear();
            //trimmer >> t_str;
            typedef std::codecvt_utf8<wchar_t> convert_type;
            std::wstring_convert<convert_type> converter;
            //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
            fileTextValue = converter.from_bytes(t_str);
            // std::wcout << fileTextValue.c_str();
            // std::cout << std::endl;
            break;
        }
    }
    if (fileTextValue.empty()) {
        throw ModuleException(L"file text.txt not exist");
    }
}

void Test::process(IConfigurationTool *configurationTool, IExecutionContextTool *executionContextTool, IValueFactory *factory) {
    // send messages
    executionContextTool->addMessage(factory->createData(counter));
    counter++;
    executionContextTool->addMessage(factory->createData(value));
    executionContextTool->addMessage(factory->createData(fileTextValue));

    // read input messages
    for (long i = 0; i < executionContextTool->getExecutionContext()->countSource(); ++i) {
        auto actions = executionContextTool->getMessages(i);
        for (IAction *action: *actions) {
            for (IMessage *message: *(action->getMessages())) {
                executionContextTool->addMessage(message);
            }
        }
    }

    // execute all execution contexts and result messages send as own message
    long countManagedExecutionContexts = executionContextTool->getFlowControlTool()->countManagedExecutionContexts();
    for (long i = 0; i < countManagedExecutionContexts; ++i) {
        std::vector<IValue *> params;
        params.push_back(factory->createData(param));
        CommandType type = SMCApi::CommandType::COMMAND_EXECUTE;
        executionContextTool->getFlowControlTool()->executeNow(type, i, &params);
        auto actions = executionContextTool->getFlowControlTool()->getMessagesFromExecuted(i);
        for (IAction *action: *actions) {
            for (IMessage *message: *(action->getMessages())) {
                executionContextTool->addMessage(message);
            }
        }
    }

    // read managed configurations
    long countManagedConfigurations = executionContextTool->getConfigurationControlTool()->countManagedConfigurations();
    for (long i = 0; i < countManagedConfigurations; ++i) {
        executionContextTool->addMessage(
                factory->createData(executionContextTool->getConfigurationControlTool()->getManagedConfiguration(i)->getName()));
    }

    // create new random configuration
    long countModules = executionContextTool->getConfigurationControlTool()->countModules();
    std::srand(std::time(nullptr));
    long idModule = std::rand() % countModules;
    CFGIModule *module = executionContextTool->getConfigurationControlTool()->getModule(idModule);
    CFGIConfigurationManaged *configuration = executionContextTool->getConfigurationControlTool()->createConfiguration(
            countManagedConfigurations, configurationTool->getContainer(), module, L"cfg-" + std::to_wstring(std::rand()));
    executionContextTool->addMessage(factory->createData(L"created cfg " + configuration->getName()));
    if (countManagedConfigurations > 0) {
        CFGIConfigurationManaged *configurationManaged = executionContextTool->getConfigurationControlTool()->getManagedConfiguration(0);
        if (configurationManaged->countExecutionContexts() > 0) {
            CFGIExecutionContextManaged *ec = configurationManaged->getExecutionContext(0);
            if (ec) {
                CFGIModule *moduleMain = configurationManaged->getModule();
                // add first execution context of created configuration to execution context list of first execution context first managed configuration
                if ((moduleMain->getMinCountExecutionContexts(0) <= ec->countExecutionContexts() + 1) &&
                    (moduleMain->getMaxCountExecutionContexts(0) == -1 ||
                     moduleMain->getMaxCountExecutionContexts(0) > ec->countExecutionContexts())) {
                    CFGIExecutionContextManaged *iExecutionContextManaged = configuration->getExecutionContext(0);
                    ec->insertExecutionContext(ec->countExecutionContexts(), iExecutionContextManaged);
                    executionContextTool->addMessage(
                            factory->createData(
                                    L"add " + configuration->getName() + L"." + iExecutionContextManaged->getName() + L" to " +
                                    configurationManaged->getName() +
                                    L"." + ec->getName()));
                }
                // add created configuration to managed configuration list of first execution context first managed configuration
                if ((moduleMain->getMinCountManagedConfigurations(0) <= ec->countManagedConfigurations() + 1) &&
                    (moduleMain->getMaxCountManagedConfigurations(0) == -1 ||
                     moduleMain->getMaxCountManagedConfigurations(0) > ec->countManagedConfigurations())) {
                    ec->insertManagedConfiguration(ec->countManagedConfigurations(), configuration);
                    executionContextTool->addMessage(
                            factory->createData(
                                    L"add " + configuration->getName() + L" to " + configurationManaged->getName() + L"." + ec->getName()));
                }
                // add first execution context of created configuration as source to first execution context first managed configuration
                if ((moduleMain->getMinCountSources(0) <= ec->countSource() + 1) &&
                    (moduleMain->getMaxCountSources(0) == -1 || moduleMain->getMaxCountSources(0) > ec->countSource())) {
                    CFGIExecutionContextManaged *iExecutionContextManaged = configuration->getExecutionContext(0);
                    CFGISourceManaged *sourceManaged = ec->createSourceExecutionContext(iExecutionContextManaged, SourceGetType::SGT_NEW, 0, false);
                    std::vector<std::unique_ptr<long>> filterParams;
                    filterParams.push_back(std::make_unique<long>(0));
                    filterParams.push_back(std::make_unique<long>(-1));
                    sourceManaged->createFilter(&filterParams, 0, 0, 0);
                    executionContextTool->addMessage(
                            factory->createData(
                                    L"add " + configuration->getName() + L"." + iExecutionContextManaged->getName() + L" to " +
                                    configurationManaged->getName() +
                                    L"." + ec->getName() + L" as source"));
                }
            }
        }
    }
}

void Test::update(IConfigurationTool *tool, IValueFactory *factory) {
    //std::wcout << "update " << tool->getName() << std::endl;
    stop(tool, factory);
    start(tool, factory);
}

void Test::stop(IConfigurationTool *tool, IValueFactory *factory) {
    //std::wcout << "stop " << tool->getName() << std::endl;
    delete this;
}

Test::~Test() {
    //std::wcout << "~Test" << std::endl;
}

Test::Test() {
    //std::wcout << "Test" << std::endl;
}

IMethod *getInstance() {
    return new Test();
}
