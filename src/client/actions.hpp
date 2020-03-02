#pragma once

#include <base/config.hpp>
#include <base/subprogram_router.hpp>


int testConnection(base::SubprogramRouter& router);
int getBalance(base::SubprogramRouter& router);
int transfer(base::SubprogramRouter& router);
int createContract(base::SubprogramRouter& router);
int messageCall(base::SubprogramRouter& router);
int compileCode(base::SubprogramRouter& router);
int generateKeys(base::SubprogramRouter& router);


class ActionBase {
public:
    explicit ActionBase(base::SubprogramRouter& router)
        : _router{router}
    {}

    virtual ~ActionBase() = default;

    virtual void setupOptionsParser(base::ProgramOptionsParser& parser) const = 0;
    virtual void checkOptions(const base::ProgramOptionsParser& parser) const = 0;
    virtual void execute() const = 0;

    int run() {
        setupOptionsParser(_router.getOptionsParser());
        checkOptions(_router.getOptionsParser());
        execute();

        return base::config::EXIT_OK;
    }

private:
    base::SubprogramRouter& _router;
};