#include "program_options.hpp"

#include <sstream>

namespace base
{

namespace po = boost::program_options;

ProgramOptionsParser::ProgramOptionsParser() : _name(), _options_description("Allowed options"), _processor(nullptr)
{
    addFlag("help", "Print help message");
}

ProgramOptionsParser::ProgramOptionsParser(const std::string& name)
    : _name(name), _options_description("Allowed options"), _processor(nullptr)
{
    addFlag("help", "Print help message");
}

ProgramOptionsParser::ProgramOptionsParser(const std::string& name,
                                           std::function<void(const ProgramOptionsParser&)> processor)
    : _name(name), _options_description(std::string("Allowed options for ") + name), _processor(processor)
{
    addFlag("help", "Print help message");
}

std::shared_ptr<ProgramOptionsParser>
ProgramOptionsParser::createSubParser(const std::string& name, const std::string& descendant_description,
                                      const std::function<void(const ProgramOptionsParser&)>& processor)
{
    if(name.empty()) {
        RAISE_ERROR(base::InvalidArgument, "name for sub parser is empty");
    }
    else if(_descendants.count(name)) {
        RAISE_ERROR(base::InvalidArgument, "this sub parser already exists");
    }

    auto parser = std::make_shared<ProgramOptionsParser>(name, processor);
    _descendants.insert(std::pair<std::string, std::shared_ptr<ProgramOptionsParser>>(name, parser));
    _descendant_descriptions.insert(std::pair<std::string, std::string>(name, descendant_description));
    return parser;
}


void ProgramOptionsParser::addFlag(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), help.c_str());
}

void ProgramOptionsParser::process(int argc, char** argv)
{
    try {
        static constexpr size_t START_POSITION = 1; // first after executable
        if(argc > START_POSITION) {
            std::string sub_program(argv[START_POSITION]);
            if(_descendants.count(sub_program)) {
                _descendants.find(sub_program)->second->process(argc - START_POSITION, argv + START_POSITION);
                return; // not parse after found inner layer
            }
        }
        auto parsed_options = ::boost::program_options::parse_command_line(argc, argv, _options_description);
        ::boost::program_options::store(parsed_options, _options);
        ::boost::program_options::notify(_options);
        if(_processor != nullptr) {
            _processor(*this);
        }
    }
    catch(const boost::program_options::error& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }
}

std::string ProgramOptionsParser::helpMessage()
{
    std::stringstream ss;
    ss << _options_description << std::endl;
    if(!_descendant_descriptions.empty()) {
        ss << "Allowed commands:" << std::endl;
        std::string prefix;
        if (!_name.empty()){
            prefix.append(_name);
            prefix.append(" ");
        }
        for(auto& child: _descendant_descriptions) {
            static constexpr const char* SPACE = "   [ --help ]    ";
            ss << prefix << child.first << SPACE << child.second << std::endl; // TODO: add formatting later
        }
    }
    return ss.str();
}

bool ProgramOptionsParser::hasOption(const std::string& flag_name) const
{
    return _options.find(flag_name) != _options.end();
}



} // namespace base
