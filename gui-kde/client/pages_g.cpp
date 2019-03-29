extern "C" {
#include "pages_g.h"
}
#include "logging.h"
#include "application.h"

void update_start_page() {
  KV::Application::UpdateUsers(nullptr);
}

