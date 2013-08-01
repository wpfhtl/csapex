/// HEADER
#include <csapex/message_provider_manager.h>

/// SYSTEM
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;
using namespace csapex;

MessageProviderManager::MessageProviderManager()
    : PluginManager<csapex::MessageProvider>("csapex::MessageProvider")
{
}

MessageProviderManager& MessageProviderManager::instance()
{
    static MessageProviderManager instance;
    return instance;
}

void MessageProviderManager::fullReload()
{
    instance().reload();

    classes.clear();

    typedef std::pair<std::string, PluginManager<csapex::MessageProvider>::Constructor> PAIR;
    foreach(PAIR pair, availableClasses()) {
        MessageProvider::Ptr prov(pair.second());
        foreach(const std::string& extension, prov->getExtensions()) {
            std::string ext = extension;
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            registerMessageProvider(ext, pair.second);
        }
    }
}

MessageProvider::Ptr MessageProviderManager::createMessageProvider(const std::string& path)
{
    return instance().createMessageProviderHelper(path);
}

MessageProvider::Ptr MessageProviderManager::createMessageProviderHelper(const std::string& path)
{
    if(!pluginsLoaded()) {
        fullReload();
    }

    bfs::path file(path);

    bool is_dir = bfs::is_directory(file) ;
    std::string ext = is_dir ? ".DIRECTORY" : file.extension().string();

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if(path.empty()) {
        return MessageProvider::NullPtr;
    }

    std::cout << "extension is " << ext << std::endl;

    if(classes.empty()) {
        throw std::runtime_error("no message providers registered!");
    }

    return classes[ext]();
}

void MessageProviderManager::registerMessageProvider(const std::string &type, Constructor constructor)
{
    instance().classes[type] = constructor;
}
