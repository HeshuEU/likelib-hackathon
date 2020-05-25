#pragma once

#include "base/program_options.hpp"

namespace base
{

class SubprogramRouter
{
  public:
    //---------------------------
    SubprogramRouter(std::string name, std::function<int(SubprogramRouter&)> processor);
    //---------------------------
    void addSubprogram(const std::string& name,
                       const std::string& descendant_description,
                       const std::function<int(SubprogramRouter&)>& processor);
    //---------------------------
    const ProgramOptionsParser& getOptionsParser() const noexcept;
    ProgramOptionsParser& getOptionsParser() noexcept;
    //---------------------------
    int process(int argc, const char* const* argv);
    void update();
    //---------------------------
    std::string helpMessage() const;
    //---------------------------
  private:
    //---------------------------
    const std::string _name;
    const std::function<int(SubprogramRouter&)> _processor;
    //---------------------------
    ProgramOptionsParser _program_options;
    std::vector<std::string> _stored_options;
    //---------------------------
    std::map<std::string, SubprogramRouter> _descendants;
    std::map<std::string, std::string> _descendant_descriptions;
};

} // namespace base
