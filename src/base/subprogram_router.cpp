#include "subprogram_router.hpp"

#include <vector>

namespace base
{

SubprogramRouter::SubprogramRouter(const std::string& name, std::function<int(SubprogramRouter&)> processor)
    : _name(name), _processor(processor), _program_options(std::make_shared<ProgramOptionsParser>())
{}


void SubprogramRouter::addSubprogram(const std::string& name, const std::string& descendant_description,
    const std::function<int(SubprogramRouter&)>& processor)
{
    if(name.empty()) {
        RAISE_ERROR(base::InvalidArgument, "name of subprogram is empty");
    }
    else if(_descendants.count(name)) {
        RAISE_ERROR(base::InvalidArgument, "this subprogram already exists");
    }

    _descendants.insert(std::pair<std::string, std::shared_ptr<SubprogramRouter>>(
        name, std::make_shared<SubprogramRouter>(name, processor)));
    _descendant_descriptions.insert(std::pair<std::string, std::string>(name, descendant_description));
}


std::shared_ptr<ProgramOptionsParser> SubprogramRouter::optionsParser()
{
    return _program_options;
}


std::string SubprogramRouter::helpMessage() const
{
    std::stringstream ss;
    ss << _program_options->helpMessage() << std::endl;
    if(!_descendant_descriptions.empty()) {
        ss << "Allowed commands:" << std::endl;
        static constexpr const char* PREFIX = "   ";
        std::string prefix(PREFIX);
        if(!_name.empty()) {
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


void SubprogramRouter::update()
{
    std::vector<char*> options(_stored_options.size());
    for(std::size_t i = 0; i < _stored_options.size(); i++) {
        options[i] = _stored_options[i].data();
    }
    _program_options->process(_stored_options.size(), options.data());
}

int SubprogramRouter::process(int argc, char** argv)
{
    static constexpr int START_POSITION = 1; // first after executable
    if(argc > START_POSITION) {
        std::string sub_program(argv[START_POSITION]);
        if(_descendants.count(sub_program)) {
            return _descendants.find(sub_program)->second->process(argc - START_POSITION, argv + START_POSITION);
        }
        else if(sub_program.find('-') == std::string::npos) {
            RAISE_ERROR(base::InvalidArgument, "subprogram was not found");
        }
    }
    _stored_options.clear();
    for(int i = 0; i < argc; i++) {
        _stored_options.emplace_back(argv[i]);
    }
    return _processor(*this);
}

} // namespace base