#include "Application.hpp"
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace suplex {
    extern std::shared_ptr<Application> CreateApplication(int argc, char** argv);
}  // namespace suplex

bool g_Running = true;

int main(int argc, char** argv)
{
    auto app = suplex::CreateApplication(argc, argv);
    app->Initialize();
    app->Run();
    app->Cleanup();
    return 0;
}
