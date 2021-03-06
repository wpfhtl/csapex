/// HEADER
#include <csapex/core/settings.h>

/// PROJECT
#include <csapex/param/io.h>
#include <csapex/utility/yaml_node_builder.h>

/// SYSTEM
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include <fstream>
#include <iostream>
#include <string>

#ifdef WIN32
#include <Shlobj.h>
#else
#include <pwd.h>
#endif

#if (BOOST_VERSION / 100000) >= 1 && (BOOST_VERSION / 100 % 1000) >= 54
namespace bf3 = boost::filesystem;
#else
namespace bf3 = boost::filesystem3;
#endif

using namespace csapex;

const std::string Settings::settings_file = defaultConfigPath() + "cfg/persistent_settings";
const std::string Settings::config_extension = ".apex";
const std::string Settings::template_extension = ".apext";
const std::string Settings::message_extension = ".apexm";
const std::string Settings::default_config = Settings::defaultConfigFile();
const std::string Settings::config_selector = "Configs(*" + Settings::config_extension + ");;LegacyConfigs(*.vecfg)";

const std::string Settings::namespace_separator = ":/:";


std::string Settings::defaultConfigPath()
{
#ifdef WIN32
	CHAR path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path))) {
		return std::string(path) + "/.csapex/";
	}
#else
    struct passwd *pw = getpwuid(getuid());
    return std::string(pw->pw_dir) + "/.csapex/";
#endif
}

std::string Settings::defaultConfigFile()
{
    std::string dir = Settings::defaultConfigPath();

    if(!bf3::exists(dir)) {
        bf3::create_directories(dir);
    }

    std::string file = dir + "default" + Settings::config_extension;

    if(!bf3::exists(file)) {
        //        createDefaultConfig(file);
    }

    return file;
}

Settings::Settings()
{
    load();
}

void Settings::save()
{
    YAML::Emitter yaml;
    yaml << YAML::BeginSeq;

    for(std::map<std::string, csapex::param::Parameter::Ptr>::iterator it = settings_.begin(); it != settings_.end(); ++it) {
        csapex::param::Parameter::Ptr p = it->second;
        YAML::Node n;
        p->serialize(n);
        yaml << n;
    }

    yaml << YAML::EndSeq;

    std::ofstream ofs(settings_file.c_str());
    ofs << yaml.c_str();
}

void Settings::load()
{
    std::ifstream ifs(settings_file.c_str());
    YAML::Parser parser(ifs);

    YAML::NodeBuilder builder;
    if (!parser.HandleNextDocument(builder)) {
        std::cerr << "cannot read the config" << std::endl;
    }
    YAML::Node doc = builder.Root();

    if(doc.Type() != YAML::NodeType::Sequence) {
        std::cerr << "cannot read the settings" << std::endl;
        return;
    }

    for(std::size_t i = 0, n = doc.size(); i < n; ++i) {
        csapex::param::Parameter::Ptr p = doc[i].as<csapex::param::Parameter::Ptr>();

        settings_[p->name()] = p;
    }
}

void Settings::add(csapex::param::Parameter::Ptr p)
{
    settings_[p->name()] = p;
    settingsChanged();
}

csapex::param::Parameter::Ptr Settings::get(const std::string &name)
{
    return settings_.at(name);
}

bool Settings::knows(const std::string &name) const
{
    return settings_.find(name) != settings_.end();
}
