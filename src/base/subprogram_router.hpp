#pragma once

#include "program_options.hpp"

namespace base
{

class SubprogramRouter
{
  public:
    //==============================
    explicit SubprogramRouter(const std::string& name, std::function<int(SubprogramRouter&)> processor);
    //==============================
    void addSubprogram(const std::string& name, const std::string& descendant_description,
        const std::function<int(SubprogramRouter&)>& processor);
    //==============================
    std::shared_ptr<ProgramOptionsParser> optionsParser();
    //==============================
    int process(int argc, char** argv);
    void update();
    //==============================
    std::string helpMessage() const;
    //==============================
  private:
    //==============================
    const std::string _name;
    const std::function<int(SubprogramRouter&)> _processor;
    //==============================
    std::shared_ptr<ProgramOptionsParser> _program_options;
    std::vector<std::string> _stored_options;
    //==============================
    std::map<std::string, std::shared_ptr<SubprogramRouter>> _descendants;
    std::map<std::string, std::string> _descendant_descriptions;
};

} // namespace base
