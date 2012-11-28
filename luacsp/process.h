#pragma once

namespace csp
{
    class Operation;

    class Process
    {
    private:
        Process* parentProcess;
        Operation* operation;
    };
}